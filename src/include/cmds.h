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

#ifndef SYLKIE_SRC_INCLUDE_CMDS_H
#define SYLKIE_SRC_INCLUDE_CMDS_H

#include <netinet/if_ether.h>
#include <netinet/ip6.h>

#include <packet.h>
#include <utils.h>

struct packet_command {
  struct sylkie_sender *sender;
  struct sylkie_packet *pkt;
  int timeout;
  int repeat;
};

GENERIC_LIST_FORWARD(struct packet_command *, pkt_cmd_list);

enum listen_action_type {
  ACTION_PRINT,
  ACTION_IPC_COMMAND,
};

struct listen_action {
  enum listen_action_type type;
  union {
    struct ipc_command * ipc_command;
    struct {
      const char* data;
      size_t len;
    } print_command;
  } action;
};

struct listen_command {
  struct sylkie_sender *sender;
  struct in6_addr *src;
  struct in6_addr *dst;
  char* iface;
  int timeout;
  struct listen_action *(*action)(const struct ethhdr* eth,
                                  const struct ip6_hdr* ip6,
                                  const u_int8_t *packet, size_t len);
};

struct known_router {
  struct in6_addr r_addr;
  struct in6_addr s_addr;
};

GENERIC_LIST_FORWARD(struct known_router *, known_routers);

struct listen_command *listen_command_create(struct sylkie_sender *sender,
                                             struct in6_addr *src,
                                             struct in6_addr *dst);

void listen_command_free(struct listen_command *cmd);

GENERIC_LIST_FORWARD(struct listen_command *, lst_cmd_list);

struct command_lists {
  struct pkt_cmd_list *pkt_cmds;
  struct lst_cmd_list *lst_cmds;
};

enum sylkie_command_type {
  SYLKIE_CMD_PACKET,
  SYLKIE_CMD_LISTEN,
};

int command_lists_add(struct command_lists *lists,
                      enum sylkie_command_type type, void *cmd);

void command_lists_free(struct command_lists *lists);
#endif
