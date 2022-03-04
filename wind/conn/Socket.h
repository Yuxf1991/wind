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

#include "SockAddrInet.h"

namespace wind {
namespace conn {
class Socket : base::NonCopyable {
public:
    // Will call abort() if create socket failed.
    static Socket createNonBlockSocket(int domain, int type, int protocol);

    explicit Socket(int sockfd);
    ~Socket() noexcept;

    // movable
    Socket(Socket &&other);
    Socket &operator=(Socket &&other);

    int fd() const { return fd_.get(); }

    // will call abort() if bind failed.
    void bindOrDie(const SockAddrInet &addr) const;
    // will call abort() if listen failed.
    void listenOrDie() const;

    // success: return valid fd
    // failure: return -1
    int accept(SockAddrInet &peerAddr) const;
    // success: return 0
    // failure: return -1
    int connect(const SockAddrInet &addr) const;

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
