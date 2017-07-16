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
#include <strings.h>

#include <arpa/inet.h>
#include <netinet/ether.h>
#include <netinet/in.h>

#include <cfg.h>
#include <utils.h>

// Integer parsers
//
// NB: Currently integer parsers only accept a positive value or -1

static struct cfg_set_item* parse_int_cmdline(const struct cfg_parser* parser,
                                              const char* arg) {
    int value = strtol(arg, NULL, 10);
    if (value < 0) {
        if (strcmp(arg, "-1") == 0) {
            value = -1;
        } else {
            return NULL;
        }
    }
    return cfg_set_item_create(parser, &value);
}

#ifdef BUILD_JSON
static struct cfg_set_item* parse_int_json(const struct cfg_parser* parser,
                                           struct json_object* jobj) {
    int value = 0;
    enum json_type type = json_object_get_type(jobj);
    if (type == json_type_int) {
        value = json_object_get_int(jobj);
        if (value < -1) {
            return NULL;
        }
        return cfg_set_item_create(parser, &value);
    } else {
        return NULL;
    }
}
#endif

// Word parsers
//
// Parser used to parse a 16 bit unsigned integer

static struct cfg_set_item* parse_word_cmdline(const struct cfg_parser* parser,
                                               const char* arg) {
    int value = strtol(arg, NULL, 10);
    if (value > 0xffff || value < 0) {
        return NULL;
    } else {
        return cfg_set_item_create(parser, &value);
    }
}

#ifdef BUILD_JSON
static struct cfg_set_item* parse_word_json(const struct cfg_parser* parser,
                                            struct json_object* jobj) {
    int value = 0;
    enum json_type type = json_object_get_type(jobj);
    if (type == json_type_int) {
        value = json_object_get_int(jobj);
        if (value > 0xffff || value < 0) {
            return NULL;
        } else {
            return cfg_set_item_create(parser, &value);
        }
    } else {
        return NULL;
    }
}
#endif

// Byte parsers
//
// Parser used to parse a 8 bit unsigned integer

static struct cfg_set_item* parse_byte_cmdline(const struct cfg_parser* parser,
                                               const char* arg) {
    int value = strtol(arg, NULL, 10);
    if (value > 0xff || value < 0) {
        return NULL;
    } else {
        return cfg_set_item_create(parser, &value);
    }
}

#ifdef BUILD_JSON
static struct cfg_set_item* parse_byte_json(const struct cfg_parser* parser,
                                            struct json_object* jobj) {
    int value = 0;
    enum json_type type = json_object_get_type(jobj);
    if (type == json_type_int) {
        value = json_object_get_int(jobj);
        if (value > 0xff || value < 0) {
            return NULL;
        } else {
            return cfg_set_item_create(parser, &value);
        }
    } else {
        return NULL;
    }
}
#endif

// String parsers
//
// Parser used to convert a user provided string to a cfg_set_item

// Helper function used to copy the given string.
//
// TODO(dlrobertson): This seems rediculous. strdup probably does this
// better. clang-tidy throws a fit when strdup is used. Find a way to
// use strdup instead or prove that this is better.
static char* copy_str(const char* arg) {
    int len;
    char* value;
    if (!arg || (len = strlen(arg)) <= 0) {
        value = malloc(1);
        if (value) {
            value[0] = '\0';
            return value;
        } else {
            return NULL;
        }
    } else {
        value = malloc(len + 1);
        if (value) {
            strncpy(value, arg, len);
            value[len] = '\0';
            return value;
        } else {
            return NULL;
        }
    }
}

static struct cfg_set_item*
parse_string_cmdline(const struct cfg_parser* parser, const char* arg) {
    char* value = copy_str(arg);
    struct cfg_set_item* item = NULL;
    if (value) {
        // TODO(dlrobertson): linters complain about using cfg_set_item_create
        // here. Figure out why that is the case. There is probably a bug here
        item = malloc(sizeof(struct cfg_set_item));
        item->value.type = parser->type;
        item->name = parser->long_name;
        item->value.data = value;
        return item;
    } else {
        return NULL;
    }
}

