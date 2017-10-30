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

#ifndef SYLKIE_INCLUDE_HEADERS_H
#define SYLKIE_INCLUDE_HEADERS_H

#include <sys/types.h>

#include "errors.h"

/**
 * \defgroup protocol protocol list structures and methods
 * \ingroup libsylkie
 * @{
 */

/**
 * \brief Enumerated list of supported protocols
 */
enum sylkie_proto_type {
  SYLKIE_ETH = 0,
  SYLKIE_IPv6,
  SYLKIE_ICMPv6,
  SYLKIE_DATA,
  SYLKIE_INVALID_HDR_TYPE,
};

/**
 * \brief Generic structure used to define a protocol header
 */
struct sylkie_proto {
  /// Type of the protocol header
  enum sylkie_proto_type type;
  /// Raw bytes
  void *data;
  /// length of the associated data
  size_t len;
};

/**
 * \brief Node of the protocol header linked list
 */
struct sylkie_proto_node {
  /// The protocol header for the given node
  struct sylkie_proto hdr;
  /// The next node in the list of protocol headers
  struct sylkie_proto_node *next;
  /// The next previous node in the list of protocol headers
  struct sylkie_proto_node *prev;
};

/**
 * \brief List of protocol header nodes that form a packet
 */
struct sylkie_proto_list;

/**
 * \memberof sylkie_proto
 *
 * \brief Initialize a sylkie_proto
 * \param hdr Pointer to the proto to initialize
 * \param type Protocol type of the header
 * \param data Raw data contained in the header
 * \param len Length of the raw data
 */
int sylkie_proto_init(struct sylkie_proto *hdr, enum sylkie_proto_type type,
                      void *data, size_t len);

/**
 * \memberof sylkie_proto_node
 *
 * \brief Free allocated resources associated with a given node
 */
void sylkie_proto_node_free(struct sylkie_proto_node *node);

/**
 * \memberof sylkie_proto_list
 *
 * \brief Initialize a sylkie_proto_list
 */
struct sylkie_proto_list *sylkie_proto_list_init();

/**
 * \memberof sylkie_proto_list
 *
 * \brief Add a protocol header to the given list
 * \param lst Protocol list to add headers to
 * \param type Protocol header type
 * \param data Raw header data
 * \param len Length of the raw header data
 */
enum sylkie_error sylkie_proto_list_add(struct sylkie_proto_list *lst,
                                        enum sylkie_proto_type type, void *data,
                                        size_t len);

/**
 * \memberof sylkie_proto_list
 *
 * \brief Add a sylkie_proto_node to a given protocol list
 * \param lst Protocol list to add the node to
 * \param node Protocol header list node to add to the list
 */
enum sylkie_error sylkie_proto_list_add_node(struct sylkie_proto_list *lst,
                                             struct sylkie_proto_node *node);

/**
 * \memberof sylkie_proto_list
 *
 * \brief Remove a sylkie_proto_node from a given protocol list
 * \param lst Protocol header list to remove the node from
 * \param node Pointer to the protocol header list node to remove
 */
enum sylkie_error sylkie_proto_list_rm_node(struct sylkie_proto_list *lst,
                                            struct sylkie_proto_node *node);

/**
 * \memberof sylkie_proto_list
 *
 * \brief Remove all protocol nodes from a given list of the given type
 * \param lst Protocol header list to remove the nodes from
 * \param type Type of protocol nodes to remove
 */
enum sylkie_error sylkie_proto_list_rm(struct sylkie_proto_list *lst,
                                       enum sylkie_proto_type type);

/**
 * \memberof sylkie_proto_list
 *
 * \brief Return the head of a given protocol list
 * \param lst Protocol header list to return the head of
 */
struct sylkie_proto_node *sylkie_proto_list_head(struct sylkie_proto_list *lst);

/**
 * \memberof sylkie_proto_list
 *
 * \brief Return the tail of a given protocol list
 * \param lst Protocol header list to return the tail of
 */
struct sylkie_proto_node *sylkie_proto_list_tail(struct sylkie_proto_list *lst);

/**
 * \memberof sylkie_proto_list
 *
 * \brief Free all allocated resources associated with the list
 * \param lst Protocol header list to deallocate
 */
void sylkie_proto_list_free(struct sylkie_proto_list *lst);

#define SYLKIE_HEADER_LIST_FOREACH(lst, node)                                  \
  for (node = sylkie_proto_list_head(lst); node; node = node->next)

#define SYLKIE_HEADER_LIST_REV_FOREACH(lst, node)                                  \
  for (node = sylkie_proto_list_tail(lst); node; node = node->prev)

/// @} end of doxygen protocol group

#endif
