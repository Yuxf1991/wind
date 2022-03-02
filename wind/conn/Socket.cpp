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

#include "base/Log.h"

namespace wind {
namespace conn {
namespace detail {
int nonBlockSocketCreate(int domain, int type, int protocol)
{
    int fd = ::socket(domain, type | SOCK_NONBLOCK | SOCK_CLOEXEC, protocol);
    if (fd < 0) {
        LOG_SYS_FATAL << "Create socket err: " << strerror(errno) << "!";
    }

    return fd;
}
} // namespace detail

Socket::Socket(int sockfd) : fd_(sockfd) {}

Socket::~Socket() noexcept {}

// movable
Socket::Socket(Socket &&other) : fd_(std::move(other.fd_)) {}

Socket &Socket::operator=(Socket &&other)
{
    fd_ = std::move(other.fd_);
    return *this;
}

Socket Socket::createNonBlockSocket(int domain, int type, int protocol)
{
    return Socket(detail::nonBlockSocketCreate(domain, type, protocol));
}

int Socket::accept(SockAddrInet &peerAddr) const
{
    socklen_t len = peerAddr.len();
    int peerfd = TEMP_FAILURE_RETRY(::accept(fd_.get(), peerAddr.data(), &len));
    if (peerfd < 0) {
        LOG_SYS_FATAL << "Socket Fd(" << fd_.get() << ") accept err: " << strerror(errno) << "!";
    }

    return peerfd;
}

void Socket::bind(const SockAddrInet &addr) const
{
    int ret = TEMP_FAILURE_RETRY(::bind(fd_.get(), addr.get(), addr.len()));
    if (ret < 0) {
        LOG_SYS_FATAL << "Socket Fd(" << fd_.get() << ") bind err: " << strerror(errno) << "!";
    }
}

void Socket::listen() const
{
    int ret = TEMP_FAILURE_RETRY(::listen(fd_.get(), SOMAXCONN));
    if (ret < 0) {
        LOG_SYS_FATAL << "Socket Fd(" << fd_.get() << ") listen err: " << strerror(errno) << "!";
    }
}

void Socket::connect(const SockAddrInet &addr) const
{
    int ret = TEMP_FAILURE_RETRY(::connect(fd_.get(), addr.get(), addr.len()));
    if (ret < 0) {
        LOG_SYS_FATAL << "Socket Fd(" << fd_.get() << ") connect to " << addr.ipPortString()
                      << " err: " << strerror(errno) << "!";
    }
}
} // namespace conn
} // namespace wind