#ifdef BUILD_JSON
static struct cfg_set_item* parse_string_json(const struct cfg_parser* parser,
                                              struct json_object* jobj) {
    char* value = NULL;
    struct cfg_set_item* item = NULL;
    enum json_type type = json_object_get_type(jobj);
    if (type == json_type_string) {
        value = copy_str(json_object_get_string(jobj));
        if (value) {
            // TODO(dlrobertson): linters complain about using
            // cfg_set_item_create here. Figure out why that is
            // the case. There is probably a bug here
            item = malloc(sizeof(struct cfg_set_item));
            item->value.type = parser->type;
            item->name = parser->long_name;
            item->value.data = value;
            return item;
        } else {
            return NULL;
        }
    } else {
        return NULL;
    }
}
#endif

// IPv6 address parsers
//
// Parse a ipv6 address from a user provided string.

static struct cfg_set_item*
parse_ipv6_addr_cmdline(const struct cfg_parser* parser, const char* arg) {
    int res = 0;
    struct in6_addr* value = malloc(sizeof(struct in6_addr));

    if (!value) {
        return NULL;
    }

    res = inet_pton(AF_INET6, arg, value);
    if (res <= 0) {
        return NULL;
    } else {
        return cfg_set_item_create(parser, value);
    }
}

#ifdef BUILD_JSON
static struct cfg_set_item*
parse_ipv6_addr_json(const struct cfg_parser* parser,
                     struct json_object* jobj) {
    int res = 0;
    const char* tmp = NULL;
    struct in6_addr* value = malloc(sizeof(struct in6_addr));
    enum json_type type = json_object_get_type(jobj);

    if (!value) {
        return NULL;
    }

    if (type == json_type_string) {
        tmp = json_object_get_string(jobj);
        res = inet_pton(AF_INET6, tmp, value);
        if (res <= 0) {
            free(value);
            return NULL;
        } else {
            return cfg_set_item_create(parser, value);
        }
    } else {
        free(value);
        return NULL;
    }
}
#endif

// Mac parsers
//
// Parse a mac address from a user provided string.

static struct cfg_set_item* parse_mac_cmdline(const struct cfg_parser* parser,
                                              const char* arg) {
    int res = 0;
    u_int8_t* value = malloc(ETH_ALEN);

    if (!value) {
        return NULL;
    }

    res = parse_hwaddr(arg, value);
    if (res < 0) {
        free(value);
        return NULL;
    } else {
        return cfg_set_item_create(parser, value);
    }
}

#ifdef BUILD_JSON
static struct cfg_set_item* parse_mac_json(const struct cfg_parser* parser,
                                           struct json_object* jobj) {
    int res = 0;
    u_int8_t* value = malloc(ETH_ALEN);
    const char* tmp = NULL;
    enum json_type type = json_object_get_type(jobj);

    if (!value) {
        return NULL;
    }

    if (type == json_type_string) {
        tmp = json_object_get_string(jobj);
        res = parse_hwaddr(tmp, value);
        if (res < 0) {
            free(value);
            return NULL;
        } else {
            return cfg_set_item_create(parser, value);
        }
    } else {
        free(value);
        return NULL;
    }
}
#endif

// Boolean parsers
//
// NB: The command line version does not need a parser. The value is assumed by
// the presence of the option.

#ifdef BUILD_JSON
static struct cfg_set_item* parse_bool_json(const struct cfg_parser* parser,
                                            struct json_object* jobj) {
    bool value;
    enum json_type type = json_object_get_type(jobj);
    if (type == json_type_boolean) {
        value = json_object_get_boolean(jobj);
        return cfg_set_item_create(parser, &value);
    } else {
        return NULL;
    }
}
#endif

// Helper function used to choose the correct parsing function based on the
// argument type for command line input
static struct cfg_set_item* parse_cmdline_arg(const struct cfg_parser* parser,
                                              const char* arg) {
    switch (parser->type) {
    case CFG_INT:
        return parse_int_cmdline(parser, arg);
    case CFG_WORD:
        return parse_word_cmdline(parser, arg);
    case CFG_BYTE:
        return parse_byte_cmdline(parser, arg);
    case CFG_STRING:
        return parse_string_cmdline(parser, arg);
    case CFG_IPV6_ADDRESS:
        return parse_ipv6_addr_cmdline(parser, arg);
    case CFG_HW_ADDRESS:
        return parse_mac_cmdline(parser, arg);
    default:
        return NULL;
    }
}

