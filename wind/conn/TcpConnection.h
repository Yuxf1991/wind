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

#include <memory>

#include "base/EventLoop.h"
#include "Socket.h"

namespace wind {
namespace conn {
class TcpConnection : private base::NonCopyable, public std::enable_shared_from_this<TcpConnection> {
public:
    TcpConnection(base::EventLoop *loop, string name, int sockFd);
    ~TcpConnection() noexcept;

    string name() const
    {
        return name_;
    }

    SockAddrInet getLocalAddr() const
    {
        return localAddr_;
    }
    SockAddrInet getPeerAddr() const
    {
        return peerAddr_;
    }

private:
    base::EventLoop *loop_;
    string name_;
    Socket socket_;
    SockAddrInet localAddr_;
    SockAddrInet peerAddr_;
    std::shared_ptr<base::EventChannel> channel_;
};
} // namespace conn
} // namespace wind
