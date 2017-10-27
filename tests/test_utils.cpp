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

#include <gtest/gtest.h>
#include <iostream>

extern "C" {
#include <utils.h>
}

GENERIC_LIST(int, int_lst);

TEST(generic_list, simple) {
  struct int_lst_item *item_42 = NULL, *item = NULL;
  struct int_lst *lst = int_lst_init();
  int i = 0;

  ASSERT_TRUE(lst);

  item = int_lst_add(lst, 0x22b);
  ASSERT_TRUE(item);

  item_42 = int_lst_add(lst, 0x2a);
  ASSERT_TRUE(item_42);

  item = int_lst_add(lst, 0x80);
  ASSERT_TRUE(item);

  int_lst_rm(lst, item_42);

  for (i = 0, item = lst->head; item; item = item->next, ++i) {
    ASSERT_NE(item->value, 0x2a);
  }

  ASSERT_EQ(i, 2);

  item = lst->tail;
  int_lst_rm(lst, lst->head);

  ASSERT_EQ(lst->head, item);
  int_lst_rm(lst, lst->tail);

  ASSERT_FALSE(lst->head);
  ASSERT_FALSE(lst->tail);

  int_lst_free(lst);
}

TEST(generic_list, larger_rm) {
  struct int_lst_item *item_42 = NULL, *item = NULL;
  struct int_lst *lst = int_lst_init();
  int i = 0;

  ASSERT_TRUE(lst);

  for (i = 0; i < 10; ++i) {
    item = int_lst_add(lst, i);
    ASSERT_TRUE(item);
    if (i == 5) {
      item_42 = int_lst_add(lst, 0x2a);
    }
  }

  int_lst_rm(lst, item_42);

  ASSERT_EQ(lst->head->value, 0);
  for (i = 0, item = lst->head; item; item = item->next, ++i) {
    ASSERT_NE(item->value, 0x2a);
  }
  ASSERT_EQ(i, 10);

  int_lst_free(lst);
}

TEST(parsing, parse_hwaddr) {
  u_int8_t eth[6];
  int res = parse_hwaddr("52:54:00:11:bf:3c", eth);
  ASSERT_EQ(res, 0);
  ASSERT_EQ(eth[0], 0x52);
  ASSERT_EQ(eth[1], 0x54);
  ASSERT_EQ(eth[2], 0x00);
  ASSERT_EQ(eth[3], 0x11);
  ASSERT_EQ(eth[4], 0xbf);
  ASSERT_EQ(eth[5], 0x3c);
}
