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
#include <sys/file.h>

#include "base/Log.h"

namespace wind {
namespace conn {

Acceptor::Acceptor(base::EventLoop *eventLoop, const SockAddrInet &listenAddr, bool reusePort, int type, int protocol)
    : loop_(eventLoop),
      acceptSocket_(sockets::createNonBlockSocketOrDie(listenAddr.family(), type, protocol)),
      acceptChannel_(std::make_shared<base::EventChannel>(acceptSocket_.fd(), loop_)),
      idleFd_(base::utils::createIdleFdOrDie()),
      acceptorType_(AcceptorType::INET)
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(reusePort);
    acceptSocket_.bindOrDie(listenAddr);
    acceptChannel_->setReadCallback([this](base::TimeStamp) { handleRead(); });
}

Acceptor::Acceptor(base::EventLoop *eventLoop, const SockAddrUnix &listenAddr, int type, int protocol)
    : loop_(eventLoop),
      acceptSocket_(sockets::createNonBlockSocketOrDie(listenAddr.family(), type, protocol)),
      acceptChannel_(std::make_shared<base::EventChannel>(acceptSocket_.fd(), loop_)),
      idleFd_(base::utils::createIdleFdOrDie()),
      acceptorType_(AcceptorType::LOCAL)
{
    reuseAndLockUnixAddrOrDie(listenAddr.toString());
    acceptSocket_.bindOrDie(listenAddr);
    acceptChannel_->setReadCallback([this](base::TimeStamp) { handleRead(); });
}

Acceptor::~Acceptor() noexcept
{
    if (!lockFileName_.empty()) {
        // FIXME: check ret
        (void)TEMP_FAILURE_RETRY(::flock(sockLockFileFd_.get(), LOCK_UN | LOCK_NB));
        (void)TEMP_FAILURE_RETRY(::unlink(lockFileName_.c_str()));
    }

    acceptChannel_->disableAll();
}

void Acceptor::reuseAndLockUnixAddrOrDie(const std::string &socketPath)
{
    lockFileName_ = socketPath + sockets::SOCKET_LOCK_SUFFIX;
    sockLockFileFd_.reset(TEMP_FAILURE_RETRY(::open(lockFileName_.c_str(), O_RDONLY | O_CREAT, 0600)));
    if (!sockLockFileFd_.valid()) {
        LOG_SYS_FATAL << "Create lock file for Socket(fd: " << acceptSocket_.fd() << ", path: " << socketPath
                      << ") failed: " << strerror(errno) << ".";
    }

    int ret = TEMP_FAILURE_RETRY(::flock(sockLockFileFd_.get(), LOCK_EX | LOCK_NB));
    if (ret < 0) {
        LOG_SYS_FATAL << "flock for fd: " << sockLockFileFd_.get() << " failed: " << strerror(errno) << ".";
    }

    // FIXME: check ret
    (void)TEMP_FAILURE_RETRY(::unlink(socketPath.c_str()));
}

void Acceptor::setAcceptCallback(const AcceptCallbackInet &callback)
{
    if (acceptorType_ != AcceptorType::INET) {
        LOG_WARN << "Unix Acceptor can't set AcceptCallbackInet.";
        return;
    }

    loop_->runInLoop([callback, this]() { inetAcceptCallback_ = callback; });
}

void Acceptor::setAcceptCallback(const AcceptCallbackUnix &callback)
{
    if (acceptorType_ != AcceptorType::LOCAL) {
        LOG_WARN << "Inet Acceptor can't set AcceptCallbackUnix.";
        return;
    }

    loop_->runInLoop([callback, this]() { unixAcceptCallback_ = callback; });
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

void Acceptor::acceptNewInetConn()
{
    SockAddrInet remoteAddr;
    base::UniqueFd remoteFd(acceptSocket_.accept(remoteAddr));
    if (!remoteFd.valid()) {
        LOG_ERROR << "Socket(fd:" << acceptSocket_.fd() << ") failed: " << strerror(errno) << ".";
        handleAcceptError();
    } else {
        if (inetAcceptCallback_ != nullptr) {
            inetAcceptCallback_(remoteFd.release(), remoteAddr);
        }
    }
}

void Acceptor::acceptNewUnixConn()
{
    SockAddrUnix remoteAddr;
    base::UniqueFd remoteFd(acceptSocket_.accept(remoteAddr));
    if (!remoteFd.valid()) {
        LOG_ERROR << "Socket(fd:" << acceptSocket_.fd() << ") failed: " << strerror(errno) << ".";
        handleAcceptError();
    } else {
        if (unixAcceptCallback_ != nullptr) {
            unixAcceptCallback_(remoteFd.release(), remoteAddr);
        }
    }
}

void Acceptor::handleAcceptError()
{
    // "The special problem of accept()ing when you can't"
    // By Marc Lehmann, author of libev.
    if (errno == EMFILE) {
        idleFd_.reset();
        base::UniqueFd remoteFd(sockets::accept(acceptSocket_.fd(), nullptr, 0));
        LOG_INFO << "It is too busy to accept new connection.";
        remoteFd.reset();
        idleFd_.reset(base::utils::createIdleFdOrDie());
    }
}

void Acceptor::handleRead()
{
    assertInLoopThread();

    switch (acceptorType_) {
        case AcceptorType::INET:
            acceptNewInetConn();
            break;
        case AcceptorType::LOCAL:
            acceptNewUnixConn();
            break;
        default:
            LOG_ERROR << "Not supported acceptor type.";
            break;
    }
}
} // namespace conn
} // namespace wind
