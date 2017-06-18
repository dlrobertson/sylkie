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

#include <stdlib.h>
#include <strings.h>

#include <nd.h>
#include <packet.h>

#include <stdio.h>

struct sylkie_packet* sylkie_nd_packet_create(
    const u_int8_t eth_src[ETH_ALEN], const u_int8_t eth_dst[ETH_ALEN],
    struct in6_addr* ip_src, struct in6_addr* ip_dst, struct icmp6_hdr* icmp6,
    struct sylkie_buffer* buf, enum sylkie_error* err) {
    struct sylkie_nd_packet* ndpkt = malloc(sizeof(struct sylkie_nd_packet));
    if (!ndpkt) {
        if (err) {
            *err = SYLKIE_NULL_INPUT;
        }
        return NULL;
    }

    bzero(ndpkt, sizeof(struct sylkie_nd_packet));

    memcpy(&ndpkt->eth.h_dest, eth_dst, ETH_ALEN);
    memcpy(&ndpkt->eth.h_source, eth_src, ETH_ALEN);
    ndpkt->eth.h_proto = htons(ETH_P_IPV6);

    ndpkt->ipv6.ip6_vfc = 0x60;
    ndpkt->ipv6.ip6_nxt = 0x3a;
    ndpkt->ipv6.ip6_hops = 0xff;
    ndpkt->ipv6.ip6_plen = buf ? htons(buf->len + sizeof(struct icmp6_hdr))
                               : sizeof(struct icmp6_hdr);

    memcpy(&ndpkt->ipv6.ip6_dst, ip_dst, sizeof(struct in6_addr));
    memcpy(&ndpkt->ipv6.ip6_src, ip_src, sizeof(struct in6_addr));

    memcpy(&ndpkt->icmp6, icmp6, sizeof(struct icmp6_hdr));

    ndpkt->buf = buf;

    if (err) {
        *err = SYLKIE_SUCCESS;
    }
    return sylkie_packet_init(SYLKIE_PKT_NEIGH_DISC, ndpkt);
}

struct sylkie_packet* sylkie_neighbor_advert_create(
    const u_int8_t eth_src[ETH_ALEN], const u_int8_t eth_dst[ETH_ALEN],
    struct in6_addr* ip_src, struct in6_addr* ip_dst, struct in6_addr* tgt_ip,
    const u_int8_t tgt_hw[ETH_ALEN], enum sylkie_error* err) {
    static const u_int8_t options[2] = {0x02, 0x01};
    struct icmp6_hdr icmpv6;
    struct sylkie_buffer* buf =
        sylkie_buffer_init(sizeof(struct in6_addr) + 2 + ETH_ALEN);

    sylkie_buffer_add(buf, tgt_ip, sizeof(struct in6_addr));

    sylkie_buffer_add(buf, options, sizeof(options));

    sylkie_buffer_add(buf, tgt_hw, ETH_ALEN);

    bzero(&icmpv6, sizeof(struct icmp6_hdr));

    icmpv6.icmp6_type = ND_NEIGHBOR_ADVERT;
    icmpv6.icmp6_data8[0] = 0x20;

    return sylkie_nd_packet_create(eth_src, eth_dst, ip_src, ip_dst, &icmpv6,
                                   buf, err);
}

struct sylkie_packet* sylkie_router_advert_create(
    const u_int8_t eth_src[ETH_ALEN], const u_int8_t eth_dst[ETH_ALEN],
    struct in6_addr* ip_src, struct in6_addr* ip_dst, struct in6_addr* tgt_ip,
    u_int8_t prefix, const u_int8_t tgt_hw[ETH_ALEN], enum sylkie_error* err) {
    struct icmp6_hdr icmpv6;
    struct sylkie_buffer* buf = sylkie_buffer_init(2 + ETH_ALEN + 8);
    static const u_int8_t source_options[2] = {0x01, 0x01};
    static const u_int8_t prefix_options[2] = {0x03, 0x04};
    bzero(&icmpv6, sizeof(struct icmp6_hdr));

    icmpv6.icmp6_type = ND_ROUTER_ADVERT;
    sylkie_buffer_add_value(buf, 0x00, 8);
    sylkie_buffer_add(buf, prefix_options, sizeof(prefix_options));
    sylkie_buffer_add_value(buf, prefix, 1);
    sylkie_buffer_add_value(buf, 0x00, 13);
    sylkie_buffer_add(buf, tgt_ip, sizeof(struct in6_addr));
    sylkie_buffer_add(buf, source_options, sizeof(source_options));
    sylkie_buffer_add(buf, tgt_hw, ETH_ALEN);

    return sylkie_nd_packet_create(eth_src, eth_dst, ip_src, ip_dst, &icmpv6,
                                   buf, err);
}

static void icmp_set_checksum(struct sylkie_nd_packet* ndpkt) {
    u_int32_t sum = 0;
    u_int16_t* tmp;
    int i;
    tmp = (u_int16_t*)&ndpkt->ipv6.ip6_src;
    for (i = 0; i < (sizeof(struct in6_addr) / 2); ++i) {
        sum += tmp[i];
    }

    tmp = (u_int16_t*)&ndpkt->ipv6.ip6_dst;
    for (i = 0; i < (sizeof(struct in6_addr) / 2); ++i) {
        sum += tmp[i];
    }

    sum += ndpkt->ipv6.ip6_plen;
    sum += htons(ndpkt->ipv6.ip6_nxt);

    tmp = (u_int16_t*)&ndpkt->icmp6;
    for (i = 0; i < (sizeof(struct icmp6_hdr) / 2); ++i) {
        sum += tmp[i];
    }

    if (ndpkt->buf) {
        tmp = (u_int16_t*)ndpkt->buf->data;
        for (i = 0; i < ndpkt->buf->len / 2; ++i) {
            sum += tmp[i];
        }

        if (ndpkt->buf->len % 2) {
            sum += ndpkt->buf->data[i];
        }
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += sum >> 16;

    ndpkt->icmp6.icmp6_cksum = ~sum;
}

struct sylkie_buffer* sylkie_nd_packet_to_buffer(struct sylkie_nd_packet* ndpkt,
                                                 enum sylkie_error* err) {
    struct sylkie_buffer* buf = sylkie_buffer_init(0);
    if (!ndpkt) {
        if (err) {
            *err = SYLKIE_NULL_INPUT;
        }
        return NULL;
    }

    icmp_set_checksum(ndpkt);

    sylkie_buffer_add(buf, &ndpkt->eth, sizeof(ndpkt->eth));
    sylkie_buffer_add(buf, &ndpkt->ipv6, sizeof(ndpkt->ipv6));
    sylkie_buffer_add(buf, &ndpkt->icmp6, sizeof(ndpkt->icmp6));
    if (ndpkt->buf) {
        sylkie_buffer_add(buf, ndpkt->buf->data, ndpkt->buf->len);
    }

    // Clear the cksum after converting to buffer. Theoretically we could
    // keep this around, but if a user changes a value, this should change
    //
    // TODO(drobertson): is this the right approach?
    ndpkt->icmp6.icmp6_cksum = 0;

    if (err) {
        *err = SYLKIE_SUCCESS;
    }
    return buf;
}

void sylkie_nd_packet_free(struct sylkie_nd_packet* ndpkt) {
    if (ndpkt) {
        if (ndpkt->buf) {
            sylkie_buffer_free(ndpkt->buf);
        }
        free(ndpkt);
        ndpkt = NULL;
    }
}
