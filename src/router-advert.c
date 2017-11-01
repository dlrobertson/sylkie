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

#include <ndp.h>
#include <sender.h>
#include <sender_map.h>

#include <arpa/inet.h>
#include <unistd.h>

#include <sylkie_config.h>

#ifdef BUILD_JSON
#include <json-c/json.h>
#endif

#include <cfg.h>
#include <cmds.h>

const struct cfg_parser ra_parsers[] = {
    {'h', "help", CFG_BOOL, "print helpful usage information"},
    {'i', "interface", CFG_STRING,
     "network interface that will be used to send packets"},
    {'s', "src-mac", CFG_HW_ADDRESS, "source address for the ethernet frame"},
    {'d', "dst-mac", CFG_HW_ADDRESS,
     "destination address for the ethernet frame"},
    {'S', "src-ip", CFG_IPV6_ADDRESS, "source ipv6 address in IPv6 header"},
    {'D', "dst-ip", CFG_IPV6_ADDRESS,
     "destination ipv6 address in IPv6 header"},
    {'t', "target-mac", CFG_HW_ADDRESS,
     "link layer address used for the target address "
     "option of the advertisement"},
    {'R', "router-ip", CFG_IPV6_ADDRESS, "ipv6 address of the router to spoof"},
    {'p', "prefix", CFG_INT, "send the packet <num> times"},
    {'r', "repeat", CFG_INT, "send the packet <num> times"},
    {'l', "lifetime", CFG_WORD, "valid and preferred lifetime of router"},
    {'z', "timeout", CFG_INT, "wait <n> seconds before sending agein"}};

const struct cfg_template *generate_ra_template() {
  static const struct cfg_template ra_templt = {
      .usage = "sylkie ra [OPTIONS]",
      .summary =
          "Send ICMPv6 Router Advertisement messages to the given address",
      .parsers = ra_parsers,
      .parsers_sz = sizeof(ra_parsers) / sizeof(struct cfg_parser),
      .subcmds = NULL,
      .subcmds_sz = 0};
  return &ra_templt;
}

int ra_parse(struct sylkie_sender_map *ifaces, const struct cfg_set *set,
             struct command_lists *cmds) {
  int prefix = 64;
  enum sylkie_error err = SYLKIE_SUCCESS;
  struct sylkie_sender *sender = NULL;
  static const u_int8_t all_nodes_eth[] = {0x33, 0x33, 0x00, 0x00, 0x00, 0x01};
  static struct in6_addr all_nodes_ip = {
      .s6_addr = {0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00, 0x00, 0x00, 0x01}};
  char *iface_name = NULL;
  const u_int8_t *dst_mac = NULL;
  const u_int8_t *src_mac = NULL;
  const u_int8_t *tgt_mac = NULL;
  u_int16_t lifetime = 0;
  struct in6_addr *dst_addr = NULL;
  struct in6_addr *router_addr = NULL;
  struct in6_addr *src_addr = NULL;
  struct packet_command *cmd = malloc(sizeof(struct packet_command));

  if (!cmd) {
    fprintf(stderr, "No memory.\n");
    return -1;
  } else {
    cmd->pkt = NULL;
    cmd->sender = NULL;
    cmd->timeout = -1;
    cmd->repeat = 0;
  }

  if (cfg_set_find_type(set, "interface", CFG_STRING, &iface_name)) {
    fprintf(stderr, "Must provide an interface to use.\n");
    cfg_set_usage(set, stderr);
    free(cmd);
    return -1;
  }

  cfg_set_find_type(set, "dst-mac", CFG_HW_ADDRESS, &dst_mac);
  cfg_set_find_type(set, "dst-ip", CFG_IPV6_ADDRESS, &dst_addr);
  cfg_set_find_type(set, "src-ip", CFG_IPV6_ADDRESS, &src_addr);
  cfg_set_find_type(set, "router-ip", CFG_IPV6_ADDRESS, &router_addr);
  cfg_set_find_type(set, "lifetime", CFG_WORD, &lifetime);

  if (!dst_mac && !dst_addr) {
    dst_mac = all_nodes_eth;
    dst_addr = &all_nodes_ip;
  } else if (!(dst_mac && dst_addr)) {
    fprintf(stderr, "Must provide a destination mac and ip"
                    " address or none at all.\n");
    cfg_set_usage(set, stderr);
    free(cmd);
    return -1;
  }

  if (!router_addr) {
    fprintf(stderr, "Must provide a router ip address to spoof.\n");
    cfg_set_usage(set, stderr);
    free(cmd);
    return -1;
  } else if (!src_addr) {
    src_addr = router_addr;
  }

  sender = sylkie_sender_map_get_name(ifaces, iface_name);
  if (!sender) {
    sender = sylkie_sender_map_add(ifaces, iface_name, &err);
  }

  if (err) {
    switch (err) {
    case SYLKIE_EPERM:
      fprintf(stderr,
              "%s: This program makes heavy use of raw sockets which"
              " require uid=0. You can either build with -DENABLE_SETUID"
              " or you must run this as root.\n",
              sylkie_strerror(err));
      break;
    case SYLKIE_NO_DEVICE:
      fprintf(stderr, "%s: Could not find the device \"%s\"\n",
              sylkie_strerror(err), iface_name);
      break;
    default:
      fprintf(stderr, "%s\n", sylkie_strerror(err));
      break;
    }
    free(cmd);
    return -1;
  }

  cmd->sender = sender;

  if (!sender) {
    fprintf(stderr, "Failed to create sender. Please consider submitting a bug"
                    " report at https://github.com/dlrobertson/sylkie\n");
    free(cmd);
    return -1;
  }

  if (cfg_set_find_type(set, "src-mac", CFG_HW_ADDRESS, &src_mac)) {
    src_mac = sylkie_sender_addr(sender);
  }

  if (cfg_set_find_type(set, "target-mac", CFG_HW_ADDRESS, &tgt_mac)) {
    tgt_mac = src_mac;
  }

  cfg_set_find_type(set, "prefix", CFG_BYTE, &prefix);

  cmd->pkt =
      sylkie_router_advert_create(src_mac, dst_mac, src_addr, dst_addr,
                                  router_addr, prefix, lifetime, tgt_mac, &err);

  if (!cmd->pkt) {
    fprintf(stderr, "%s: Could not create forged router advert\n",
            sylkie_strerror(err));
    free(cmd);
    return -1;
  }

  cfg_set_find_type(set, "timeout", CFG_INT, &cmd->timeout);
  cfg_set_find_type(set, "repeat", CFG_INT, &cmd->repeat);

  if (command_lists_add(cmds, SYLKIE_CMD_PACKET, cmd)) {
    sylkie_packet_free(cmd->pkt);
    free(cmd);
    return -1;
  }

  return 0;
}
