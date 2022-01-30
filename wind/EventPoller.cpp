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
#include "EventChannel.h"
#include "TimeStamp.h"

#include <cstddef>
#include <sys/epoll.h>
#include <unistd.h>
#include <unordered_map>

namespace wind {
namespace detail {

} // namespace detail

size_t EventPoller::eventSize_ = 32;

EventPoller::EventPoller() : epollFd_(::epoll_create1(EPOLL_CLOEXEC)), activeEvents_(eventSize_) {}

EventPoller::~EventPoller() noexcept
{    
}

TimeStamp EventPoller::pollOnce(std::vector<std::shared_ptr<EventChannel>>& activeChannels, int timeOutMs)
{  
    auto cnt = TEMP_FAILURE_RETRY(::epoll_wait(epollFd_.get(), activeEvents_.data(), eventSize_, timeOutMs));
    auto pollTime = TimeStamp::now();
    if (cnt <= 0) {
        // TODO: err log
    } else {
        for (int i = 0; i < cnt; ++i) {
            const auto &event = activeEvents_[i];
            int fd = event.data.fd;
            if (channels_.count(fd) == 0 && channels_.at(fd) == nullptr) {
                // TODO: log
                continue;
            }

            const auto &channel = channels_.at(fd);
            channel->setRecevicedEvents(event.events);
            activeChannels.emplace_back(channels_.at(fd));
        }
    }

    if (static_cast<size_t>(cnt) == eventSize_) {
        eventSize_ *= 2;
        activeEvents_.resize(eventSize_);
    }

    return pollTime;
}

void EventPoller::updateChannel(std::shared_ptr<EventChannel> channel)
{
    if (channel == nullptr) {
        return;
    }

    int channelFd = channel->fd();
    auto channelEvnets = channel->eventsToHandle();
    if (channelEvnets == enum_cast(EventType::NONE)) {
        removeChannel(channelFd);
        return;
    }

    epoll_event epollEvent = {};
    epollEvent.events = channelEvnets; // we assert the EventType is associated with EPOLL_EVENTS
    epollEvent.data.fd = channelFd;
    int ret = 0;
    if (channels_.count(channelFd) == 0) {
        // new channel    
        ret = TEMP_FAILURE_RETRY(::epoll_ctl(epollFd_.get(), EPOLL_CTL_ADD, channelFd, &epollEvent));
    } else {
        // modify channel
        ret = TEMP_FAILURE_RETRY(::epoll_ctl(epollFd_.get(), EPOLL_CTL_MOD, channelFd, &epollEvent));
    }

    if (ret != 0) {
        // TODO: err log.
    } else {
        channels_[channelFd] = std::move(channel);
    }
}

void EventPoller::removeChannel(int fd)
{
    if (channels_.count(fd) == 0) {
        // TODO: log
        return;
    }

    int ret = TEMP_FAILURE_RETRY(::epoll_ctl(epollFd_.get(), EPOLL_CTL_DEL, fd, nullptr));
    if (ret < 0) {
        // TODO: err log
    }
    channels_.erase(fd);
}
} // namespace wind
