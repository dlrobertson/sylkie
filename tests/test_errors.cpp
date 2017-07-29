#include <gtest/gtest.h>

extern "C" {
#include <errno.h>
#include <errors.h>
#include <string.h>
}

TEST(errors, error_strings) {
    int i = 0;
    for (i = 0; i < SYLKIE_INVALID_ERR; ++i) {
        ASSERT_NE(
            strcmp(sylkie_strerror((enum sylkie_error)i), "Invalid error."), 0);
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
