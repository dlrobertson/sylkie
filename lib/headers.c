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
#include <string.h>

#include <headers.h>

// sylkie_header_node methods

static void rm_header_node(struct sylkie_header_list* lst,
                           struct sylkie_header_node* node,
                           struct sylkie_header_node* prev) {
    if (prev) {
        // We are a not the head
        prev->next = node->next;
    } else {
        // We are the head node
        if (lst->tail && node->next) {
            lst->tail->next = node->next;
        } else {
            // We are the only node
            lst->head = NULL;
        }
    }
    node->next = NULL;
}

struct sylkie_header_node* sylkie_header_node_init(enum sylkie_header_type type,
                                                   void* data, size_t len) {
    struct sylkie_header_node* hdr = malloc(sizeof(struct sylkie_header_node));
    if (!hdr) {
        return NULL;
    }
    hdr->type = type;
    hdr->len = len;
    hdr->data = malloc(len);
    if (!hdr->data) {
        free(hdr);
        return NULL;
    }
    memcpy(hdr->data, data, len);
    return hdr;
}

void sylkie_header_node_free(struct sylkie_header_node* hdr) {
    if (hdr) {
        if (hdr->data) {
            free(hdr->data);
        }
        free(hdr);
        hdr = NULL;
    }
}

void sylkie_header_list_init(struct sylkie_header_list* lst,
                             struct sylkie_header_node* header) {
    lst->head = header;
    lst->tail = NULL;
}

enum sylkie_error sylkie_header_list_add(struct sylkie_header_list* lst,
                                         struct sylkie_header_node* header) {
    if (!lst || !lst->head) {
        return SYLKIE_NULL_INPUT;
    } else if (lst->tail) {
        lst->tail->next = header;
        header->next = lst->head;
    } else {
        // Second header
        lst->tail = header;
        header->next = lst->head;
    }
    return SYLKIE_SUCCESS;
}

enum sylkie_error
sylkie_header_list_rm_node(struct sylkie_header_list* lst,
                           struct sylkie_header_node* header) {
    struct sylkie_header_node* node;
    struct sylkie_header_node* prev = NULL;
    if (!lst || !lst->head || !header) {
        return SYLKIE_NULL_INPUT;
    } else {
        HEADER_LIST_FOREACH(lst, node) {
            if (node == header) {
                rm_header_node(lst, node, prev);
                return SYLKIE_SUCCESS;
            } else {
                prev = node;
            }
        }
        return SYLKIE_NOT_FOUND;
    }
}

enum sylkie_error sylkie_header_list_rm_type(struct sylkie_header_list* lst,
                                             enum sylkie_header_type type) {
    struct sylkie_header_node* node;
    struct sylkie_header_node* prev = NULL;
    if (!lst || !lst->head) {
        return SYLKIE_NULL_INPUT;
    } else {
        HEADER_LIST_FOREACH(lst, node) {
            if (node->type == type) {
                rm_header_node(lst, node, prev);
                return SYLKIE_SUCCESS;
            } else {
                prev = node;
            }
        }
        return SYLKIE_NOT_FOUND;
    }
}

void sylkie_header_free(struct sylkie_header_list* lst) {
    struct sylkie_header_node* node;
    struct sylkie_header_node* tmp;
    if (lst && lst->head) {
        node = lst->head;
        while (node) {
            tmp = node->next;
            free(node);
            node = tmp;
        }
    }
}
