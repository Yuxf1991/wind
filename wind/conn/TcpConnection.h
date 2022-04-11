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
#include <mutex>

#include "base/EventLoop.h"
#include "Buffer.h"
#include "Socket.h"
#include "TcpCallbacks.h"

namespace wind {
namespace conn {
enum class TcpConnectionState {
    CONNECTING,
    CONNECTED,
    DISCONNECTING,
    DISCONNECTED,
};

class TcpConnection : private base::NonCopyable, public std::enable_shared_from_this<TcpConnection> {
public:
    TcpConnection(base::EventLoop *loop, string name, int sockFd);
    virtual ~TcpConnection() noexcept;

    const string &name() const
    {
        return name_;
    }
    TcpConnectionState state() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return state_;
    }
    bool connected() const
    {
        return state() == TcpConnectionState::CONNECTED;
    }
    bool disConnected() const
    {
        return state() == TcpConnectionState::DISCONNECTED;
    }

    string stateString() const;

    base::EventLoop *getOwnerLoop() const
    {
        return loop_;
    }

    SockAddrInet getLocalAddr() const
    {
        return localAddr_;
    }
    SockAddrInet getPeerAddr() const
    {
        return peerAddr_;
    }

    // not thread safe.
    void setConnectionCallback(TcpConnectionCallback callback)
    {
        connectionCallback_ = std::move(callback);
    }
    // not thread safe.
    void setMessageCallback(TcpMessageCallback callback)
    {
        messageCallback_ = std::move(callback);
    }
    // not thread safe.
    void setCloseCallback(TcpCloseCallback callback)
    {
        closeCallback_ = std::move(callback);
    }

    // called by TcpServer when accepting a new connection.
    // enable this connection's read channel and set it's state to connected.
    void onEstablished();
    // called by TcpServer when removing this connection from its ConnectionMap.
    // disable this connection's channel and set it's state to disConnected.
    void onRemoved();

    const Buffer *sendBuffer() const
    {
        return &sendBuffer_;
    }
    Buffer *sendBuffer()
    {
        return const_cast<Buffer *>(static_cast<const TcpConnection &>(*this).sendBuffer());
    }
    const Buffer *recvBuffer() const
    {
        return &recvBuffer_;
    }
    Buffer *recvBuffer()
    {
        return const_cast<Buffer *>(static_cast<const TcpConnection &>(*this).recvBuffer());
    }

    void send(string message);

private:
    void assertInLoopThread();

    void setState(TcpConnectionState state)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        state_ = state;
    }

    void callConnectionCallback();
    void callMessageCallback(base::TimeStamp receivedTime);

    void handleRead(base::TimeStamp receivedTime);
    void handleWrite();
    void handleError();
    void handleClose();

    void sendInLoop(string &&message);

    base::EventLoop *loop_;
    string name_;
    Socket socket_;
    SockAddrInet localAddr_;
    SockAddrInet peerAddr_;
    std::shared_ptr<base::EventChannel> channel_;

    mutable std::mutex mutex_;
    TcpConnectionState state_ = TcpConnectionState::DISCONNECTED;

    TcpConnectionCallback connectionCallback_;
    TcpMessageCallback messageCallback_;
    TcpCloseCallback closeCallback_;

    Buffer sendBuffer_;
    Buffer recvBuffer_;
};

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
} // namespace conn
} // namespace wind
