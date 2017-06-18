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

#include <errors.h>
#include <nd.h>
#include <packet.h>

struct sylkie_packet* sylkie_packet_init(enum sylkie_request_type type,
                                         void* data) {
    struct sylkie_packet* pkt = malloc(sizeof(struct sylkie_packet));
    bzero(pkt, sizeof(struct sylkie_packet));
    pkt->type = type;
    pkt->data = data;
    return pkt;
}

void sylkie_packet_free(struct sylkie_packet* pkt) {
    if (pkt) {
        if (pkt->data) {
            switch (pkt->type) {
            case SYLKIE_PKT_NEIGH_DISC:
                sylkie_nd_packet_free(pkt->data);
                break;
            default:
                break;
            }
        }
        free(pkt);
        pkt = NULL;
    }
}
