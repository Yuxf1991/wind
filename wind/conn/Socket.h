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

#pragma once

#include "base/UniqueFd.h"

#include "SockAddr.h"

namespace wind {
namespace conn {
namespace sockets {
// will implicitly set CLOEXEC flag and call abort() if create socket failed.
int createNonBlockSocketOrDie(int family, int type, int protocol);

void bindOrDie(int fd, const sockaddr *addr, socklen_t addrLen);
int accept(int fd, sockaddr *addr, socklen_t addrLen);
int connect(int fd, const sockaddr *addr, socklen_t addrLen);

SockAddrInet getLocalAddrInet(int sockFd);
SockAddrInet getPeerAddrInet(int sockFd);
SockAddrUnix getLocalAddrUnix(int sockFd);
SockAddrUnix getPeerAddrUnix(int sockFd);

int getSocketError(int sockFd);
bool isSelfConnectInet(int sockFd);
bool isSelfConnectUnix(int sockFd);
} // namespace sockets

class Socket : base::NonCopyable {
public:
    explicit Socket(int sockFd);
    ~Socket() noexcept;

    // movable
    Socket(Socket &&other) noexcept;
    Socket &operator=(Socket &&other) noexcept;

    int fd() const
    {
        return fd_.get();
    }

    // will call abort() if bind failed.
    template <typename SockType>
    void bindOrDie(const SockAddr<SockType> &addr) const
    {
        sockets::bindOrDie(fd_.get(), addr.getSockAddr(), addr.len());
    }
    // will call abort() if listen failed.
    void listenOrDie() const;

    // success: return a valid fd and fill peerAddr
    // failure: return -1
    template <typename SockType>
    int accept(SockAddr<SockType> &peerAddr) const
    {
        return sockets::accept(fd_.get(), peerAddr.data(), peerAddr.capacity());
    }
    // success: return 0
    // failure: return -1
    template <typename SockType>
    int connect(const SockAddr<SockType> &remoteAddr) const
    {
        return sockets::connect(fd_.get(), remoteAddr.getSockAddr(), remoteAddr.len());
    }

    // will check ret value in this func and log err info, same as follows
    void setTcpNoDelay(bool on); // set TCP_NODELAY of the socket (disable/enable Nagle's algorithm).
    void setKeepAlive(bool on);  // set SO_KEEPALIVE
    void setReusePort(bool on);  // set SO_REUSEPORT
    void setReuseAddr(bool on);  // set SO_REUSEADDR

private:
    int setSocketOpt(int level, int optName, bool on);

    base::UniqueFd fd_;
};
} // namespace conn
} // namespace wind
