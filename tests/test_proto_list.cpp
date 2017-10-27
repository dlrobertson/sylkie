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
#include <netinet/icmp6.h>
#include <netinet/if_ether.h>
#include <netinet/ip6.h>
#include <proto_list.h>
#include <strings.h>
}

TEST(proto_list, basic) {
  struct icmp6_hdr icmp6;
  struct sylkie_proto_list *lst = sylkie_proto_list_init();
  enum sylkie_error err;

  bzero(&icmp6, sizeof(struct icmp6_hdr));
  icmp6.icmp6_type = ND_NEIGHBOR_ADVERT;
  icmp6.icmp6_data8[0] = 0x20;
  err = sylkie_proto_list_add(lst, SYLKIE_ICMPv6, &icmp6,
                              sizeof(struct icmp6_hdr));
  ASSERT_EQ(err, SYLKIE_SUCCESS);
  sylkie_proto_list_free(lst);
}

TEST(proto_list, rm) {
  int i = 0;
  struct ethhdr eth;
  struct ip6_hdr ipv6;
  struct icmp6_hdr icmp6;
  struct sylkie_proto_node *node;
  struct sylkie_proto_list *lst = sylkie_proto_list_init();
  enum sylkie_error err;

  err = sylkie_proto_list_add(lst, SYLKIE_ETH, &eth, sizeof(struct ethhdr));
  ASSERT_EQ(err, SYLKIE_SUCCESS);

  err = sylkie_proto_list_add(lst, SYLKIE_IPv6, &ipv6, sizeof(struct ip6_hdr));
  ASSERT_EQ(err, SYLKIE_SUCCESS);

  err = sylkie_proto_list_add(lst, SYLKIE_ICMPv6, &icmp6,
                              sizeof(struct icmp6_hdr));
  ASSERT_EQ(err, SYLKIE_SUCCESS);

  err = sylkie_proto_list_add(lst, SYLKIE_DATA, NULL, 0);
  ASSERT_EQ(err, SYLKIE_SUCCESS);

  SYLKIE_HEADER_LIST_FOREACH (lst, node) {
    switch (i) {
    case 0:
      EXPECT_EQ(node->hdr.type, SYLKIE_ETH);
      break;
    case 1:
      EXPECT_EQ(node->hdr.type, SYLKIE_IPv6);
      break;
    case 2:
      EXPECT_EQ(node->hdr.type, SYLKIE_ICMPv6);
      break;
    case 3:
      EXPECT_EQ(node->hdr.type, SYLKIE_DATA);
      break;
    default:
      ASSERT_TRUE(false);
      break;
    }
    ++i;
  }

  ASSERT_EQ(i, 4);

  err = sylkie_proto_list_rm(lst, SYLKIE_IPv6);
  ASSERT_EQ(err, SYLKIE_SUCCESS);

  i = 0;
  SYLKIE_HEADER_LIST_FOREACH (lst, node) {
    switch (i) {
    case 0:
      EXPECT_EQ(node->hdr.type, SYLKIE_ETH);
      break;
    case 1:
      EXPECT_EQ(node->hdr.type, SYLKIE_ICMPv6);
      break;
    case 2:
      EXPECT_EQ(node->hdr.type, SYLKIE_DATA);
      break;
    default:
      ASSERT_TRUE(false);
      break;
    }
    ++i;
  }

  ASSERT_EQ(i, 3);

  sylkie_proto_list_free(lst);
}
