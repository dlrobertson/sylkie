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
