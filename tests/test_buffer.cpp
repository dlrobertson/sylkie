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

#include <gtest/gtest.h>

extern "C" {
#include <buffer.h>
}

static char eight_nulls[] = "\x00\x00\x00\x00\x00\x00\x00\x00";

TEST(buffer, zero_size_init) {
  size_t i;
  int res = 0;
  struct sylkie_buffer *buf = sylkie_buffer_init(0);

  res = sylkie_buffer_add(buf, eight_nulls, sizeof(eight_nulls));

  EXPECT_EQ(res, 0);

  for (i = 0; i < buf->len; ++i) {
    EXPECT_EQ(buf->data[i], 0x00);
  }
  EXPECT_EQ(i, (size_t)9);

  sylkie_buffer_free(buf);
}

TEST(buffer, null_add) {
  int res = 0;
  struct sylkie_buffer *buf = NULL;

  res = sylkie_buffer_add(buf, eight_nulls, sizeof(eight_nulls));

  EXPECT_EQ(res, -1);

  sylkie_buffer_free(buf);
}
