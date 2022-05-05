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

TcpConnection::TcpConnection(EventLoop *loop, std::string name, int sockFd)
    : loop_(loop),
      name_(std::move(name)),
      socket_(sockFd),
      localAddr_(sockets::getLocalAddrInet(sockFd)),
      peerAddr_(sockets::getPeerAddrInet(sockFd)),
      channel_(std::make_shared<EventChannel>(sockFd, loop_))
{
    channel_->setReadCallback([this](TimeStamp receivedTime) { handleRead(receivedTime); });
    channel_->setWriteCallback([this]() { handleWrite(); });
    channel_->setErrorCallback([this]() { handleError(); });
    channel_->setCloseCallback([this]() { handleClose(); });
    socket_.setKeepAlive(true);
    setState(TcpConnectionState::CONNECTING);
}

TcpConnection::~TcpConnection() noexcept {}

std::string TcpConnection::stateString() const
{
    std::string res;
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

void TcpConnection::onEstablished()
{
    loop_->runInLoop([this]() {
        ASSERT(state() == TcpConnectionState::CONNECTING);
        channel_->tie(shared_from_this());
        channel_->enableReading(true);
        setState(TcpConnectionState::CONNECTED);
        callConnectionCallback();
    });
}

void TcpConnection::onRemoved()
{
    loop_->runInLoop([this]() {
        if (WIND_UNLIKELY(state() == TcpConnectionState::CONNECTED)) {
            setState(TcpConnectionState::DISCONNECTED);
            channel_->disableAll(true);

            callConnectionCallback();
        }
    });
}

void TcpConnection::send(std::string message)
{
    loop_->runInLoop([this, msg(std::move(message))]() mutable { sendInLoop(std::move(msg)); });
}

void TcpConnection::sendInLoop(std::string &&message)
{
    assertInLoopThread();

    if (state_ != TcpConnectionState::CONNECTED) {
        LOG_WARN << "TcpConnection::sendInLoop give up because conn is not connected. State: " << stateString() << ".";
        return;
    }

    const char *msgData = message.data();
    std::size_t msgLen = message.size();
    ssize_t wroteBytes = 0;
    std::size_t remaining = msgLen;

    // we can write directly if there is nothing in output queue.
    if (sendBuffer_.bytesReadable() == 0 && !channel_->isWriting()) {
        wroteBytes = socket_.write(msgData, msgLen);
        if (wroteBytes < 0) {
            // TODO: error handle
            wroteBytes = 0;
        } else {
            ASSERT(static_cast<std::size_t>(wroteBytes) <= msgLen);
            remaining = msgLen - static_cast<std::size_t>(wroteBytes);
        }
    }

    ASSERT(remaining <= msgLen);
    ASSERT(wroteBytes >= 0);
    if (remaining > 0) {
        sendBuffer_.append(msgData + static_cast<std::size_t>(wroteBytes), remaining);
        if (!channel_->isWriting()) {
            channel_->enableWriting(true);
        }
    }
}

void TcpConnection::handleRead(TimeStamp receivedTime)
{
    assertInLoopThread();

    int savedError = 0;
    auto n = recvBuffer_.handleSocketRead(socket_.fd(), savedError);

    if (n == 0) {
        handleClose();
    } else if (n > 0) {
        callMessageCallback(receivedTime);
    } else {
        // n < 0
        // TODO: error handle.
    }
}

void TcpConnection::handleWrite()
{
    assertInLoopThread();

    if (WIND_UNLIKELY(!channel_->isWriting())) {
        LOG_ERROR << "TcpConnection::onChannelWritable: channel(fd: " << channel_->fd() << ") is not writable!";
        return;
    }

    ssize_t wroteBytes = socket_.write(sendBuffer_.peek(), sendBuffer_.bytesReadable());
    if (wroteBytes < 0) {
        // TODO: error handle
    } else {
        sendBuffer_.resume(static_cast<std::size_t>(wroteBytes));
        if (sendBuffer_.bytesReadable() == 0) {
            // write complete, can disable channel's write event.
            channel_->disableWriting(true);
        }
    }
}

void TcpConnection::handleError() {}

void TcpConnection::handleClose()
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
