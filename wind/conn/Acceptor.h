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
#include "Socket.h"

namespace wind {
namespace conn {
// This Acceptor can support for both network and unix local ipc.
enum class AcceptorType { INET_ACCEPTOR, UNIX_ACCEPTOR, UNKNOWN };

using AcceptCallbackInet = std::function<void(int, const SockAddrInet &)>;
using AcceptCallbackUnix = std::function<void(int, const SockAddrUnix &)>;

class Acceptor : base::NonCopyable {
public:
    Acceptor(
        base::EventLoop *eventLoop,
        const SockAddrInet &listenAddr,
        bool reusePort = true,
        int type = SOCK_STREAM,
        int protocol = IPPROTO_TCP);
    Acceptor(base::EventLoop *eventLoop, const SockAddrUnix &listenAddr, int type = SOCK_STREAM, int protocol = 0);
    ~Acceptor() noexcept;

    void setAcceptCallback(const AcceptCallbackInet &callback);
    void setAcceptCallback(const AcceptCallbackUnix &callback);

    void listen();

    bool listening() const
    {
        return listening_;
    }

private:
    void reuseAndLockUnixAddrOrDie(const string &socketPath);
    void assertInLoopThread() const;
    void handleAccptError();
    void acceptNewInetConn();
    void acceptNewUnixConn();
    void handleRead();

    std::atomic<bool> listening_ = false;

    base::EventLoop *loop_ = nullptr;
    Socket acceptSocket_;
    std::shared_ptr<base::EventChannel> acceptChannel_;

    base::UniqueFd idleFd_;

    AcceptorType acceptorType_ = AcceptorType::UNKNOWN;
    AcceptCallbackInet inetAcceptCallback_;
    AcceptCallbackUnix unixAcceptCallback_;
    base::UniqueFd lockfd_;
};
} // namespace conn
} // namespace wind
