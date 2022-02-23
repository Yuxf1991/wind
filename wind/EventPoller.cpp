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

#include "EventPoller.h"

#include "EventLoop.h"
#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "EventPoller"
#endif // LOG_TAG
#include "Log.h"
#include "Utils.h"

namespace wind {
namespace detail {
string epollOperationToString(int operation)
{
    switch (operation) {
        case EPOLL_CTL_ADD: return "EpollCtlAdd";
        case EPOLL_CTL_MOD: return "EpollCtlMod";
        case EPOLL_CTL_DEL: return "EpollCtlDel";
        default: return "UnknownEpollCtl";
    }
}
} // namespace detail

size_t EventPoller::eventSize_ = 32;

EventPoller::EventPoller(EventLoop *eventLoop)
    : eventLoop_(eventLoop), epollFd_(::epoll_create1(EPOLL_CLOEXEC)), activeEvents_(eventSize_)
{
    LOG_FATAL_IF(eventLoop_ == nullptr) << "EventLoop is null!";
}

EventPoller::~EventPoller() noexcept {}

TimeStamp EventPoller::pollOnce(std::vector<std::shared_ptr<EventChannel>> &activeChannels, int timeOutMs)
{
    auto cnt = TEMP_FAILURE_RETRY(::epoll_wait(epollFd_.get(), activeEvents_.data(), eventSize_, timeOutMs));
    auto pollTime = TimeStamp::now();
    if (cnt < 0) {
        LOG_WARN << "epoll_wait error: " << strerror(errno) << ".";
    } else {
        for (int i = 0; i < cnt; ++i) {
            const auto &event = activeEvents_[i];
            int fd = event.data.fd;
            if (channels_.count(fd) == 0 && channels_.at(fd) == nullptr) {
                LOG_WARN << "epoll_wait returned a channel that is not in poller " << epollFd_.get() << ".";
                continue;
            }

            const auto &channel = channels_.at(fd);
            channel->setRecevicedEvents(event.events);
            activeChannels.emplace_back(channel);
        }

        if (static_cast<size_t>(cnt) == eventSize_) {
            eventSize_ *= 2;
            activeEvents_.resize(eventSize_);
        }
    }

    return pollTime;
}

void EventPoller::assertInLoopThread()
{
    ASSERT(eventLoop_ != nullptr);
    eventLoop_->assertInLoopThread();
}

void EventPoller::epollCtl(const std::shared_ptr<EventChannel> &channel, int operation)
{
    ASSERT(channel != nullptr);
    int fd = channel->fd();
    epoll_event epollEvent{0};
    epollEvent.events = channel->listeningEvents();
    epollEvent.data.fd = fd;
    int ret = TEMP_FAILURE_RETRY(::epoll_ctl(epollFd_.get(), operation, fd, &epollEvent));
    if (ret < 0) {
        LOG_ERROR << detail::epollOperationToString(operation) << " failed for fd " << fd << ": " << strerror(errno)
                  << "!";
    }
}

void EventPoller::updateChannel(std::shared_ptr<EventChannel> channel)
{
    if (channel == nullptr) {
        return;
    }

    eventLoop_->assertInLoopThread();

    int fd = channel->fd();
    if (channel->hasNoEvent()) {
        eventLoop_->removeChannel(fd);
        return;
    }

    if (channels_.count(fd) == 0) {
        // new channel
        epollCtl(channel, EPOLL_CTL_ADD);
        channels_[fd] = std::move(channel);
    } else {
        // modify channel
        epollCtl(channels_[fd], EPOLL_CTL_MOD);
    }
}

void EventPoller::removeChannel(int fd)
{
    eventLoop_->assertInLoopThread();

    if (channels_.count(fd) == 0) {
        LOG_WARN << "Can't find channel " << fd << " in poller " << epollFd_.get() << ".";
        return;
    }

    epollCtl(channels_[fd], EPOLL_CTL_DEL);
    channels_.erase(fd);
}
} // namespace wind
