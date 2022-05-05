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

#include <optional>

#include "base/EventLoop.h"
#include "SockAddr.h"

namespace wind {
namespace conn {
class Connector : private base::NonCopyable, public std::enable_shared_from_this<Connector> {
    using OnConnectedCallback = std::function<void(int sockFd)>;
    enum class State { DISCONNECTED, CONNECTING, CONNECTED };

    enum class ConnectorType { INET, LOCAL, UNKNOWN };

    // 500ms for initial retry delay.
    static constexpr TimeType INITIAL_RETRY_DELAY = 500;
    // 30s for initial retry delay.
    static constexpr TimeType MAX_RETRY_DELAY = 30 * base::MILLI_SECS_PER_SECOND;

public:
    Connector(
        base::EventLoop *loop,
        const SockAddrInet &remoteAddr,
        int dataType = SOCK_STREAM,
        int protocol = IPPROTO_TCP);
    Connector(base::EventLoop *loop, const SockAddrUnix &remoteAddr, int dataType = SOCK_STREAM, int protocol = 0);
    ~Connector() noexcept;

    // must be called before calling start() and thread safe.
    void setOnConnectedCallback(OnConnectedCallback callback);

    void start();   // thread-safe
    void stop();    // thread-safe
    void restart(); // can only be called in loop's thread.

    std::string remoteAddr() const;

private:
    void assertInLoopThread();
    void startInLoop();

    std::string stateString() const;

    void tryConnectInet();
    void unLockUnixAddr();
    void reuseAndLockUnixAddrOrDie(const std::string &socketPath);
    void tryConnectUnix();
    void handleConnectResult(base::UniqueFd &&sockFd, int savedErrno);

    void establish(base::UniqueFd &&sockFd);
    void connecting(int sockFd);
    void retry();

    bool isSelfConnect(int sockFd);
    void removeChannel();
    void onChannelWritable();
    void onChannelError();

    base::EventLoop *loop_ = nullptr;

    SockAddrInet inetRemoteAddr_;
    SockAddrUnix unixRemoteAddr_;
    // Only for unix domain socket.
    base::UniqueFd sockLockFileFd_;
    std::string lockFileName_;

    int dataType_ = SOCK_STREAM;
    int protocol_ = 0;
    ConnectorType connectorType_ = ConnectorType::UNKNOWN;

    mutable std::mutex mutex_;
    OnConnectedCallback onConnectedCallback_; // guarded by mutex_.
    std::shared_ptr<base::EventChannel> channel_;

    State state_ = State::DISCONNECTED;
    std::atomic<bool> started_ = false;

    TimeType retryDelay_ = INITIAL_RETRY_DELAY; // retry delay in ms.
    std::optional<base::TimerId> retryingTimer_;
};

using ConnectorPtr = std::shared_ptr<Connector>;
} // namespace conn
} // namespace wind
