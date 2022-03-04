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

#include "Acceptor.h"

#include <fcntl.h>

namespace wind {
namespace conn {
namespace detail {
int createIdleFd()
{
    int idleFd = TEMP_FAILURE_RETRY(::open("/dev/null", O_RDONLY | O_CLOEXEC));
    if (base::isInvalidFd(idleFd)) {
        LOG_SYS_FATAL << "Create idleFd failed: " << strerror(errno) << ".";
    }
    return idleFd;
}
} // namespace detail

Acceptor::Acceptor(base::EventLoop *eventLoop, const SockAddrInet &listenAddr, bool reusePort, int type)
    : loop_(eventLoop), acceptSocket_(Socket::createNonBlockSocket(listenAddr.family(), type, 0)),
      acceptChannel_(std::make_shared<base::EventChannel>(acceptSocket_.fd(), loop_)), idleFd_(detail::createIdleFd())
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(reusePort);
    acceptChannel_->setReadCallback([this](base::TimeStamp) { handleRead(); });

    loop_->runInLoop([=]() { acceptSocket_.bindOrDie(listenAddr); });
}

Acceptor::~Acceptor() noexcept
{
    acceptChannel_->disableAll();
}

void Acceptor::setAcceptCallback(const AcceptCallback &callback)
{
    loop_->runInLoop([callback, this]() { acceptCallback_ = callback; });
}

void Acceptor::listen()
{
    if (listening()) {
        return;
    }

    listening_ = true;

    loop_->runInLoop([this]() {
        acceptSocket_.listenOrDie();
        acceptChannel_->enableReading();
    });
}

void Acceptor::assertInLoopThread() const
{
    loop_->assertInLoopThread();
}

void Acceptor::handleRead()
{
    assertInLoopThread();
    SockAddrInet remoteAddr;
    base::UniqueFd remoteFd(acceptSocket_.accept(remoteAddr));
    if (!remoteFd.valid()) {
        LOG_ERROR << "Socket(fd:" << acceptSocket_.fd() << ") failed: " << strerror(errno) << ".";
        // "The special problem of accept()ing when you can't"
        // By Marc Lehmann, author of libev.
        if (errno == EMFILE) {
            idleFd_.reset();
            remoteFd.reset(acceptSocket_.accept(remoteAddr));
            LOG_INFO << "It is too busy, so close new connection(" << remoteAddr.ipPortString() << ").";
            remoteFd.reset();
            idleFd_.reset(detail::createIdleFd());
        }
    } else {
        if (acceptCallback_ != nullptr) {
            acceptCallback_(remoteFd.release(), remoteAddr);
        }
    }
}
} // namespace conn
} // namespace wind
