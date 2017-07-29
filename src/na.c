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

#include <nd.h>
#include <sender.h>

#include <arpa/inet.h>
#include <unistd.h>

#include <sylkie_config.h>

#ifdef BUILD_JSON
#include <json-c/json.h>
#endif

#include <cfg.h>
#include <utils.h>

static struct cfg_parser parsers[] = {
    {'h', "help", CFG_BOOL, "print helpful usage information"},
    {'i', "interface", CFG_STRING,
     "network interface that will be used to send packets"},
    {'s', "src-mac", CFG_HW_ADDRESS, "source address for the ethernet frame"},
    {'d', "dst-mac", CFG_HW_ADDRESS,
     "destination address for the ethernet frame"},
    {'t', "target-mac", CFG_HW_ADDRESS,
     "link layer address used for the target address "
     "option of the advertisement"},
    {'S', "src-ip", CFG_IPV6_ADDRESS, "source ipv6 address in IPv6 header"},
    {'D', "dst-ip", CFG_IPV6_ADDRESS,
     "destination ipv6 address in IPv6 header"},
    {'T', "target-ip", CFG_IPV6_ADDRESS,
     "target address of the Neighbor Advertisement"},
    {'r', "repeat", CFG_INT, "send the packet <num> times"},
    {'z', "timeout", CFG_INT,
     "wait <seconds> before sending the packet agein"}};
static size_t parsers_sz = sizeof(parsers) / sizeof(struct cfg_parser);

int inner_do_na(const struct cfg_set* set) {
    int retval = -1, prefix = 64, timeout = 1, repeat = 1, i = 0;
    const char* iface_name;
    const u_int8_t* dst_mac = NULL;
    const u_int8_t* src_mac = NULL;
    const u_int8_t* tgt_mac = NULL;
    struct in6_addr* dst_addr;
    struct in6_addr* src_addr;
    struct in6_addr* tgt_addr;
    enum sylkie_error err = SYLKIE_INVALID_ERR;
    struct sylkie_sender* sender = NULL;
    struct sylkie_packet* pkt = NULL;

    // Required input
    if (cfg_set_find_type(set, "interface", CFG_STRING, &iface_name)) {
        fprintf(stderr, "Must provide an interface to use.\n");
        cfg_set_usage(set, stderr);
        return -1;
    } else if (cfg_set_find_type(set, "dst-mac", CFG_HW_ADDRESS, &dst_mac)) {
        fprintf(stderr, "Must provide a destination mac address.\n");
        cfg_set_usage(set, stderr);
        return -1;
    } else if (cfg_set_find_type(set, "dst-ip", CFG_IPV6_ADDRESS, &dst_addr)) {
        fprintf(stderr, "Must provide a destination ip address.\n");
        cfg_set_usage(set, stderr);
        return -1;
    } else if (cfg_set_find_type(set, "src-ip", CFG_IPV6_ADDRESS, &src_addr)) {
        fprintf(stderr, "Must provide a source ip address.\n");
        cfg_set_usage(set, stderr);
        return -1;
    }

    sender = sylkie_sender_init(iface_name, &err);

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
        return -1;
    }

    if (!sender) {
        fprintf(stderr,
                "Failed to create sender. Please consider submitting a bug"
                " report at https://github.com/dlrobertson/sylkie\n");
        return -1;
    }

    if (cfg_set_find_type(set, "src-mac", CFG_HW_ADDRESS, &src_mac)) {
        src_mac = sender->addr.sll_addr;
    }

    if (cfg_set_find_type(set, "target-mac", CFG_HW_ADDRESS, &tgt_mac)) {
        tgt_mac = src_mac;
    }

    if (cfg_set_find_type(set, "target-ip", CFG_IPV6_ADDRESS, &tgt_addr)) {
        tgt_addr = src_addr;
    }

    pkt = sylkie_neighbor_advert_create(src_mac, dst_mac, src_addr, dst_addr,
                                        tgt_addr, tgt_mac, &err);

    if (!pkt) {
        fprintf(stderr, "%s: Could not create forged neighbor advert\n",
                sylkie_strerror(err));
        return -1;
    }

    cfg_set_find_type(set, "prefix", CFG_BYTE, &prefix);
    cfg_set_find_type(set, "timeout", CFG_INT, &timeout);
    cfg_set_find_type(set, "repeat", CFG_INT, &repeat);

    if (repeat <= 0) {
        while (1) {
            retval = sylkie_sender_send_packet(sender, pkt, 0, &err);
            if (err || retval < 0) {
                fprintf(stderr, "SYLKIE Error:%s\nErrno: %s",
                        sylkie_strerror(err), strerror(errno));
                retval = -1;
                break;
            }

            if (timeout) {
                sleep(timeout);
            }
        }
    } else {
        for (i = 0; i < repeat; ++i) {
            retval = sylkie_sender_send_packet(sender, pkt, 0, &err);
            if (err || retval < 0) {
                fprintf(stderr, "SYLKIE Error:%s\nErrno: %s",
                        sylkie_strerror(err), strerror(errno));
                retval = -1;
                break;
            }

            if (timeout) {
                sleep(timeout);
            }
        }
    }

    // Cleanup

    sylkie_packet_free(pkt);
    sylkie_sender_free(sender);
    return retval;
}

#ifdef BUILD_JSON
pid_t na_json(struct json_object* jobj) {
    int res = -1;
    pid_t pid = -1;
    struct cfg_set set = {
        .usage = "sylkie na [OPTIONS]",
        .summary =
            "Send ICMPv6 Neighbor Advertisement messages to the given address",
        .parsers = parsers,
        .parsers_sz = parsers_sz};

    res = cfg_set_init_json(&set, jobj);

    if (res) {
        fprintf(stderr, "Failed to initialize parsers\n");
        return -1;
    } else if (cfg_set_find(&set, "help")) {
        fprintf(stderr,
                "\"help\" is an invalid option for running sylkie from json\n");
        cfg_set_free(&set);
        return -1;
    }

    if ((pid = fork()) < 0) {
        cfg_set_free(&set);
        return -1;
    } else if (pid == 0) {
        if ((res = lockdown())) {
            fprintf(stderr, "Could not lock down the process\n");
        } else {
            res = inner_do_na(&set);
        }
        _exit(res);
    } else {
        cfg_set_free(&set);
        return pid;
    }
}
#endif

int na_cmdline(int argc, const char** argv) {
    int res = -1;
    struct cfg_set set = {
        .usage = "sylkie na [OPTIONS]",
        .summary =
            "Send ICMPv6 Neighbor Advertisement messages to the given address",
        .parsers = parsers,
        .parsers_sz = parsers_sz};

    if (argc < 1) {
        fprintf(stderr, "Too few arguments\n");
        cfg_set_usage(&set, stderr);
        return -1;
    }

    res = cfg_set_init_cmdline(&set, --argc, ++argv);

    if (res) {
        cfg_set_usage(&set, stderr);
        return -1;
    } else if (cfg_set_find(&set, "help")) {
        cfg_set_usage(&set, stdout);
        cfg_set_free(&set);
        return 0;
    } else {
        lockdown();
        res = inner_do_na(&set);
    }

    cfg_set_free(&set);

    return res;
}
