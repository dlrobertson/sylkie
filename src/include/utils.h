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

#ifndef SYLKIE_SRC_INCLUDE_UTILS_H
#define SYLKIE_SRC_INCLUDE_UTILS_H

#include <sys/types.h>

u_int8_t hex_char_to_byte(char ch);

int parse_hwaddr(const char *arg, u_int8_t *addr);

int lockdown(void);

// Growable buffer of T
#define GENERIC_LIST_FORWARD(T, name) \
  struct name ## _item { \
    T value; \
    struct name ## _item *prev; \
    struct name ## _item *next; \
  }; \
  struct name { \
    struct name ## _item *head; \
    struct name ## _item *tail; \
  }; \
  struct name *name ## _init(); \
  struct name ## _item *name ## _add(struct name *buf, T item); \
  void name ## _rm(struct name *buf, struct name ## _item *item); \
  void name ## _free(struct name *buf); \

#define GENERIC_LIST_IMPL(T, name) \
  struct name *name ## _init() { \
    struct name *buf = (struct name *)malloc(sizeof(struct name)); \
    buf->head = NULL; \
    buf->tail = NULL; \
    if (!buf) { \
      return NULL; \
    } else { \
      return buf; \
    } \
  } \
  struct name ## _item *name ## _add(struct name *buf, T item) { \
    struct name ## _item *lst_item = (struct name ## _item *)malloc(sizeof(struct name ## _item)); \
    if (lst_item) { \
      lst_item->value = item; \
      lst_item->next = NULL; \
      lst_item->prev = NULL; \
    } else { \
      return NULL; \
    } \
    if (buf->head) { \
      buf->tail->next = lst_item; \
      lst_item->prev = buf->tail; \
      buf->tail = lst_item; \
    } else { \
      buf->head = lst_item; \
      buf->tail = lst_item; \
    } \
    return lst_item; \
  } \
  void name ## _rm(struct name *buf, struct name ## _item *item) { \
    if (item == buf->head) { \
      if (item->next) { \
        buf->head = item->next; \
        buf->head->prev = NULL; \
      } else { \
        buf->head = NULL; \
        if (item == buf->tail) { \
          buf->tail = NULL; \
        } \
      } \
    } else if (item == buf->tail) { \
      if (item->prev) { \
        buf->tail = item->prev; \
        buf->tail->next = NULL; \
      } else { \
        buf->tail = NULL; \
      } \
    } else { \
      if (item->prev) { \
        item->prev->next = item->next; \
      } \
      if (item->next) { \
        item->next->prev = item->prev; \
      } \
    } \
    free(item); \
  } \
  void name ## _free(struct name *buf) { \
    struct name ## _item *iter = buf->head; \
    struct name ## _item *next = NULL; \
    if (iter) { \
      if (!iter->next) { \
        free(iter); \
      } else { \
        while (iter) { \
          next = iter->next; \
          free(iter); \
          iter = next; \
        } \
      } \
    } \
    free(buf); \
  }

#define GENERIC_LIST(T, name) \
  GENERIC_LIST_FORWARD(T, name) \
  GENERIC_LIST_IMPL(T, name)

#endif
