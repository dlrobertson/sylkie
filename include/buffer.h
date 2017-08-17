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

#ifndef SYLKIE_INCLUDE_BUFFER_H
#define SYLKIE_INCLUDE_BUFFER_H

#include <sys/types.h>

/**
 * \defgroup buffer sylkie_buffer structures and methods
 * \ingroup libsylkie
 * @{
 */

/**
 * \brief Generic growable buffer structure
 */
struct sylkie_buffer {
    /// Length of the buffer
    size_t len;
    /// Capactity of the buffer
    size_t cap;
    /// Raw pointer to the buffer
    u_int8_t* data;
};

/**
 * \memberof sylkie_buffer
 *
 * \brief Initialize a sylkie_buffer to size `sz`
 * \param sz size of the buffer to be created
 *
 * Note: A size of zero will not allocate any memory. No memory
 * will be allocated untill the first add function is used in
 * this case.
 *
 * @see sylkie_buffer_add
 * @see sylkie_buffer_add_value
 */
struct sylkie_buffer* sylkie_buffer_init(size_t sz);

/**
 * \memberof sylkie_buffer
 *
 * \brief Append data to the buffer
 * \param buf buffer structure to which the data will be added
 * \param ptr pointer to the data to be added
 * \param sz size of the data pointer
 */
int sylkie_buffer_add(struct sylkie_buffer* buf, const void* ptr, size_t sz);

/**
 * \memberof sylkie_buffer
 *
 * \brief Apend data to the buffer
 * \param buf buffer structure to which the data will be added
 * \param value value to be repeated
 * \param sz number of times to repeat the value in the buffer
 */
int sylkie_buffer_add_value(struct sylkie_buffer* buf, const u_int8_t value,
                            size_t sz);

/**
 * \memberof sylkie_buffer
 *
 * \brief Copy the given buffer
 * \param buf buffer that will be cloned
 */
struct sylkie_buffer* sylkie_buffer_clone(const struct sylkie_buffer* buf);

/**
 * \memberof sylkie_buffer
 *
 * \brief Print the contents of the buffer in hexadecimal to stdout
 * \param buf buffer to be printed
 */
void sylkie_buffer_print(const struct sylkie_buffer* buf);

/**
 * \memberof sylkie_buffer
 *
 * \brief Free the memory allocated to this buffer
 * \param buf buffer to be freed
 */
void sylkie_buffer_free(struct sylkie_buffer* buf);

/// @} end of doxygen buffer group

#endif
