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

/**
 * \defgroup sylkie_nd NDP packet creation helpers
 * \ingroup libsylkie
 * @{
 */

/**
 * \brief Create a sylkie_packet containing a Neighbor Advertisement
 * \param eth_src Source ethernet address
 * \param eth_dst Destination ethernet address
 * \param ip_src Source IPv6 address
 * \param ip_dst Destination IPv6 address
 * \param tgt_ip Target IPv6 address (address to be spoofed)
 * \param tgt_tw Hardware address to be the new value in the neighbor cache
 * \param err Pointer to sylkie_error
 *
 * @see sylkie_packet
 */
struct sylkie_packet* sylkie_neighbor_advert_create(
    const u_int8_t eth_src[ETH_ALEN], const u_int8_t eth_dst[ETH_ALEN],
    struct in6_addr* ip_src, struct in6_addr* ip_dst, struct in6_addr* tgt_ip,
    const u_int8_t tgt_hw[ETH_ALEN], enum sylkie_error* err);

/**
 * \brief Create a sylkie_packet containing a Router Advertisement
 * \param eth_src Source ethernet address
 * \param eth_dst Destination ethernet address
 * \param ip_src Source IPv6 address
 * \param ip_dst Destination IPv6 address
 * \param tgt_ip Target IPv6 address (address to be spoofed)
 * \param prefix Link prefix
 * \param lifetime Lifetime parameter for the router
 * \param tgt_tw Hardware address to be the new value in the neighbor cache
 * \param err Pointer to sylkie_error
 *
 * NB: When the lifetime parameter is set to 0 the router indicated
 * by tgt_ip will be removed from the targetted hosts list of default
 * route. If the lifetime is >0, the router indicated by tgt_ip will
 * be added or updated in the targetted hosts list of default routes.
 *
 * @see sylkie_packet
 */
struct sylkie_packet* sylkie_router_advert_create(
    const u_int8_t eth_src[ETH_ALEN], const u_int8_t eth_dst[ETH_ALEN],
    struct in6_addr* ip_src, struct in6_addr* ip_dst, struct in6_addr* tgt_ip,
    u_int8_t prefix, u_int16_t lifetime, const u_int8_t tgt_hw[ETH_ALEN],
    enum sylkie_error* err);

/**
 * \brief Convert the sylkie_packet into a sylkie_buffer
 * \param buf Destination buffer the packet bytes will be added to
 * \param node Souce node of a sylkie_proto_node
 *
 * NB: This function is used by sylkie_packet to ensure the icmpv6
 * checksum is correctly set. In a future version this will be
 * a private function.
 *
 * @see sylkie_packet
 */
enum sylkie_error sylkie_icmpv6_to_buffer(struct sylkie_buffer* buf,
                                          const struct sylkie_proto_node* node);

/// @} end of doxygen sylkie_nd group

#endif
