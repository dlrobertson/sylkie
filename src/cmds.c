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

#include <cmds.h>

GENERIC_LIST_IMPL(struct packet_command *, pkt_cmd_list);

GENERIC_LIST_IMPL(struct listen_command *, lst_cmd_list);

struct listen_command *listen_command_create(struct sylkie_sender *sender,
                                             struct in6_addr* src,
                                             struct in6_addr* dst) {
  struct listen_command *cmd = malloc(sizeof(struct listen_command));
  if (cmd) {
    cmd->sender = sender;
    cmd->src = src;
    cmd->dst = dst;
  }
  return cmd;
}

void listen_command_free(struct listen_command* cmd) {
  if (cmd) {
    if (cmd->src) {
      free(cmd->src);
    }
    if (cmd->dst) {
      free(cmd->dst);
    }
  }
  free(cmd);
}

int command_lists_add(struct command_lists *lists,
                      enum sylkie_command_type type,
                      void* cmd) {
  void *tmp;
  if (lists) {
    switch (type) {
      case SYLKIE_CMD_PACKET:
        tmp = pkt_cmd_list_add(lists->pkt_cmds, cmd);
        break;
      case SYLKIE_CMD_LISTEN:
        tmp = lst_cmd_list_add(lists->lst_cmds, cmd);
        break;
      default:
        return -1;
    }
    return (tmp) ? 0 : -1;
  }
  return -1;
}

void command_lists_free(struct command_lists *lists) {
  if (lists) {
    if (lists->pkt_cmds) {
      pkt_cmd_list_free(lists->pkt_cmds);
    }
    if (lists->lst_cmds) {
      lst_cmd_list_free(lists->lst_cmds);
    }
  }
}

