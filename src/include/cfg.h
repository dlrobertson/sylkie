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

#ifndef SYLKIE_SRC_INCLUDE_CFG_H
#define SYLKIE_SRC_INCLUDE_CFG_H

#include <sys/types.h>

#include <stdbool.h>

#include <sylkie_config.h>

#ifdef BUILD_JSON
#include <json-c/json.h>
#endif

/**
 * \defgroup cfg cfg_set structures and methods
 * \ingroup sylkie
 * @{
 */

/**
 * \brief Valid types allowed for arguments
 */
enum cfg_value_type {
  CFG_INT = 0x0,
  CFG_WORD = 0x1,
  CFG_BYTE = 0x2,
  CFG_STRING = 0x4,
  CFG_IPV6_ADDRESS = 0x8,
  CFG_HW_ADDRESS = 0x10,
  CFG_BOOL = 0x20,
  CFG_INVALID = 0x40,
};

/**
 * \brief Define the interface for providing a given argument to a command
 */
struct cfg_parser {
  /// the single character version of the option
  char short_name;
  /// the string version of the option
  const char *long_name;
  /// the type of the value expected
  enum cfg_value_type type;
  /// helpful text describing how to use the argument
  const char *usage;
};

/**
 * \brief Template used to parse arguments
 */
struct cfg_template {
  /// the available arguments
  const struct cfg_parser *parsers;
  /// number of available arguments
  const size_t parsers_sz;
  /// usage string
  const char *usage;
  /// summary string
  const char *summary;
  /// available subcommands
  struct cfg_subcmd *subcmds;
  /// length of available subcommands
  size_t subcmds_sz;
};

// TODO(dlrobertson): Make this useful. Currently this is only used to
// ensure everything is documented in the usage fn
/**
 * \brief Define the interface for a given subcommand
 *
 * NB: This feature is currently under development and is currently only
 * used to ensure the usage function prints correctly
 */
struct cfg_subcmd {
  /// the shorthand version of a subcommand
  const char *short_name;
  /// the longhandhand version of a subcommand
  const char *long_name;
};

/**
 * \brief Represent the value of an argument provided to a command
 *
 * A value may be any one of cfg_value_type.
 */
struct cfg_value {
  union {
    void *data;
    int integer;
    u_int16_t word;
    u_int8_t byte;
    bool boolean;
  };
  enum cfg_value_type type;
};

/**
 * \brief An argument provided to a command.
 *
 * This structure includes the name of the argument and the arguments value.
 *
 * NB: The items are keyed by the long_name member of the cfg_parser struct
 * __not__ the short_name.
 */
struct cfg_set_item {
  const char *name;
  struct cfg_value value;
};

/**
 * \brief B-tree of user arguments and their associated values
 *
 * A container structure for a B-Tree containing a given number of arguments
 * and their associated values.
 */
struct cfg_map {
  struct cfg_set_item **map;
  size_t len;
  size_t cap;
};

/**
 * \brief A set of user provided arguments
 *
 * The main structure a user uses to maintain state while using the cfg_set
 * argument parsing framework
 */
struct cfg_set {
  const struct cfg_template *templt;
  struct cfg_map options;
};

/**
 * \brief Create a cfg_set_item from a parser and a pointer to the value
 */
struct cfg_set_item *cfg_set_item_create(const struct cfg_parser *parser,
                                         void *data);

/**
 * \brief Free any allocated resources associated with an item.
 *
 * This is at a minumum sizeof(struct cfg_set_item), but may also be the
 * size of the data contained depending on the value type.
 */
void cfg_set_item_free(struct cfg_set_item *item);

/**
 * \brief Populate the cfg_set map
 *
 * Populate the cfg_set map with the user provided arguments and values from
 * argv/argc.
 */
int cfg_set_init_cmdline(struct cfg_set *set, const struct cfg_template *tmplt,
                         size_t argc, const char **argv);

#ifdef BUILD_JSON
/**
 * \brief Populate the cfg_set map
 *
 * Populate the cfg_set map with the user provided arguments and values from
 * the given json_object.
 *
 * Note: Only available if compiled with json support.
 */
int cfg_set_init_json(struct cfg_set *set, const struct cfg_template *tmplt,
                      struct json_object *jobj);
#endif

/**
 * \brief Search the cfg_set for an item and return it
 *
 * Search the cfg_sets btree by name and return a pointer to the entire item.
 * When a lookup fails, the returned value is NULL
 */
const struct cfg_set_item *cfg_set_find(const struct cfg_set *set,
                                        const char *name);

/**
 * \brief Search the cfg_set for an item and type
 *
 * Search the cfg_sets btree by name and type. If a match is found, set the
 * memory pointed to by the data argument to the given value. If no match
 * is found, do not mutate the data argument, but rather return -1
 */
int cfg_set_find_type(const struct cfg_set *set, const char *name,
                      enum cfg_value_type type, void *data);

/**
 * \brief Print the help text for the template provided to cfg_set_init
 */
void cfg_set_usage(const struct cfg_set *set, FILE *output);

/**
 * \brief Free a given cfg_set item.
 *
 * Note: The stored object only needs to be freed if the type is a CFG_DATA
 */
void cfg_set_free(struct cfg_set *set);

/**
 * \brief Print the help text for the template provided to cfg_set_init
 */
void cfg_template_usage(const struct cfg_template *templt, FILE *output);

// @} end of doxygen cfg group

#endif
