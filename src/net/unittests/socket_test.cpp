//
// Created by tracy on 2020/10/15.
//

#include <gtest/gtest.h>

#include "net/Socket.h"

#include <sys/socket.h>
#include <sys/fcntl.h>

namespace wind {
    namespace test {
        TEST(SocketTest, SocketCreate) {
            int fd = sockets::createCloExec(PF_LOCAL);
            EXPECT_TRUE(fd != -1);
            int flags = ::fcntl(fd, F_GETFD);
            EXPECT_TRUE(flags | FD_CLOEXEC);
            Socket socket(fd);
            EXPECT_GE(socket, 0);
        }
    } // namespace test
} // namespace wind