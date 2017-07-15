// Copyright (c) 2017 Daniel L. Robertson
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <sylkie_config.h>

#ifdef BUILD_JSON
#include <json-c/json.h>
#endif

#include <buffer.h>
#include <cfg.h>
#include <cmds.h>

/**
 * \brief Growable buffer of pids
 * \ingroup sylkie
 *
 * Used to track pids of forked child processes.
 */
struct pid_buf {
    pid_t* pids;
    size_t len;
    size_t cap;
};

struct pid_buf* pid_buf_init(size_t len) {
    struct pid_buf* buf = malloc(sizeof(struct pid_buf));
    if (!buf) {
        return NULL;
    }
    if (len) {
        buf->pids = malloc(len * sizeof(pid_t));
        if (!buf->pids) {
            free(buf);
            return NULL;
        }
        buf->cap = len;
        buf->len = 0;
    } else {
        buf->pids = malloc(1 * sizeof(pid_t));
        buf->len = 0;
        buf->cap = 1;
    }
    return buf;
}

int pid_buf_add(struct pid_buf* buf, pid_t pid) {
    pid_t* tmp;
    if (!((buf->len + 1) < buf->cap)) {
        tmp = malloc((buf->cap << 1) * sizeof(pid_t));
        if (!tmp) {
            return -1;
        }
        memcpy(tmp, buf->pids, buf->len);
        free(buf->pids);
        buf->pids = tmp;
        buf->cap = buf->cap << 1;
    }
    buf->pids[buf->len] = pid;
    ++buf->len;
    return 0;
}

void pid_buf_free(struct pid_buf* buf) {
    if (buf) {
        if (buf->pids) {
            free(buf->pids);
        }
        free(buf);
        buf = NULL;
    }
}

static const struct cmd {
    const char* name;
    int (*cmdline_func)(int argc, const char** argv);
#ifdef BUILD_JSON
    pid_t (*json_func)(struct json_object* jobj);
#endif
} cmds[] = {{"na", na_cmdline,
#ifdef BUILD_JSON
             na_json
#endif
            },
            {"ra", ra_cmdline,
#ifdef BUILD_JSON
             ra_json
#endif
            },
            {NULL}};

void version(FILE* fd) { fprintf(fd, "sylkie version: " SYLKIE_VERSION "\n"); }

const struct cmd* find_cmd(const char* input) {
    const struct cmd* cmd;
    if (input) {
        for (cmd = cmds; cmd && cmd->name; ++cmd) {
            if (strcmp(input, cmd->name) == 0) {
                return cmd;
            }
        }
    }
    return NULL;
}

struct sylkie_buffer* read_file(const char* arg) {
    int fd, res;
    u_int8_t tmp_buffer[1025];
    struct sylkie_buffer* buf = sylkie_buffer_init(1024);
    if (!buf) {
        return NULL;
    }
    if ((fd = open(arg, O_RDONLY)) < 0) {
        fprintf(stderr, "%s cannot be opened\n", arg);
        return NULL;
    }
    while ((res = read(fd, tmp_buffer, 1024)) != 0) {
        if (res < 0) {
            fprintf(stderr, "Failed to read from file %s\n", arg);
            return NULL;
        }
        sylkie_buffer_add(buf, tmp_buffer, res);
    }
    // Note: buf->len will be 0 if the file is empty
    return buf;
}

#ifdef BUILD_JSON
pid_t run_json_cmd(int (*func)(struct json_object* jobj),
                   struct json_object* jobj) {
    pid_t pid = -1;
    enum json_type type;
    struct json_object* tmp;
    int i, len = json_object_array_length(jobj);
    for (i = 0; i < len; ++i) {
        tmp = json_object_array_get_idx(jobj, i);
        type = json_object_get_type(tmp);
        if (type != json_type_object) {
            fprintf(stderr, "Unexpected type at index %d\n", i);
            return -1;
        }
        pid = func(tmp);
    }
    return pid;
}

