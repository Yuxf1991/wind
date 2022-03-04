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

#include "base/EventLoop.h"

#include "SockAddrInet.h"
#include "Socket.h"
#include <cstddef>
#include <sys/socket.h>

namespace wind {
namespace conn {
using AcceptCallback = std::function<void(int, const SockAddrInet &)>;

class Acceptor : base::NonCopyable {
public:
    Acceptor(base::EventLoop *eventLoop, const SockAddrInet &listenAddr, int type = SOCK_STREAM);
    ~Acceptor() noexcept;

    void setAcceptCallback(const AcceptCallback &callback);

    void listen();

    bool listening() const { return listening_; }

private:
    void assertInLoopThread() const;
    void handleRead();

    std::atomic<bool> listening_ = false;

    base::EventLoop *loop_ = nullptr;
    Socket socket_;
    std::shared_ptr<base::EventChannel> acceptChannel_;

    base::UniqueFd idleFd_;
    AcceptCallback acceptCallback_;
};
} // namespace conn
} // namespace wind
