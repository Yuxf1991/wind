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

#include "Connector.h"

#include <fcntl.h>
#include <sys/file.h>

#include "base/CurrentThread.h"
#include "base/Log.h"

#include "Socket.h"

namespace wind {
namespace conn {
using namespace base;

namespace detail {
const char *LOCK_SUFFIX = ".lock";
} // namespace detail

Connector::Connector(EventLoop *loop, const SockAddrInet &remoteAddr, int dataType, int protocol)
    : loop_(loop),
      inetRemoteAddr_(remoteAddr),
      dataType_(dataType),
      protocol_(protocol),
      connectorType_(ConnectorType::INET)
{
    if (loop_ == nullptr) {
        LOG_SYS_FATAL << "Connector::ctor: EventLoop is null!";
    }
}

Connector::Connector(EventLoop *loop, const SockAddrUnix &remoteAddr, int dataType, int protocol)
    : loop_(loop),
      unixRemoteAddr_(remoteAddr),
      dataType_(dataType),
      protocol_(protocol),
      connectorType_(ConnectorType::LOCAL)
{
    if (loop_ == nullptr) {
        LOG_SYS_FATAL << "Connector::ctor: EventLoop is null!";
    }
}

Connector::~Connector() noexcept
{
    removeChannel();
}

void Connector::setOnConnectedCallback(OnConnectedCallback callback)
{
    if (started_) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    onConnectedCallback_ = std::move(callback);
}

void Connector::stop()
{
    started_ = false;
    auto future = loop_->schedule([this]() {
        if (state_ == State::CONNECTING) {
            state_ = State::DISCONNECTED;
            removeChannel();
            if (retryingTimer_.has_value()) {
                loop_->cancel(retryingTimer_.value());
            }
        }
    });
    future.wait();
}

void Connector::restart()
{
    assertInLoopThread();

    if (started_) {
        stop();
    }

    state_ = State::DISCONNECTED;
    retryDelay_ = INITIAL_RETRY_DELAY;
    started_ = true;
    startInLoop();
}

void Connector::start()
{
    if (started_) {
        return;
    }

    started_ = true;
    loop_->runInLoop([this]() { startInLoop(); });
}

void Connector::assertInLoopThread()
{
    loop_->assertInLoopThread();
}

void Connector::startInLoop()
{
    assertInLoopThread();
    ASSERT(state_ == State::DISCONNECTED);
    if (!started_) {
        return;
    }

    switch (connectorType_) {
        case ConnectorType::INET:
            tryConnectInet();
            break;
        case ConnectorType::LOCAL:
            tryConnectUnix();
            break;
        default:
            LOG_WARN << "Unknown connector type!";
            break;
    }
}

void Connector::tryConnectInet()
{
    UniqueFd sockFd(sockets::createNonBlockSocketOrDie(inetRemoteAddr_.family(), dataType_, protocol_));
    int ret = sockets::connect(sockFd.get(), inetRemoteAddr_.getSockAddr(), inetRemoteAddr_.len());
    int savedErrno = (ret == 0) ? 0 : errno;
    handleConnectResult(std::move(sockFd), savedErrno);
}

void Connector::unLockUnixAddr()
{
    if (!lockFileName_.empty()) {
        // FIXME: check ret
        (void)TEMP_FAILURE_RETRY(::flock(sockLockFileFd_.get(), LOCK_UN | LOCK_NB));
        (void)TEMP_FAILURE_RETRY(::unlink(lockFileName_.c_str()));
    }
}

void Connector::reuseAndLockUnixAddrOrDie(const std::string &socketPath)
{
    unLockUnixAddr();

    lockFileName_ = socketPath + detail::LOCK_SUFFIX;
    sockLockFileFd_.reset(TEMP_FAILURE_RETRY(::open(lockFileName_.c_str(), O_RDONLY | O_CREAT, 0600)));
    if (!sockLockFileFd_.valid()) {
        LOG_SYS_FATAL << "Create lock file path: " << socketPath << ") failed: " << strerror(errno) << ".";
    }

    int ret = TEMP_FAILURE_RETRY(::flock(sockLockFileFd_.get(), LOCK_EX | LOCK_NB));
    if (ret < 0) {
        LOG_SYS_FATAL << "flock for fd: " << sockLockFileFd_.get() << " failed: " << strerror(errno) << ".";
    }

    // FIXME: check ret
    (void)TEMP_FAILURE_RETRY(::unlink(socketPath.c_str()));
}

void Connector::tryConnectUnix()
{
    UniqueFd sockFd(sockets::createNonBlockSocketOrDie(unixRemoteAddr_.family(), dataType_, protocol_));
    SockAddrUnix localAddr(unixRemoteAddr_.toString() + "-" + CurrentThread::pidString());
    reuseAndLockUnixAddrOrDie(localAddr.toString());
    sockets::bindOrDie(sockFd.get(), localAddr.getSockAddr(), localAddr.len());
    int ret = sockets::connect(sockFd.get(), unixRemoteAddr_.getSockAddr(), unixRemoteAddr_.len());
    int savedErrno = (ret == 0) ? 0 : errno;
    handleConnectResult(std::move(sockFd), savedErrno);
}

void Connector::establish(UniqueFd &&sockFd)
{
    LOG_INFO << "Connector::establish: connect to " << remoteAddr() << " succeed.";
    auto fd = std::move(sockFd);
    state_ = State::CONNECTED;
    if (onConnectedCallback_ != nullptr) {
        // The callback should take the ownership of this socket fd.
        onConnectedCallback_(fd.release());
    }
}

void Connector::removeChannel()
{
    if (channel_ == nullptr) {
        return;
    }
    channel_->disableAll(true);

    // we can't reset channel_ immediately since we may in channel_'s handleEvent() probably.
    loop_->queueToLoop([this]() { channel_ = nullptr; });
}

// check if isSelfConnect
bool Connector::isSelfConnect(int sockFd)
{
    bool isSelfConnect = false;
    switch (connectorType_) {
        case ConnectorType::INET:
            isSelfConnect = sockets::isSelfConnectInet(sockFd);
            break;
        case ConnectorType::LOCAL:
            isSelfConnect = sockets::isSelfConnectUnix(sockFd);
            break;
        default:
            // unexpected.
            break;
    }
    return isSelfConnect;
}

void Connector::onChannelWritable()
{
    ASSERT(channel_ != nullptr); // Because we are in channel_'s handleEvent.

    UniqueFd sockFd(channel_->fd());
    removeChannel();
    if (state_ == State::CONNECTING) {
        int err = sockets::getSocketError(sockFd.get());
        if (err == 0) {
            if (isSelfConnect(sockFd.get())) {
                LOG_WARN << "Connector::onChannelWritable: self connected, retry...";
                retry();
            } else {
                establish(std::move(sockFd));
            }
        } else {
            LOG_WARN << "Connector::onChannelWritable: Socket err: " << strerror(err) << ", retry...";
            retry();
        }
    } else {
        // unexpected
        LOG_ERROR << "Connector::onChannelWritable unexpected error.";
        state_ = State::DISCONNECTED;
    }
}

void Connector::onChannelError()
{
    ASSERT(channel_ != nullptr); // Because we are in channel_'s handleEvent.

    LOG_ERROR << "Connector::onChannelError: state = " << stateString();
    if (state_ == State::CONNECTING) {
        UniqueFd fd(channel_->fd());
        removeChannel();
        LOG_DEBUG << "Connector::onChannelError: socket error: " << strerror(sockets::getSocketError(fd.get())) << ".";
        retry();
    } else {
        // unexpected
        LOG_ERROR << "Connector::onChannelError unexpected error.";
        state_ = State::DISCONNECTED;
    }
}

std::string Connector::stateString() const
{
    std::string res = "UNKNOWN";
    switch (state_) {
        case State::DISCONNECTED:
            res = "DISCONNECTED";
            break;
        case State::CONNECTING:
            res = "CONNECTING";
            break;
        case State::CONNECTED:
            res = "CONNECTED";
            break;
    }
    return res;
}

std::string Connector::remoteAddr() const
{
    std::string remoteAddr;
    switch (connectorType_) {
        case ConnectorType::INET:
            remoteAddr = inetRemoteAddr_.toString();
            break;
        case ConnectorType::LOCAL:
            remoteAddr = unixRemoteAddr_.toString();
            break;
        default:
            remoteAddr = "UNKNOWN";
            break;
    }
    return remoteAddr;
}

void Connector::retry()
{
    state_ = State::DISCONNECTED;
    if (!started_) {
        LOG_WARN << "Connector::retry: do not started.";
        return;
    }

    LOG_INFO << "Connector: retry connecting to " << remoteAddr() << " in " << retryDelay_ << " milliseconds.";
    retryingTimer_ = loop_->runAfter([this]() { startInLoop(); }, retryDelay_ * base::MICRO_SECS_PER_MILLISECOND);
    retryDelay_ = std::min(MAX_RETRY_DELAY, retryDelay_ * 2);
}

void Connector::connecting(int sockFd)
{
    ASSERT(state_ == State::DISCONNECTED);
    state_ = State::CONNECTING;

    ASSERT(channel_ == nullptr);
    channel_ = std::make_shared<EventChannel>(sockFd, loop_);
    channel_->setWriteCallback([this]() { onChannelWritable(); });
    channel_->setErrorCallback([this]() { onChannelError(); });
    channel_->enableWriting(true);
}

void Connector::handleConnectResult(UniqueFd &&sockFd, int savedErrno)
{
    assertInLoopThread();
    UniqueFd fd = std::move(sockFd);
    switch (savedErrno) {
        case 0:
            establish(std::move(fd));
            break;
        case EINPROGRESS:
        case EISCONN:
            connecting(fd.release());
            break;
        case EAGAIN:
        case EADDRINUSE:
        case EADDRNOTAVAIL:
        case ECONNREFUSED:
        case ENETUNREACH:
            retry();
            break;
        default:
            LOG_ERROR << "Connector: tryConnect error: " << strerror(savedErrno) << ".";
            break;
    }
}
} // namespace conn
} // namespace wind