#ifdef BUILD_JSON
// Helper function used to choose the correct parsing function based on the
// argument type for json input
static struct cfg_set_item* parse_json_arg(const struct cfg_parser* parser,
                                           struct json_object* jobj) {
    switch (parser->type) {
    case CFG_INT:
        return parse_int_json(parser, jobj);
    case CFG_WORD:
        return parse_word_json(parser, jobj);
    case CFG_BYTE:
        return parse_byte_json(parser, jobj);
    case CFG_STRING:
        return parse_string_json(parser, jobj);
    case CFG_IPV6_ADDRESS:
        return parse_ipv6_addr_json(parser, jobj);
    case CFG_HW_ADDRESS:
        return parse_mac_json(parser, jobj);
    case CFG_BOOL:
        return parse_bool_json(parser, jobj);
    default:
        return NULL;
    }
}
#endif

// Generic function used to create a set item given a parser and a pointer to
// the raw value.
struct cfg_set_item* cfg_set_item_create(const struct cfg_parser* parser,
                                         void* data) {
    struct cfg_set_item* item = NULL;

    // If data is NULL there is no need to continue
    if (!data) {
        return NULL;
    }

    item = malloc(sizeof(struct cfg_set_item));

    if (item) {
        item->value.type = parser->type;
        item->name = parser->long_name;
        switch (item->value.type) {
        // If the values type is CFG_STRING, CFG_IPV6_ADDRESS, or CFG_HW_ADDRESS
        // the process is the same.
        case CFG_STRING:
        case CFG_IPV6_ADDRESS:
        case CFG_HW_ADDRESS:
            item->value.data = data;
            break;
        case CFG_INT:
            item->value.integer = *(int*)data;
            break;
        case CFG_WORD:
            item->value.word = *(u_int16_t*)data;
            break;
        case CFG_BYTE:
            item->value.byte = *(u_int8_t*)data;
            break;
        case CFG_BOOL:
            item->value.boolean = *(bool*)data;
            break;
        default:
            free(item);
            return NULL;
        }
    }
    return item;
}

void cfg_set_item_free(struct cfg_set_item* item) {
    if (item) {
        // Remember to free the memory allocated to value.data if (and only if)
        // the value is of type CFG_STRING, CFG_IPV6_ADDRESS, or CFG_HW_ADDRESS
        if (item->value.type &
            (CFG_STRING | CFG_IPV6_ADDRESS | CFG_HW_ADDRESS)) {
            free(item->value.data);
            item->value.data = NULL;
        }
        free(item);
        item = NULL;
    }
}

// Helper function used by qsort to sort the btree of cfg_set_items
static int cfg_set_cmp(const void* elem1, const void* elem2) {
    const struct cfg_set_item* item1 = *(const struct cfg_set_item**)elem1;
    const struct cfg_set_item* item2 = *(const struct cfg_set_item**)elem2;
    return strcmp(item2->name, item1->name);
}

// Find the correct parser for input given the short hand version
static const struct cfg_parser*
find_parser_char(const char arg, size_t sz, const struct cfg_parser* parsers) {
    const struct cfg_parser* iter;
    const struct cfg_parser* end = parsers + sz;
    for (iter = parsers; iter < end; ++iter) {
        if (arg == iter->short_name) {
            return iter;
        }
    }
    return NULL;
}

// Find the correct parser for input given the long hand version
static const struct cfg_parser*
find_parser_str(const char* arg, size_t len, size_t sz,
                const struct cfg_parser* parsers) {
    const struct cfg_parser* iter;
    const struct cfg_parser* end = parsers + sz;
    if (!len) {
        len = strlen(arg);
    }
    for (iter = parsers; iter < end; ++iter) {
        if (strncmp(arg, iter->long_name, len) == 0) {
            return iter;
        }
    }
    return NULL;
}

int cfg_set_add(struct cfg_map* set, struct cfg_set_item* item) {
    struct cfg_set_item** tmp;
    // Ensure the memory allocated to the cfg_set map is big enough for the
    // old data and the new item.
    if (set->len + 1 > set->cap) {
        tmp = malloc(sizeof(struct cfg_set_item*) * (set->cap + 10));
        if (tmp) {
            memcpy(tmp, set->map, set->len * sizeof(struct cfg_set_item*));
            free(set->map);
            set->map = tmp;
            set->cap += 10;
        } else {
            return -1;
        }
    }
    set->map[set->len] = item;
    ++set->len;
    return 0;
}

