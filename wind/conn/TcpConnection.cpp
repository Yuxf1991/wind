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

#include "TcpConnection.h"

#include "base/Log.h"

namespace wind {
namespace conn {
using namespace base;

namespace detail {
void defaultConnectionCallback(const TcpConnectionPtr &conn)
{
    if (WIND_UNLIKELY(conn == nullptr)) {
        return;
    }

    LOG_INFO << "Connection " << conn->name() << " " << conn->stateString() << ".";
}

void defaultMessageCallback(const TcpConnectionPtr &conn, TimeStamp receivedTime)
{
    if (WIND_UNLIKELY(conn == nullptr)) {
        return;
    }

    Buffer *buffer = conn->recvBuffer();
    buffer->resumeAll(); // drop all messages by default.
    UNUSED(receivedTime);
}
} // namespace detail

TcpConnection::TcpConnection(EventLoop *loop, string name, int sockFd)
    : loop_(loop),
      name_(std::move(name)),
      socket_(sockFd),
      localAddr_(sockets::getLocalAddrInet(sockFd)),
      peerAddr_(sockets::getPeerAddrInet(sockFd)),
      channel_(std::make_shared<EventChannel>(sockFd, loop_))
{
    channel_->setReadCallback([this](TimeStamp receivedTime) { onChannelReadable(receivedTime); });
    channel_->setWriteCallback([this]() { onChannelWritable(); });
    channel_->setErrorCallback([this]() { onChannelError(); });
    channel_->setCloseCallback([this]() { onChannelClose(); });
    socket_.setKeepAlive(true);
    setState(TcpConnectionState::CONNECTING);
}

TcpConnection::~TcpConnection() noexcept {}

string TcpConnection::stateString() const
{
    string res;
    switch (state()) {
        case TcpConnectionState::CONNECTING: {
            res = "Connecting";
            break;
        }
        case TcpConnectionState::CONNECTED: {
            res = "Connected";
            break;
        }
        case TcpConnectionState::DISCONNECTING: {
            res = "Disconnecting";
            break;
        }
        case TcpConnectionState::DISCONNECTED: {
            res = "Disconnected";
            break;
        }
        default: {
            res = "Unknown";
            break;
        }
    }

    return res;
}

void TcpConnection::callConnectionCallback()
{
    if (connectionCallback_) {
        connectionCallback_(shared_from_this());
    } else {
        detail::defaultConnectionCallback(shared_from_this());
    }
}

void TcpConnection::callMessageCallback(TimeStamp receivedTime)
{
    if (messageCallback_) {
        messageCallback_(shared_from_this(), receivedTime);
    } else {
        detail::defaultMessageCallback(shared_from_this(), receivedTime);
    }
}

void TcpConnection::assertInLoopThread()
{
    loop_->assertInLoopThread();
}

void TcpConnection::connectionEstablished()
{
    loop_->runInLoop([this]() {
        ASSERT(state() == TcpConnectionState::CONNECTING);
        channel_->tie(shared_from_this());
        channel_->enableReading(true);
        setState(TcpConnectionState::CONNECTED);
        callConnectionCallback();
    });
}

void TcpConnection::connectionRemoved()
{
    loop_->runInLoop([this]() {
        if (WIND_UNLIKELY(state() == TcpConnectionState::CONNECTED)) {
            setState(TcpConnectionState::DISCONNECTED);
            channel_->disableAll(true);

            callConnectionCallback();
        }
    });
}

void TcpConnection::onChannelReadable(TimeStamp receivedTime)
{
    assertInLoopThread();

    int savedError = 0;
    auto n = recvBuffer_.handleSocketRead(socket_.fd(), savedError);

    if (n == 0) {
        onChannelClose();
    } else if (n > 0) {
        callMessageCallback(receivedTime);
    } else {
        // n < 0
        // TODO: error handle.
    }
}

void TcpConnection::onChannelWritable() {}
void TcpConnection::onChannelError() {}

void TcpConnection::onChannelClose()
{
    assertInLoopThread();

    setState(TcpConnectionState::DISCONNECTED);
    channel_->disableAll(true);
    callConnectionCallback();

    if (WIND_LIKELY(closeCallback_ != nullptr)) {
        closeCallback_(shared_from_this());
    }
}
} // namespace conn
} // namespace wind
