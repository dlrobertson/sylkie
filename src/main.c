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
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <sylkie_config.h>

#ifdef BUILD_JSON
#include <json-c/json.h>
#endif

#include <buffer.h>
#include <packet.h>
#include <sender_map.h>

#include <cfg.h>
#include <cmds.h>

// Transmitter fork main function
extern int tx_main(const struct pkt_cmd_list *lst,
                   struct sylkie_sender_map *map, int ipc);

// Receiver fork main function (main thread)
extern int rx_main(const struct lst_cmd_list *lst, pid_t tx_pid, int ipc);

// various subcommands to the sylkie command that are defined elsewhere

// neighbor advert subcommand functions
extern int na_parse(const struct sylkie_sender_map *ifaces,
                    const struct cfg_set *set, struct command_lists *lists);
extern const struct cfg_template *generate_na_template();

// router advert subcommand functions
extern int ra_parse(const struct sylkie_sender_map *ifaces,
                    const struct cfg_set *set, struct command_lists *lists);
extern const struct cfg_template *generate_ra_template();

static struct sylkie_sender_map *s_ifaces = NULL;

static const struct cmd {
  const char *short_name;
  const char *long_name;
  int (*parse)(const struct sylkie_sender_map *ifaces,
               const struct cfg_set *set, struct command_lists *lists);
  const struct cfg_template *(*generate_template)();
} cmds[] = {{"na", "neighbor-advert", na_parse, generate_na_template},
            {"ra", "router-advert", ra_parse, generate_ra_template},
            {NULL}};

// Helper functions used to translate a json file or command line
// arguments into a cfg_set
#ifdef BUILD_JSON
int parse_json(struct json_object *jobj, const struct cmd *cmd,
               struct command_lists *lists) {
  int res = -1;
  struct cfg_set set;

  res = cfg_set_init_json(&set, cmd->generate_template(), jobj);

  if (res) {
    fprintf(stderr, "Failed to initialize parsers\n");
    return res;
  } else if (cfg_set_find(&set, "help")) {
    fprintf(stderr,
            "\"help\" is an invalid option for running sylkie from json\n");
    cfg_set_free(&set);
    return res;
  }

  res = cmd->parse(s_ifaces, &set, lists);
  cfg_set_free(&set);
  return res;
}
#endif

int parse_cmdline(int argc, const char **argv, const struct cmd *cmd,
                  struct command_lists *cmd_lists) {
  int res = -1;
  struct cfg_set set;

  if (argc < 1) {
    fprintf(stderr, "Too few arguments\n");
    cfg_set_usage(&set, stderr);
    return res;
  }

  res = cfg_set_init_cmdline(&set, cmd->generate_template(), --argc, ++argv);

  if (res) {
    cfg_set_usage(&set, stderr);
    return res;
  } else if (cfg_set_find(&set, "help")) {
    cfg_set_usage(&set, stdout);
    cfg_set_free(&set);
    exit(0);
  }

  res = cmd->parse(s_ifaces, &set, cmd_lists);
  cfg_set_free(&set);

  return res;
}

void version(FILE *fd) { fprintf(fd, "sylkie version: " SYLKIE_VERSION "\n"); }

const struct cmd *find_cmd(const char *input) {
  const struct cmd *cmd;
  if (input) {
    for (cmd = cmds; cmd && cmd->short_name && cmd->long_name; ++cmd) {
      if (strcmp(input, cmd->short_name) == 0 ||
          strcmp(input, cmd->long_name) == 0) {
        return cmd;
      }
    }
  }
  return NULL;
}

struct sylkie_buffer *read_file(const char *arg) {
  int fd, res;
  u_int8_t tmp_buffer[1025];
  struct sylkie_buffer *buf = sylkie_buffer_init(1024);
  if (!buf) {
    return NULL;
  }
  if ((fd = open(arg, O_RDONLY)) < 0) {
    fprintf(stderr, "%s cannot be opened\n", arg);
    sylkie_buffer_free(buf);
    return NULL;
  }
  while ((res = read(fd, tmp_buffer, 1024)) != 0) {
    if (res < 0) {
      fprintf(stderr, "Failed to read from file %s\n", arg);
      sylkie_buffer_free(buf);
      return NULL;
    }
    sylkie_buffer_add(buf, tmp_buffer, res);
  }
  // Note: buf->len will be 0 if the file is empty
  return buf;
}