// Helper function used to parse longhand arguments. This is a bit more
// complicated than it needs to be, due to the initial implementation
// of sylkie. The initial implementation did not require that argments
// to longhang options be of the form --option=argument, but instead
// --option argument. As a result, we also need to pass the next string
// in argv to this function
static int cfg_set_parse_long(struct cfg_map* map,
                              const struct cfg_parser* parsers, size_t sz,
                              const char* arg, const char* next,
                              bool* next_consumed) {
    char* iter = NULL;
    const struct cfg_parser* parser = NULL;
    struct cfg_set_item* item = NULL;
    static bool true_value = true;
    bool used_equals = false;
    int len = 0;
    if ((iter = strchr(arg, '=')) != NULL) {
        len = iter - arg;
        next = iter + 1;
        used_equals = true;
    }
    if ((parser = find_parser_str(arg, len, sz, parsers))) {
        // We don't need to parse an argument for boolean options
        if (parser->type != CFG_BOOL) {
            if (next) {
                item = parse_cmdline_arg(parser, next);
                if (item) {
                    if (cfg_set_add(map, item)) {
                        fprintf(stderr, "ERROR: No memory\n");
                        free(item);
                        return -1;
                    }
                } else {
                    fprintf(stderr, "ERROR: %s invalid argument for %s\n", next,
                            parser->long_name);
                    return -1;
                }
            } else {
                fprintf(stderr, "ERROR: --%s requires an argument\n", arg);
                return -1;
            }
            if (!used_equals) {
                *next_consumed = true;
            }
        } else {
            item = cfg_set_item_create(parser, &true_value);
            if (item) {
                if (cfg_set_add(map, item)) {
                    fprintf(stderr, "ERROR: No memory\n");
                    free(item);
                    return -1;
                }
            } else {
                fprintf(stderr, "ERROR: Could not create value for %s\n", arg);
                return -1;
            }
        }
    } else {
        fprintf(stderr, "Unknown argument: %s\n", arg);
        return -1;
    }

    return 0;
}

// Helper function used to parse shorthand arguments. This is a bit more
// complicated, as combinations (e.g -vh, -ip) should be expected. -vh
// would be a legal combination, as neither of them requires an argument.
// However -ip would be invalid, as both of them require an argument and
// it is therefore unclear who owns the given argument.
static int cfg_set_parse_short(struct cfg_map* map,
                               const struct cfg_parser* parsers, size_t sz,
                               const char* arg, size_t len, const char* next,
                               bool* next_consumed) {
    int i;
    const struct cfg_parser* parser = NULL;
    struct cfg_set_item* item = NULL;
    static bool true_value = true;
    // Flag value indicating the argument has been consumed by one of the
    // options
    for (i = 0; i < len; ++i) {
        if ((parser = find_parser_char(arg[i], sz, parsers))) {
            // We don't need to parse an argument for boolean options
            if (parser->type != CFG_BOOL) {
                if (*next_consumed) {
                    fprintf(stderr, "ERROR: Invalid combination \"-%s\". -%c"
                                    "requires an argument\n",
                            arg, arg[i]);
                    return -1;
                } else if (!next) {
                    fprintf(stderr, "ERROR: -%c requires an argument\n",
                            arg[i]);
                    return -1;
                }
                *next_consumed = true;
                item = parse_cmdline_arg(parser, next);
                if (item) {
                    if (cfg_set_add(map, item)) {
                        free(item);
                        fprintf(stderr, "ERROR: No memory\n");
                        return -1;
                    }
                } else {
                    fprintf(stderr, "ERROR: %s invalid argument for %s\n", next,
                            parser->long_name);
                    return -1;
                }
            } else {
                item = cfg_set_item_create(parser, &true_value);
                if (item) {
                    if (cfg_set_add(map, item)) {
                        free(item);
                        fprintf(stderr, "ERROR: No memory\n");
                        return -1;
                    }
                } else {
                    fprintf(stderr, "ERROR: %s invalid argument for %s\n", next,
                            parser->long_name);
                    return -1;
                }
            }
            item = NULL;
        } else {
            fprintf(stderr, "ERROR: Unknown argument %c\n", arg[i]);
            return -1;
        }
    }
    return 0;
}

