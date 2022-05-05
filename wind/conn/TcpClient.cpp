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

#include "TcpClient.h"

#include "base/Log.h"

namespace wind {
namespace conn {
using namespace base;
TcpClient::TcpClient(base::EventLoop *loop, std::string name, const SockAddrInet &remoteAddr)
    : loop_(loop), name_(std::move(name)), connector_(std::make_shared<Connector>(loop, remoteAddr))
{
    connector_->setOnConnectedCallback([this](int sockFd) { onConnected(sockFd); });
}

TcpClient::~TcpClient() noexcept
{
    // TODO: close the connection if necessary.
}

void TcpClient::start()
{
    if (started_) {
        return;
    }

    started_ = true;
    LOG_INFO << "TcpClient(" << name_ << ") start connecting to " << connector_->remoteAddr() << "...";
    connector_->start();
}

void TcpClient::shutdown()
{
    // TODO: call connection's shutdown().
}

void TcpClient::stop()
{
    started_ = false;

    LOG_INFO << "TcpClient(" << name_ << ") stop connecting.";
    connector_->stop();
}

void TcpClient::setConnectionCallback(TcpConnectionCallback callback)
{
    if (started_) {
        LOG_INFO << "TcpClient::setConnectionCallback: already started.";
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    connectionCallback_ = std::move(callback);
}

void TcpClient::setMessageCallback(TcpMessageCallback callback)
{
    if (started_) {
        LOG_INFO << "TcpClient::setConnectionCallback: already started.";
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    messageCallback_ = std::move(callback);
}

void TcpClient::assertInLoopThread()
{
    loop_->assertInLoopThread();
}

void TcpClient::onConnected(int sockFd)
{
    assertInLoopThread();

    const auto peerAddr = sockets::getPeerAddrInet(sockFd);
    std::string connName = name_ + "-" + peerAddr.toString() + "-" + std::to_string(nextConnId_++);

    auto newConn = std::make_shared<TcpConnection>(loop_, connName, sockFd);
    newConn->setConnectionCallback(connectionCallback_);
    newConn->setMessageCallback(messageCallback_);
    newConn->setCloseCallback([this](const TcpConnectionPtr &conn) { onDisconnected(conn); });
    newConn->onEstablished();

    {
        std::lock_guard<std::mutex> lock(mutex_);
        connection_ = newConn;
    }
}

void TcpClient::onDisconnected(const TcpConnectionPtr &conn)
{
    if (WIND_UNLIKELY(conn == nullptr)) {
        return;
    }

    ASSERT(connection_ == conn);
    ASSERT(loop_ == conn->getOwnerLoop());

    assertInLoopThread();

    {
        std::lock_guard<std::mutex> lock(mutex_);
        connection_ = nullptr;
    }
    conn->onRemoved();

    // TODO: need retry?
}
} // namespace conn
} // namespace wind
