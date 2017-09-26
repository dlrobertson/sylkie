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

#ifndef SYLKIE_INCLUDE_SENDER_H
#define SYLKIE_INCLUDE_SENDER_H

#include <stdio.h>

#include <linux/if_packet.h>
#include <sys/types.h>

#include "buffer.h"
#include "errors.h"
#include "packet.h"

/**
 * \defgroup sender sylkie_sender structures and methods
 * \ingroup libsylkie
 * @{
 */

/**
 * \brief Generic structure used to send packets
 *
 * NB: sylkie_sender make heavy use of raw sockets.
 */
struct sylkie_sender;

/**
 * \memberof sylkie_sender
 *
 * \brief Initialize a sender for a given interface
 * \param iface Name of the interface to be used
 * \param err Pointer to sylkie_error to be set on error
 */
struct sylkie_sender *sylkie_sender_init(const char *iface,
                                         enum sylkie_error *err);

/**
 * \memberof sylkie_sender
 *
 * \brief Send a sylkie_buffer
 * \param sender sylkie_sender to be used to send the buffer
 * \param buf sylkie_buffer to be sent
 * \param flags Flags to be set on send
 * \param err Pointer to sylkie_error to be set on error
 */
int sylkie_sender_send_buffer(struct sylkie_sender *sender,
                              const struct sylkie_buffer *buf, int flags,
                              enum sylkie_error *err);

/**
 * \memberof sylkie_sender
 *
 * \brief Send a sylkie_packet
 * \param sender sylkie_sender to be used to send the packet
 * \param pkt sylkie_packet to be sent
 * \param flags Flags to be set on send
 * \param err Pointer to sylkie_error to be set on error
 */
int sylkie_sender_send_packet(struct sylkie_sender *sender,
                              struct sylkie_packet *pkt, int flags,
                              enum sylkie_error *err);

/**
 * \memberof sylkie_sender
 *
 * \brief Send raw data given a the length of the data
 * \param sender sylkie_sender to be used to send the data
 * \param data Raw data to be sent
 * \param len Size of the raw data to be sent
 * \param flags Flags to be set on send
 * \param err Pointer to sylkie_error to be set on error
 */
int sylkie_sender_send(struct sylkie_sender *sender, const u_int8_t *data,
                       size_t len, int flags, enum sylkie_error *err);

/**
 * \memberof sylkie_sender
 *
 * \brief Return the link address associated with a given sender
 * \param sender sylkie_sender to return the address for
 */
const u_int8_t *sylkie_sender_addr(struct sylkie_sender *sender);

/**
 * \memberof sylkie_sender
 *
 * \brief Deallocate all associated resources
 * \param sender sylkie_sender to be deallocated
 */
void sylkie_sender_free(struct sylkie_sender *sender);

/**
 * \memberof sylkie_sender
 *
 * \brief Pretty print the metadata associated with the sender
 * \param sender sylkie_sender to print
 */
void sylkie_print_sender(struct sylkie_sender *sender, FILE *output);

/// @} end of doxygen sender group

#endif
