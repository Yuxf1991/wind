//
// Created by tracy on 2020/10/15.
//

#include "Socket.h"
#include "Utils.h"

#include <sys/socket.h>
#include <cassert>
#include <fcntl.h>
#include <unistd.h>

namespace wind {
    namespace sockets {
#ifndef HAVE_ACCEPT4
        int setCloExecOrClose(int fd) {
            assert(fd != -1);
            int flags = ::fcntl(fd, F_GETFD);
            if (flags == -1) {
                return -1;
            }

            if (::fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1) {
                return -1;
            }

            return 0;
        }
#endif

        int createCloExec(int domain) {
            int fd = -1;
#ifdef HAVE_ACCEPT4
            SYSCALL_CHECK(fd = ::socket(domain, SOCK_STREAM | SOCK_CLOEXEC, 0));
#else
            SYSCALL_CHECK(fd = ::socket(domain, SOCK_STREAM, 0));
            if (setCloExecOrClose(fd) == -1) {
                SYSCALL_CHECK(close(fd));
                fd = -1;
            }
#endif
            return fd;
        }
    } // namespace sockets

    Socket::Socket(int socket) : m_socket(socket) {}

    Socket::~Socket() {
        SYSCALL_CHECK(::close(m_socket));
    }
} // namespace wind