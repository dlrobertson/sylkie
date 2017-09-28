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

#ifndef SYLKIE_INCLUDE_SENDER_MAP_H
#define SYLKIE_INCLUDE_SENDER_MAP_H

#include <sender.h>

/**
 * \defgroup sender_map B-Tree map structure for sylkie_sender's
 * \ingroup libsylkie
 * @{
 */

/**
 * \brief Cache of sylkie_sender's
 *
 * Useful when using multiple sylkie_senders.
 *
 * Note: There may only be one sender per interface at any one point
 * in the map.
 *
 * @see sylkie_sender
 */
struct sylkie_sender_map;

/**
 * \brief Initialize a new sender map
 */
struct sylkie_sender_map *sylkie_sender_map_init(size_t sz);

/**
 * \brief Create a new entry in the cache for the given interface
 *
 * NB: This will return the sender for the interface if one already
 * exists in the map.
 */
struct sylkie_sender *sylkie_sender_map_add(struct sylkie_sender_map *map,
                                            const char *iface,
                                            enum sylkie_error *err);

/**
 * \brief Return the sender for the given interface index
 *
 * This function uses a B-tree search and therefore is O(log n)
 */
struct sylkie_sender *sylkie_sender_map_get(struct sylkie_sender_map *map,
                                            int ifindex);

/**
 * \brief Return the sender for the given name
 *
 * NB: this function is O(n)
 *
 * @see sylkie_sender_map_get
 */
struct sylkie_sender *sylkie_sender_map_get_name(struct sylkie_sender_map *map,
                                                 const char* name);

/**
 * \brief Free all associated resources
 * \param map Map of sylkie_senders to free the resources of
 */
void sylkie_sender_map_free(struct sylkie_sender_map *map);

#define SYLKIE_SENDER_MAP_FOREACH(map, cur) \
    for (cur = map->senders; cur < map->senders + map->len && cur; ++cur)

/// @} end of doxygen sender_map group

#endif
