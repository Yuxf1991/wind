//
// Created by tracy on 2020/10/15.
//

#include <gtest/gtest.h>
#include <sys/socket.h>
#include <unistd.h>

#include "Utils.h"

namespace wind::test {
    TEST(ScopedFd, ScopedFdTest) {
        utils::ScopedFd fd;
        EXPECT_EQ(fd, -1);
        fd.reset(10);
        EXPECT_GT(fd, 0);
        EXPECT_EQ(fd, 10);
        fd.reset(); // failed: bad fd.
        EXPECT_LT(fd, 0);
        EXPECT_EQ(fd, -1);
    }

    TEST(SysCallCheck, SysCallCheckTest) {
        int fd = -1;
        int ret = -1;
        SYSCALL_CHECK(fd = socket(AF_LOCAL, SOCK_STREAM, 0)); // should success.
        EXPECT_GE(fd, 0);
        SYSCALL_CHECK(ret = close(fd)); // should success
        EXPECT_EQ(ret, 0);
        SYSCALL_CHECK(fd = socket(23, SOCK_STREAM, 7)); // should fail
        EXPECT_LT(fd, 0);
    }
} // namespace wind::test
