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

#ifndef WIND_SOCKET_H
#define WIND_SOCKET_H

#include "base/UniqueFd.h"

#include "SockAddrInet.h"

namespace wind {
namespace network {
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

    void bind(const SockAddrInet &addr) const;
    void listen() const;
    int accept(SockAddrInet &peerAddr) const;
    void connect(const SockAddrInet &addr) const;

private:
    base::UniqueFd fd_;
};
} // namespace network
} // namespace wind
#endif // WIND_SOCKET_H
