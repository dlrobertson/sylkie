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
#include <errors.h>

static struct {
    enum sylkie_error code;
    const char* string;
} sylkie_error_strings[] = {
    {SYLKIE_SUCCESS, "Success."},
    {SYLKIE_FATAL, "Fatal error."},
    {SYLKIE_NULL_INPUT, "Null input."},
    {SYLKIE_NO_DEVICE, "No such device."},
    {SYLKIE_NOT_FOUND, "Not found."},
    {SYLKIE_SYSCALL_FAILED, "Syscall failed."},
    {SYLKIE_NO_MEM, "No memory."},
    {SYLKIE_TOO_LARGE, "Input too large."},
    {SYLKIE_EPERM, "Operation not permitted."},
    {SYLKIE_EAGAIN, "Resource temporarily unavailable."},
    {SYLKIE_EAGAIN, "Invalid input."},
    {SYLKIE_INVALID_ERR, "Invalid error."}};

void sylkie_error_set(enum sylkie_error* err, enum sylkie_error new_err) {
    if (err && *err != new_err) {
        *err = new_err;
    }
}

void sylkie_error_from_errno(enum sylkie_error* err) {
    if (err) {
        switch (errno) {
        case EACCES:
        case EPERM:
            *err = SYLKIE_EPERM;
            break;
        case EAGAIN:
            *err = SYLKIE_EAGAIN;
            break;
        case EINVAL:
            *err = SYLKIE_EINVAL;
            break;
        case ENXIO:
        case ENODEV:
        case ENOENT:
            *err = SYLKIE_NO_DEVICE;
            break;
        case ENOMEM:
            *err = SYLKIE_NO_MEM;
            break;
        default:
            *err = SYLKIE_SYSCALL_FAILED;
            break;
        }
    }
}

const char* sylkie_strerror(const enum sylkie_error err) {
    if (err < SYLKIE_INVALID_ERR) {
        return sylkie_error_strings[err].string;
    } else {
        return sylkie_error_strings[SYLKIE_INVALID_ERR].string;
    }
}