// Initialize a cfg_set for json parsing
#ifdef BUILD_JSON
int cfg_set_init_json(struct cfg_set* set, struct json_object* jobj) {
    const struct cfg_parser* parser = NULL;
    struct cfg_set_item* item = NULL;

    // TODO(dlrobertson): Allow this to work
    set->subcmds = NULL;
    set->subcmds_sz = 0;

    // TODO(dlrobertson): This is an entirely random number. Find a
    // more clever way of doing this or prove this is a good value.
    set->options.map = malloc(sizeof(struct cfg_set_item*) * 10);
    if (!set->options.map) {
        return -1;
    }

    // Ensure to actually set the length and capacity values
    set->options.len = 0;
    set->options.cap = 5;
    bzero(set->options.map, 5);

    // Loop through the json object and parse the associated value.
    json_object_object_foreach(jobj, key, val) {
        if ((parser = find_parser_str(key, 0, set->parsers_sz, set->parsers))) {
            item = parse_json_arg(parser, val);
            if (item) {
                cfg_set_add(&set->options, item);
            } else {
                fprintf(stderr, "ERROR: %s invalid argument for %s\n",
                        json_object_to_json_string(val), parser->long_name);
                return -1;
            }
        }
    }

    // The options.map is a btree. Remember to sort the map by the arguments
    // associated name before we finish
    //
    // TODO(dlrobertson): Look into using a merge sort here
    qsort(set->options.map, set->options.len, sizeof(struct cfg_set_item*),
          cfg_set_cmp);
    return 0;
}
#endif

// Initialize a cfg_set for command line parsing
int cfg_set_init_cmdline(struct cfg_set* set, size_t argc, const char** argv) {
    bool next_consumed = false;
    const char** iter = argv;
    const char** end = argv + argc;
    const char* arg;
    size_t len = (argc) ? (argc / 2) : 1;

    // It is likely the user is using --arg <value> or -a <value>. As aresult,
    // it is likely we need half of argc * the size of an item
    set->options.map = malloc(sizeof(struct cfg_set_item*) * len);
    if (!set->options.map) {
        return -1;
    }

    // Remember to actually set the length and capacity
    set->options.len = 0;
    set->options.cap = len;
    bzero(set->options.map, len);

    // Loop through each argument and parse the input. This gets a bit tricke
    // because each iteration could either result in iter += 1 or iter += 2.
    // This allows us to have a nice user interface with a flexible syntax,
    // but comes at the cost of more dificult parsing
    while (iter < end) {
        arg = *iter;
        len = strlen(arg);
        if (len > 1 && arg[0] == '-') {
            if (arg[1] != '-') {
                ++iter;
                if (cfg_set_parse_short(&set->options, set->parsers,
                                        set->parsers_sz, ++arg, --len, *iter,
                                        &next_consumed)) {
                    cfg_set_free(set);
                    return -1;
                }
            } else if (len > 2) {
                ++iter;
                arg += 2;
                if (cfg_set_parse_long(&set->options, set->parsers,
                                       set->parsers_sz, arg, *iter,
                                       &next_consumed)) {
                    cfg_set_free(set);
                    return -1;
                }
            } else {
                fprintf(stderr, "Unknown argument: %s\n", arg);
                cfg_set_free(set);
                return -1;
            }
        } else {
            fprintf(stderr, "Unknown argument: %s\n", arg);
            cfg_set_free(set);
            return -1;
        }
        if (next_consumed) {
            ++iter;
            next_consumed = false;
        }
    }

    // The options.map is a btree. Remember to sort the map by the arguments
    // associated name before we finish
    //
    // TODO(dlrobertson): Look into using a merge sort here
    qsort(set->options.map, set->options.len, sizeof(struct cfg_set_item*),
          cfg_set_cmp);
    return 0;
}

