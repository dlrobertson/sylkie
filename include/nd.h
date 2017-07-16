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

#ifndef SYLKIE_INCLUDE_ND_H
#define SYLKIE_INCLUDE_ND_H

#include <netinet/icmp6.h>
#include <netinet/if_ether.h>
#include <netinet/ip6.h>

#include "buffer.h"
#include "packet.h"
#include "proto_list.h"
#include "sender.h"

/*! \mainpage sylkie index page
 *
 * \section intro_sec Introduction
 *
 * This is the inner workings of the <a href="http://sylkie.io">sylkie</a>
 * command line tool.
 *
 * \section purpose_sec Purpose
 *
 * The documentation here is targeted at developers that are either
 * contributing to the tool, or are looking to do something more complex.
 * Most use cases can be covered by command line programs, but sometimes it
 * is necessary to directly use the shared lib.
 *
 * \defgroup libsylkie Core public functions and structures
 * \defgroup sylkie Private structures and functions used by main
 */

struct sylkie_packet* sylkie_neighbor_advert_create(
    const u_int8_t eth_src[ETH_ALEN], const u_int8_t eth_dst[ETH_ALEN],
    struct in6_addr* ip_src, struct in6_addr* ip_dst, struct in6_addr* tgt_ip,
    const u_int8_t tgt_hw[ETH_ALEN], enum sylkie_error* err);

struct sylkie_packet* sylkie_router_advert_create(
    const u_int8_t eth_src[ETH_ALEN], const u_int8_t eth_dst[ETH_ALEN],
    struct in6_addr* ip_src, struct in6_addr* ip_dst, struct in6_addr* tgt_ip,
    u_int8_t prefix, const u_int8_t tgt_hw[ETH_ALEN], enum sylkie_error* err);

enum sylkie_error sylkie_icmpv6_to_buffer(struct sylkie_buffer* buf,
                                          const struct sylkie_packet* pkt,
                                          const struct sylkie_proto_node* node);

#endif
