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
#include <errno.h>
#include <errors.h>
#include <string.h>
}

TEST(errors, error_strings) {
  int i = 0;
  for (i = 0; i < SYLKIE_INVALID_ERR; ++i) {
    ASSERT_NE(strcmp(sylkie_strerror((enum sylkie_error)i), "Invalid error."),
              0);
  }
  ASSERT_EQ(strcmp(sylkie_strerror(SYLKIE_INVALID_ERR), "Invalid error."), 0);
  ASSERT_EQ(
      strcmp(sylkie_strerror((enum sylkie_error)(SYLKIE_INVALID_ERR + 1024)),
             "Invalid error."),
      0);
}

TEST(errors, errrno) {
  enum sylkie_error err = SYLKIE_SUCCESS;
  errno = EACCES;
  sylkie_error_from_errno(&err);
  ASSERT_EQ(err, SYLKIE_EPERM);
  errno = EPERM;
  sylkie_error_from_errno(&err);
  ASSERT_EQ(err, SYLKIE_EPERM);
  errno = EAGAIN;
  sylkie_error_from_errno(&err);
  ASSERT_EQ(err, SYLKIE_EAGAIN);
  errno = EINVAL;
  sylkie_error_from_errno(&err);
  ASSERT_EQ(err, SYLKIE_EINVAL);
  errno = ENODEV;
  sylkie_error_from_errno(&err);
  ASSERT_EQ(err, SYLKIE_NO_DEVICE);
  errno = ENOMEM;
  sylkie_error_from_errno(&err);
  ASSERT_EQ(err, SYLKIE_NO_MEM);
  errno = E2BIG;
  sylkie_error_from_errno(&err);
  ASSERT_EQ(err, SYLKIE_SYSCALL_FAILED);
}

TEST(errors, set) {
  enum sylkie_error err = SYLKIE_SUCCESS;
  sylkie_error_set(&err, SYLKIE_FATAL);
  ASSERT_EQ(err, SYLKIE_FATAL);
}
