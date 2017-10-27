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

#include <packet.h>
#include <utils.h>

struct packet_command {
  struct sylkie_sender *sender;
  struct sylkie_packet *pkt;
  int timeout;
  int repeat;
};

GENERIC_LIST_FORWARD(struct packet_command *, pkt_cmd_list);

struct listen_command {
  struct sylkie_sender *sender;
  struct in6_addr* src;
  struct in6_addr* dst;
};

struct listen_command *listen_command_create(struct sylkie_sender *sender,
                                             struct in6_addr* src,
                                             struct in6_addr* dst);

void listen_command_free(struct listen_command* cmd);

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
                      enum sylkie_command_type type,
                      void* cmd);

void command_lists_free(struct command_lists *lists);
#endif