int run_from_json(const char* arg, struct pid_buf* pid_buf) {
    int retval = 0;
    pid_t pid = -1;
    enum json_tokener_error jerr;
    enum json_type type;
    struct json_object* jobj = NULL;
    struct json_tokener* tok = json_tokener_new();
    struct sylkie_buffer* buf = read_file(arg);
    const struct cmd* cmd;

    if (!buf) {
        json_tokener_free(tok);
        return -1;
    }

    if (!tok) {
        sylkie_buffer_free(buf);
        return -1;
    }

    if (buf->len <= 0) {
        sylkie_buffer_free(buf);
        json_tokener_free(tok);
        fprintf(stderr, "Error: Attempted to read from empty file %s\n", arg);
        return -1;
    }

    do {
        jobj = json_tokener_parse_ex(tok, (const char*)buf->data, buf->len);
        jerr = json_tokener_get_error(tok);
    } while (jerr == json_tokener_continue);

    if (jerr != json_tokener_success) {
        fprintf(stderr, "Error: %s\n", json_tokener_error_desc(jerr));
        json_object_put(jobj);
        json_tokener_free(tok);
        sylkie_buffer_free(buf);
        return -1;
    }

    json_object_object_foreach(jobj, key, val) {
        cmd = find_cmd(key);
        if (cmd) {
            type = json_object_get_type(val);
            switch (type) {
            case json_type_array:
                pid = run_json_cmd(cmd->json_func, val);
                if (pid >= 0) {
                    retval = pid_buf_add(pid_buf, pid);
                    if (retval) {
                        fprintf(stderr, "Failed to track pid but we're going "
                                        "to keep trucking\n");
                    }
                } else {
                    fprintf(stderr, "Failed to fork proccess\n");
                }
                break;
            default:
                fprintf(stderr, "Expected array of objects for key %s\n", key);
                retval = -1;
                break;
            }

        } else {
            fprintf(stderr, "Unknown command: %s\n", arg);
            retval = -1;
        }

        if (retval) {
            break;
        }
    }

    json_object_put(jobj);
    json_tokener_free(tok);
    sylkie_buffer_free(buf);

    return 0;
}
#endif

static struct cfg_parser parsers[] = {
    {'h', "help", CFG_BOOL, "print helpful usage information"},
    {'v', "version", CFG_BOOL, "print the version number of sylkie"},
    {'j', "json", CFG_STRING, "parse input from the provided json file"}};
static size_t parsers_sz = sizeof(parsers) / sizeof(struct cfg_parser);

int main(int argc, const char** argv) {
    char* input_file;
    const struct cmd* cmd;
    int retval = 0, i = 0;
    struct cfg_set set = {
        .usage = "sylkie [ OPTIONS | SUBCOMMAND ]",
        .summary = "IPv6 address spoofing with the Neighbor Discovery Protocol",
        .parsers = parsers,
        .parsers_sz = parsers_sz};
    struct pid_buf* pid_buf = NULL;

    if (argc < 2) {
        fprintf(stderr, "Too few arguments\n");
        cfg_set_usage(&set, stderr);
        return -1;
    }

    --argc;
    ++argv;
    if (argv[0] && argv[0][0] == '-') {
        retval = cfg_set_init_cmdline(&set, argc, argv);

        if (retval) {
            fprintf(stderr, "Failed to parse user input\n");
            cfg_set_usage(&set, stderr);
            return -1;
        } else {

            pid_buf = pid_buf_init(10);
            if (!pid_buf) {
                fprintf(stderr, "Could not allocate pid buffer\n");
                retval = -1;
            } else {

                if (cfg_set_find(&set, "version")) {
                    version(stdout);
                }

                if (cfg_set_find(&set, "help")) {
                    cfg_set_usage(&set, stdout);
                }

                if (!cfg_set_find_type(&set, "json", CFG_STRING, &input_file)) {
#ifdef BUILD_JSON
                    retval = run_from_json(input_file, pid_buf);
#else
                    fprintf(stderr, "sylkie must be built with json support to "
                                    "use json functions\n");
                    retval = -1;
#endif
                }

                for (i = 0; i < pid_buf->len; ++i) {
                    waitpid(pid_buf->pids[i], &retval, 0);
                }

                pid_buf_free(pid_buf);
            }
        }
    } else {
        // This is not an option. Forward to next cmd
        cmd = find_cmd(*argv);

        if (!cmd) {
            fprintf(stderr, "Unknown command: %s\n", *argv);
            cfg_set_usage(&set, stderr);
            retval = -1;
        } else {
            retval = (*cmd->cmdline_func)(argc, argv);
        }
    }

    cfg_set_free(&set);

    return retval;
}
