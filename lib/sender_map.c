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

#include <string.h>

#include <sender_map.h>

// Btree of sylkie_sender's
struct sylkie_sender_map {
  struct sylkie_sender **senders;
  size_t len;
  size_t cap;
};

// Private function used to realloc the sender_map if it is too small to
// add one more sender.
static int maybe_realloc(struct sylkie_sender_map *map) {
  size_t newcap = 0;
  struct sylkie_sender **tmp = NULL;

  newcap = map->len + 1;
  if (newcap > map->cap) {
    if (map->senders) {
      tmp = realloc(map->senders, sizeof(struct sylkie_sender *) * newcap);
    } else {
      tmp = malloc(newcap);
    }
    if (tmp) {
      map->cap = newcap;
      map->senders = tmp;
      return 0;
    } else {
      return -1;
    }
  } else {
    return 0;
  }
}

struct sylkie_sender_map *sylkie_sender_map_init(size_t sz) {
  struct sylkie_sender_map *map = malloc(sizeof(struct sylkie_sender_map));
  if (!map) {
    return NULL;
  }
  // If sz is zero, don't do anything. We can lazily allocate
  // resources as needed.
  if (sz) {
    map->senders = malloc(sizeof(struct sylkie_sender *) * sz);
    if (!map->senders) {
      free(map);
      return NULL;
    }
    map->cap = sz;
    map->len = 0;
  } else {
    map->len = 0;
    map->cap = 0;
    map->senders = NULL;
  }
  return map;
}

struct sylkie_sender *
sylkie_sender_map_get_name(const struct sylkie_sender_map *map,
                           const char *name) {
  int i;
  if (map) {
    for (i = 0; i < map->len; ++i) {
      if (map->senders[i] &&
          strcmp(sylkie_sender_name(map->senders[i]), name) == 0) {
        return map->senders[i];
      }
    }
  }
  return NULL;
}

static int sender_cmp(const void *elem1, const void *elem2) {
  int s1_idx = sylkie_sender_ifindex(*(struct sylkie_sender **)elem1);
  int s2_idx = sylkie_sender_ifindex(*(struct sylkie_sender **)elem2);
  if (s1_idx < s2_idx) {
    return -1;
  } else if (s1_idx > s2_idx) {
    return 1;
  } else {
    return 0;
  }
}

struct sylkie_sender *sylkie_sender_map_add(struct sylkie_sender_map *map,
                                            const char *name,
                                            enum sylkie_error *err) {
  struct sylkie_sender *sender;
  if (!map) {
    return NULL;
  }
  // Check to see if the current buffer is too small. If this fails,
  // Something really bad happened and we need to bail
  if (maybe_realloc(map)) {
    sylkie_error_set(err, SYLKIE_NO_MEM);
    return NULL;
  }
  sender = sylkie_sender_init(name, err);
  if (!sender) {
    return NULL;
  }
  map->senders[map->len] = sender;
  ++map->len;
  qsort(map->senders, map->len, sizeof(struct sylkie_sender *), sender_cmp);
  return sender;
}

static struct sylkie_sender *find_inner(struct sylkie_sender **begin,
                                        struct sylkie_sender **end,
                                        struct sylkie_sender **cur,
                                        int ifindex) {
  int diff = 0;
  int cur_ifindex = -1;
  // Check if we have exhaused the map
  if (cur < end && cur >= begin) {
    cur_ifindex = sylkie_sender_ifindex(*cur);
    if (ifindex == cur_ifindex) {
      // We found the item
      return *cur;
    } else if (ifindex < cur_ifindex && cur > begin) {
      // We guessed high. cur is now the end and begin is still the
      // beginning
      diff = ((cur - begin) / 2) + 1;
      return find_inner(begin, cur, cur - diff, ifindex);
    } else if (ifindex > cur_ifindex && cur < end) {
      // We guessed low. cur is not the beginning and end is still the
      // end
      diff = (end - cur) / 2;
      if (!diff) {
        diff = 1;
      }
      return find_inner(cur, end, cur + diff, ifindex);
    } else {
      // We guessed high and we cannot go lower, or we guessed
      // low and we cannot go higher.
      return NULL;
    }
  } else {
    // These aren't the droids you're looking for
    return NULL;
  }
}

struct sylkie_sender *sylkie_sender_map_get(const struct sylkie_sender_map *map,
                                            int ifindex) {
  size_t mid;
  if (map && map->len) {
    // We already know there is at least one item, so it is okay if mid is 0
    mid = map->len / 2;
    return find_inner(map->senders, map->senders + map->len, map->senders + mid,
                      ifindex);
  } else {
    return NULL;
  }
}

void sylkie_sender_map_free(struct sylkie_sender_map *map) {
  struct sylkie_sender **sender;
  if (map) {
    SYLKIE_SENDER_MAP_FOREACH(map, sender) { sylkie_sender_free(*sender); }
  }
}
