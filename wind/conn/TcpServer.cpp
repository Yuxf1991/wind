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

#include "TcpServer.h"

namespace wind {
namespace conn {
using namespace base;

namespace detail {
inline uint64_t getConnSequenceNum()
{
    static std::atomic<uint64_t> sequence(0);
    return sequence.fetch_add(1, std::memory_order_relaxed);
}
} // namespace detail

TcpServer::TcpServer(base::EventLoop *loop, const SockAddrInet &listenAddr, string name, bool reusePort)
    : mainLoop_(loop),
      name_(std::move(name)),
      threadPool_(std::make_unique<EventLoopThreadPool>(mainLoop_, "TcpServerThreadPool")),
      acceptor_(std::make_unique<Acceptor>(mainLoop_, listenAddr, reusePort))
{
    acceptor_->setAcceptCallback(
        [this](int peerFd, const SockAddrInet &peerAddr) { onNewConnection(peerFd, peerAddr); });
}

TcpServer::~TcpServer() noexcept {}

void TcpServer::start()
{
    if (running_) {
        return;
    }

    running_ = true;
    mainLoop_->runInLoop([this]() {
        threadPool_->start();
        acceptor_->listen();
    });
}

void TcpServer::assertInMainLoopThread()
{
    mainLoop_->assertInLoopThread();
}

void TcpServer::onNewConnection(int peerFd, const SockAddrInet &peerAddr)
{
    assertInMainLoopThread();

    EventLoop *ioLoop = threadPool_->getNextLoop();
    string connName = name_ + "-" + peerAddr.toString() + "-" + std::to_string(detail::getConnSequenceNum());
    auto newConn = std::make_shared<TcpConnection>(ioLoop, connName, peerFd);
    conns_[connName] = newConn;
    LOG_INFO << "New connection in: " << newConn->name() << ", localAddr: " << newConn->getLocalAddr().toString()
             << ", peerAddr: " << newConn->getPeerAddr().toString();
}
} // namespace conn
} // namespace wind