#ifdef BUILD_JSON
int get_json_cmd(const struct cmd *cmd, struct json_object *jobj,
                 struct command_lists *lists) {
  enum json_type type;
  struct json_object *tmp;
  int i, len = json_object_array_length(jobj), retval;
  for (i = 0; i < len; ++i) {
    tmp = json_object_array_get_idx(jobj, i);
    type = json_object_get_type(tmp);
    if (type != json_type_object) {
      fprintf(stderr, "Unexpected type at index %d\n", i);
      return -1;
    }
    retval = parse_json(tmp, cmd, lists);
    if (retval) {
      return retval;
    }
  }
  return 0;
}
#endif

int run_from_string(char *line, struct command_lists *cmd_lists) {
  const struct cmd *cmd;
  char *p = strtok(line, " ");
  static char *args[1024];
  int i = 0;
  while (p != NULL) {
    if (i < 1023) {
      args[i] = p;
      p = strtok(NULL, " ");
    } else {
      fprintf(stderr, "ERROR: Too many arguments provided to command\n");
      return -1;
    }
    ++i;
  }
  args[i] = NULL;
  if (i > 0 && (cmd = find_cmd(args[0])) != NULL) {
    return parse_cmdline(i, (const char **)args, cmd, cmd_lists);
  } else {
    fprintf(stderr, "ERROR: could not parse line: \n\t%s\n", line);
    return -1;
  }
}

int run_from_plaintext(const char *arg, struct command_lists *cmd_lists) {
  int retval = 0;
  struct sylkie_buffer *buf = read_file(arg);
  const u_int8_t *iter = NULL;
  const u_int8_t *end = NULL;
  const u_int8_t *pos = NULL;
  size_t len;
  char *cpy = NULL;
  if (!buf) {
    return -1;
  }

  if (buf->len <= 0) {
    sylkie_buffer_free(buf);
    fprintf(stderr, "Error: Attempted to read from empty file %s\n", arg);
    return -1;
  }

  iter = buf->data;
  end = buf->data + buf->len;
  while (iter < end && !retval) {
    pos = (const u_int8_t *)strchr((const char *)iter, '\n');
    if (pos < end && pos > iter) {
      len = pos - iter;
      cpy = malloc(len + 1);
      if (cpy) {
        memcpy(cpy, iter, len);
        cpy[len] = '\0';
        retval = run_from_string(cpy, cmd_lists);
        if (retval) {
          free(cpy);
          break;
        } else {
          free(cpy);
          iter = ++pos;
        }
      } else {
        fprintf(stderr, "Error: No memory!\n");
        retval = -1;
        break;
      }
    } else {
      break;
    }
  }
  sylkie_buffer_free(buf);
  return retval;
}

#ifdef BUILD_JSON
int run_from_json(const char *arg, struct command_lists *lists) {
  int retval = 0;
  enum json_tokener_error jerr;
  enum json_type type;
  struct json_object *jobj = NULL;
  struct json_tokener *tok = json_tokener_new();
  struct sylkie_buffer *buf = read_file(arg);
  const struct cmd *cmd;

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
    jobj = json_tokener_parse_ex(tok, (const char *)buf->data, buf->len);
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
        retval = get_json_cmd(cmd, val, lists);
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

  return retval;
}
#endif

static struct cfg_parser parsers[] = {
    {'h', "help", CFG_BOOL, "print helpful usage information"},
    {'v', "version", CFG_BOOL, "print the version number of sylkie"},
    {'j', "json", CFG_STRING, "parse input from the provided json file"},
    {'x', "execute", CFG_STRING, "parse input from the provided text file"}};
static size_t parsers_sz = sizeof(parsers) / sizeof(struct cfg_parser);

static struct cfg_subcmd subcmds[] = {{"na", "neighbor-advert"},
                                      {"ra", "router-advert"}};
