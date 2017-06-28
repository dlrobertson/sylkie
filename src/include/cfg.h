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

//
// cfg structures and methods
//

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

struct cfg_value {
    union {
        void*     data;
        int       integer;
        u_int16_t word;
        u_int8_t  byte;
        bool      boolean;
    };
    enum cfg_value_type type;
};

struct cfg_set_item {
    const char* name;
    struct cfg_value value;
};

struct cfg_set;

struct cfg_parser {
    char short_name;
    const char* long_name;
    enum cfg_value_type type;
    const char* usage;
};

struct cfg_map {
    struct cfg_set_item** map;
    size_t len;
    size_t cap;
};

struct cfg_set {
    const char* usage;
    const char* summary;
    const struct cfg_parser* parsers;
    size_t parsers_sz;
    struct cfg_map options;
};

struct cfg_set_item* cfg_set_item_create(const struct cfg_parser* parser, void* data);

void cfg_set_item_free(struct cfg_set_item* item);

int cfg_set_init_cmdline(struct cfg_set* set, size_t argc, const char** argv);

#ifdef BUILD_JSON
int cfg_set_init_json(struct cfg_set* set, struct json_object* jobj);
#endif

const struct cfg_set_item* cfg_set_find(const struct cfg_set* set, const char* name);

int cfg_set_find_type(const struct cfg_set* set, const char* name, enum cfg_value_type type,
                      void* data);

// Function used to pring the help text given the parsers provided to cfg_set_init
void cfg_set_usage(const struct cfg_set* set, FILE* output);

// Free a given cfg_set item. The stored object only
// needs to be freed if the type is a CFG_DATA
void cfg_set_free(struct cfg_set* set);

// Specific parsers frequently used by sylkie

#endif
