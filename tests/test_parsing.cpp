#include <gtest/gtest.h>

extern "C" {
#include <utils.h>
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