static size_t subcmds_sz = sizeof(subcmds) / sizeof(struct cfg_subcmd);

int main(int argc, const char **argv) {
  char *input_file;
  const struct cmd *cmd;
  int retval = 0;
  struct pkt_cmd_list_item *item;
  pid_t tx_pid = 0;
  struct cfg_set set;
  struct cfg_template templt = {
      .usage = "sylkie [ OPTIONS | SUBCOMMAND ]",
      .summary = "IPv6 address spoofing with the Neighbor Discovery Protocol",
      .parsers = parsers,
      .parsers_sz = parsers_sz,
      .subcmds = subcmds,
      .subcmds_sz = subcmds_sz};
  struct command_lists cmd_lists;
  int sv[2] = {-1, -1};
  cmd_lists.pkt_cmds = NULL;
  cmd_lists.lst_cmds = NULL;

  // Ensure we were given args
  if (argc < 2) {
    fprintf(stderr, "Too few arguments\n");
    cfg_template_usage(&templt, stderr);
    return -1;
  }

  // Initialize the global command list and
  // interface cache.
  cmd_lists.pkt_cmds = pkt_cmd_list_init();
  cmd_lists.lst_cmds = lst_cmd_list_init();
  if (!cmd_lists.pkt_cmds || !cmd_lists.lst_cmds) {
    fprintf(stderr, "Could not initialize list of commands: No Memory\n");
    return -1;
  }

  s_ifaces = sylkie_sender_map_init(2);
  if (!s_ifaces) {
    fprintf(stderr, "Could not initialize list of senders: No Memory\n");
    return -1;
  }

  // Parse the user input and create a command list
  // from it.
  --argc;
  ++argv;
  if (argv[0] && argv[0][0] == '-') {
    retval = cfg_set_init_cmdline(&set, &templt, argc, argv);

    if (retval) {
      fprintf(stderr, "Failed to parse user input\n");
      cfg_set_usage(&set, stderr);
    } else {
      if (cfg_set_find(&set, "version")) {
        version(stdout);
      }

      if (cfg_set_find(&set, "help")) {
        cfg_set_usage(&set, stdout);
      }

      if (!cfg_set_find_type(&set, "json", CFG_STRING, &input_file)) {
#ifdef BUILD_JSON
        retval = run_from_json(input_file, &cmd_lists);
#else
        fprintf(stderr, "sylkie must be built with json support to "
                        "use json functions\n");
        retval = -1;
#endif
      } else if (!retval &&
                 !cfg_set_find_type(&set, "execute", CFG_STRING, &input_file)) {
        retval = run_from_plaintext(input_file, &cmd_lists);
      }
      cfg_set_free(&set);
    }
  } else {
    // This is not an option. Forward to next cmd
    cmd = find_cmd(*argv);

    if (!cmd) {
      fprintf(stderr, "Unknown command: %s\n", *argv);
      cfg_template_usage(&templt, stderr);
      retval = -1;
    } else {
      retval = parse_cmdline(argc, argv, cmd, &cmd_lists);
    }
  }

  // Setup the socket for communication on the rx and tx procs
  if (!retval) {
    if (!socketpair(AF_UNIX, SOCK_SEQPACKET, 0, sv)) {
      // Create the receiver and transmitter procs
      if (cmd_lists.pkt_cmds->head || cmd_lists.lst_cmds->head) {
        if ((tx_pid = fork()) < 0) {
          fprintf(stderr, "Failed to create transmitter fork\n");
          retval = -1;
        } else if (tx_pid == 0) {
          retval = tx_main(cmd_lists.pkt_cmds, s_ifaces, sv[0]);
          _exit(retval);
        } else {
          rx_main(NULL, tx_pid, sv[1]);
        }
      }
      close(sv[0]);
      close(sv[1]);
    } else {
      fprintf(stderr, "Could not create ipc socets\n");
    }
  }

  for (item = cmd_lists.pkt_cmds->head; item; item = item->next) {
    sylkie_packet_free(item->value->pkt);
    free(item->value);
  }
  command_lists_free(&cmd_lists);

  return retval;
}
