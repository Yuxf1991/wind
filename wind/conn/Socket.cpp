// MIT License

// Copyright (c) 2020 Tracy

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "Socket.h"

#include <netinet/tcp.h>

#include "base/Log.h"

namespace wind {
namespace conn {
namespace sockets {
#ifdef NO_ACCEPT4
void setNonBlockAndCloseOnExec(int fd)
{
    // set non-block
    int flags = TEMP_FAILURE_RETRY(::fcntl(fd, F_GETFL, 0));
    flags |= O_NONBLOCK;
    int ret = TEMP_FAILURE_RETRY(::fcntl(fd, F_SETFL, flags));
    if (ret < 0) {
        LOG_ERROR << "Set socket(fd: " << fd << ") non-block failed: " << strerror(errno) << ".";
    }

    // set close-on-exec
    flags = TEMP_FAILURE_RETRY(::fcntl(fd, F_GETFD, 0));
    flags |= FD_CLOEXEC;
    ret = TEMP_FAILURE_RETRY(::fcntl(fd, F_SETFD, flags));
    if (ret < 0) {
        LOG_ERROR << "Set socket(fd: " << fd << ") close-on-exec failed: " << strerror(errno) << ".";
    }
}
#endif // NO_ACCEPT4

int createNonBlockSocketOrDie(int family, int type, int protocol)
{
    int fd = TEMP_FAILURE_RETRY(::socket(family, type | SOCK_NONBLOCK | SOCK_CLOEXEC, protocol));
    if (fd < 0) {
        LOG_SYS_FATAL << "Create non-block and close-on-exec socket failed: " << strerror(errno) << ".";
    }

    return fd;
}

void bindOrDie(int fd, const sockaddr *addr, socklen_t addrLen)
{
    int ret = TEMP_FAILURE_RETRY(::bind(fd, addr, addrLen));
    if (ret < 0) {
        LOG_SYS_FATAL << "Socket Fd(" << fd << ") bind err: " << strerror(errno) << ".";
    }
}

int accept(int fd, sockaddr *addr, socklen_t addrLen)
{
    socklen_t len = addrLen;
#ifdef NO_ACCEPT4
    int peerfd = TEMP_FAILURE_RETRY(::accept(fd, addr, &len));
    setNonBlockAndCloseOnExec(peerfd);
#else  // NO_ACCEPT4
    int peerfd = TEMP_FAILURE_RETRY(::accept4(fd, addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC));
#endif // NO_ACCEPT4

    if (peerfd < 0) {
        LOG_ERROR << "Accept error of Socket(fd: " << fd << "): " << strerror(errno);
    }

    return peerfd;
}

int connect(int fd, const sockaddr *addr, socklen_t addrLen)
{
    return TEMP_FAILURE_RETRY(::connect(fd, addr, addrLen));
}

SockAddrInet getLocalAddrInet(int sockFd)
{
    SockAddrInet addr;
    socklen_t addrLen = addr.len();
    if (::getsockname(sockFd, addr.data(), &addrLen) < 0) {
        LOG_ERROR << "getLocalAddrInet error for Socket(fd: " << sockFd << "): " << strerror(errno) << ".";
    }
    return addr;
}

SockAddrInet getPeerAddrInet(int sockFd)
{
    SockAddrInet addr;
    socklen_t addrLen = addr.len();
    if (::getpeername(sockFd, addr.data(), &addrLen) < 0) {
        LOG_ERROR << "getPeerAddrInet error for Socket(fd: " << sockFd << "): " << strerror(errno) << ".";
    }
    return addr;
}

SockAddrUnix getLocalAddrUnix(int sockFd)
{
    SockAddrUnix addr;
    socklen_t addrLen = addr.capacity();
    if (::getsockname(sockFd, addr.data(), &addrLen) < 0) {
        LOG_ERROR << "getLocalAddrUnix error for Socket(fd: " << sockFd << "): " << strerror(errno) << ".";
    }
    return addr;
}

SockAddrUnix getPeerAddrUnix(int sockFd)
{
    SockAddrUnix addr;
    socklen_t addrLen = addr.capacity();
    if (::getpeername(sockFd, addr.data(), &addrLen) < 0) {
        LOG_ERROR << "getPeerAddrUnix error for Socket(fd: " << sockFd << "): " << strerror(errno) << ".";
    }
    return addr;
}
} // namespace sockets

Socket::Socket(int sockfd) : fd_(sockfd) {}

Socket::~Socket() noexcept {}

// movable
Socket::Socket(Socket &&other) noexcept : fd_(std::move(other.fd_)) {}

Socket &Socket::operator=(Socket &&other) noexcept
{
    fd_ = std::move(other.fd_);
    return *this;
}

void Socket::listenOrDie() const
{
    int ret = TEMP_FAILURE_RETRY(::listen(fd_.get(), SOMAXCONN));
    if (ret < 0) {
        LOG_SYS_FATAL << "Socket Fd(" << fd_.get() << ") listen err: " << strerror(errno) << "!";
    }
}

int Socket::setSocketOpt(int level, int optName, bool on)
{
    int opt = on ? 1 : 0;
    return TEMP_FAILURE_RETRY(::setsockopt(fd(), level, optName, &opt, static_cast<socklen_t>(sizeof(opt))));
}

void Socket::setTcpNoDelay(bool on)
{
    int ret = setSocketOpt(IPPROTO_TCP, TCP_NODELAY, on);
    if (ret < 0) {
        LOG_ERROR << "Socket(fd: " << fd() << ") setTcpNoDelay " << on << " failed: " << strerror(errno) << ".";
    }
}

void Socket::setKeepAlive(bool on)
{
    int ret = setSocketOpt(SOL_SOCKET, SO_KEEPALIVE, on);
    if (ret < 0) {
        LOG_ERROR << "Socket(fd: " << fd() << ") setKeepAlive " << on << " failed: " << strerror(errno) << ".";
    }
}

void Socket::setReusePort(bool on)
{
    int ret = setSocketOpt(SOL_SOCKET, SO_REUSEPORT, on);
    if (ret < 0) {
        LOG_ERROR << "Socket(fd: " << fd() << ") setReusePort " << on << " failed: " << strerror(errno) << ".";
    }
}

void Socket::setReuseAddr(bool on)
{
    int ret = setSocketOpt(SOL_SOCKET, SO_REUSEADDR, on);
    if (ret < 0) {
        LOG_ERROR << "Socket(fd: " << fd() << ") setReuseAddr " << on << " failed: " << strerror(errno) << ".";
    }
}
} // namespace conn
} // namespace wind