// Core function used to conduct the binary search
static const struct cfg_set_item*
cfg_set_find_inner(struct cfg_set_item** begin, struct cfg_set_item** end,
                   struct cfg_set_item** cur, const char* name) {
    int res = 0;
    int diff = 0;

    // Check if we have exhaused the map
    if (cur < end && cur >= begin) {
        res = strcmp((*cur)->name, name);
        if (!res) {
            // We found the item
            return *cur;
        } else if (res < 0 && cur > begin) {
            // We guessed high. cur is now the end and begin is still the
            // beginning
            diff = ((cur - begin) / 2) + 1;
            return cfg_set_find_inner(begin, cur, cur - diff, name);
        } else if (res > 0 && cur < end) {
            // We guessed low. cur is not the beginning and end is still the
            // end
            diff = (end - cur) / 2;
            if (!diff) {
                diff = 1;
            }
            return cfg_set_find_inner(cur, end, cur + diff, name);
        } else {
            // We guessed high and we cannot go lower, or we guessed
            // low and we cannot go higher.
            return NULL;
        }
    } else {
        // These aren't the droids you're looking for
        return NULL;
    }
}

const struct cfg_set_item* cfg_set_find(const struct cfg_set* set,
                                        const char* name) {
    size_t mid;
    if (set->options.len) {
        // We already know there is at least one item, so it is okay if mid is 0
        mid = set->options.len / 2;
        return cfg_set_find_inner(set->options.map,
                                  set->options.map + set->options.len,
                                  set->options.map + mid, name);
    } else {
        return NULL;
    }
}

// Essentially the same thing as cfg_set_find, just with a bit of iceing on top
// of it
int cfg_set_find_type(const struct cfg_set* set, const char* name,
                      enum cfg_value_type type, void* data) {
    const struct cfg_set_item* item = cfg_set_find(set, name);
    if (item) {
        if (item->value.type == type) {
            switch (item->value.type) {
            case CFG_STRING:
            case CFG_IPV6_ADDRESS:
            case CFG_HW_ADDRESS:
                *(void**)data = item->value.data;
                break;
            case CFG_INT:
                *(int*)data = item->value.integer;
                break;
            case CFG_WORD:
                *(u_int16_t*)data = item->value.word;
                break;
            case CFG_BYTE:
                *(u_int8_t*)data = item->value.byte;
                break;
            case CFG_BOOL:
                *(bool*)data = item->value.boolean;
                break;
            default:
                return -1;
            }
            return 0;
        } else {
            return -1;
        }
    } else {
        return -1;
    }
}

// Convert a cfg_set into nice help text. General format is
//
// Usage: <usage string>
//
// <summary string>
//
// Avalable options:
//
// -<c> | --<string>    <Helpful parser text>
//
void cfg_set_usage(const struct cfg_set* set, FILE* output) {
    int i, longest = 0, tmp = 0;
    if (set->usage) {
        fprintf(output, "Usage: %s\n\n", set->usage);
    }
    if (set->summary) {
        fprintf(output, "%s\n\n", set->summary);
    }
    if (set->subcmds_sz) {
        fprintf(output, "Available subcommands:\n");
        for (i = 0, longest = 0; i < set->subcmds_sz; ++i) {
            if ((tmp = strlen(set->subcmds[i].long_name)) > longest) {
                longest = tmp;
            }
        }
        // TODO(dlrobertson): Snag the subcommands set summary and print here
        for (i = 0; i < set->subcmds_sz; ++i) {
            fprintf(output, " %s | %-*s\n", set->subcmds[i].short_name, longest,
                    set->subcmds[i].long_name);
        }
        fprintf(output, "\n");
    }
    if (set->parsers_sz) {
        for (i = 0, longest = 0; i < set->parsers_sz; ++i) {
            if ((tmp = strlen(set->parsers[i].long_name)) > longest) {
                longest = tmp;
            }
        }
        fprintf(output, "Available options:\n");
        for (i = 0; i < set->parsers_sz; ++i) {
            // TODO(dlrobertson): If a given parser has a super long string
            // this looks wierd. Find a way to handle long parser usage strings
            // better.
            fprintf(output, " -%c | --%-*s  %s\n", set->parsers[i].short_name,
                    longest, set->parsers[i].long_name, set->parsers[i].usage);
        }
    }
}

// Free a cfg_set
void cfg_set_free(struct cfg_set* set) {
    if (set && set->options.map) {
        size_t i;
        for (i = 0; i < set->options.len; ++i) {
            if (set->options.map[i]) {
                cfg_set_item_free(set->options.map[i]);
            }
        }
        free(set->options.map);
    }
}
