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

#include <map>

#include "base/EventLoopThreadPool.h"
#include "Acceptor.h"
#include "TcpConnection.h"

namespace wind {
namespace conn {
class TcpServer : base::NonCopyable {
    using ConnectionPtr = std::shared_ptr<TcpConnection>;
    using ConnectionMap = std::map<string, ConnectionPtr>;

public:
    TcpServer(
        base::EventLoop *loop,
        const SockAddrInet &listenAddr,
        string name = "WindTcpServer",
        bool reusePort = true);
    virtual ~TcpServer() noexcept;
    void start();

private:
    void assertInMainLoopThread();
    void onNewConnection(int peerFd, const SockAddrInet &peerAddr);

    base::EventLoop *mainLoop_ = nullptr;
    string name_;
    std::unique_ptr<base::EventLoopThreadPool> threadPool_;
    std::unique_ptr<Acceptor> acceptor_;
    std::atomic<bool> running_ = false;
    ConnectionMap conns_;
};
} // namespace conn
} // namespace wind
