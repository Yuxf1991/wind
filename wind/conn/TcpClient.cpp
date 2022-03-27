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
#include "TcpConnection.h"

namespace wind {
namespace conn {
using namespace base;
TcpClient::TcpClient(base::EventLoop *loop, string name, const SockAddrInet &remoteAddr)
    : loop_(loop), name_(std::move(name)), connector_(std::make_shared<Connector>(loop, remoteAddr))
{
    connector_->setOnConnectCallback([this](int sockFd) { onConnect(sockFd); });
}

TcpClient::~TcpClient() noexcept {}

void TcpClient::start()
{
    if (started_) {
        return;
    }

    started_ = true;
    LOG_INFO << "TcpClient(" << name_ << ") start connecting to " << connector_->remoteAddr() << "...";
    connector_->start();
}

void TcpClient::stop()
{
    started_ = false;

    LOG_INFO << "TcpClient(" << name_ << ") stop connecting.";
    connector_->stop();
}

void TcpClient::assertInLoopThread()
{
    loop_->assertInLoopThread();
}

void TcpClient::onConnect(int sockFd)
{
    assertInLoopThread();

    const auto peerAddr = sockets::getPeerAddrInet(sockFd);
    string connName = name_ + "-" + peerAddr.toString() + "-" + std::to_string(connId_++);
    auto newConn = std::make_shared<TcpConnection>(loop_, connName, sockFd);

    {
        std::lock_guard<std::mutex> lock(mutex_);
        connection_ = newConn;
    }
}
} // namespace conn
} // namespace wind
