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
    using ConnectionMap = std::map<string, TcpConnectionPtr>;

public:
    TcpServer(
        base::EventLoop *loop,
        const SockAddrInet &listenAddr,
        string name = "WindTcpServer",
        bool reusePort = true);
    virtual ~TcpServer() noexcept;

    const string &name() const
    {
        return name_;
    }

    // Should be called before calling start(), thread safe.
    void setConnectionCallback(TcpConnectionCallback callback);
    // Should be called before calling start(), thread safe.
    void setMessageCallback(TcpMessageCallback callback);
    // Should be called before calling start(), thread safe.
    void setThreadNum(size_t threadNum);

    void start();

private:
    void assertInMainLoopThread();
    void onNewConnection(int peerFd, const SockAddrInet &peerAddr);
    void onRemoveConnection(const TcpConnectionPtr &conn);

    std::atomic<bool> running_ = false;
    mutable std::mutex mutex_;
    base::EventLoop *mainLoop_ = nullptr;
    string name_;
    std::unique_ptr<base::EventLoopThreadPool> threadPool_;
    std::unique_ptr<Acceptor> acceptor_;
    std::atomic<uint64_t> connId_ = 0;
    ConnectionMap conns_;

    TcpConnectionCallback connectionCallback_;
    TcpMessageCallback messageCallback_;
};
} // namespace conn
} // namespace wind
