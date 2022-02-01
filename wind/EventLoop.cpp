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

#include "EventLoop.h"

#include <vector>

#include "CurrentThread.h"
#include "Utils.h"

namespace wind {
__thread EventLoop *t_currLoop = nullptr; // current thread's event_loop

EventLoop::EventLoop() :
    poller_(std::make_unique<EventPoller>(this)),
    wakeUpChannel_(std::make_shared<EventChannel>(utils::createEventFd(), this))
{
    wakeUpChannel_->setReadCallback([this](TimeStamp) { wakeUpCallback(); });
    t_currLoop = this;
}

EventLoop::~EventLoop() noexcept {}

void EventLoop::stop()
{
    running_ = false;
    wakeUp();
}

void EventLoop::updateChannel(const std::shared_ptr<EventChannel> &channel)
{
    if (channel == nullptr) {
        // TODO: log
        return;
    }

    int channelFd = channel->fd();
    if (holdChannels_.count(channelFd) > 0 && holdChannels_[channelFd] != channel) {
        // remove old channel first
        removeChannel(channelFd);
    }

    holdChannels_[channelFd] = channel;
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(int channelFd)
{
    poller_->removeChannel(channelFd);
    holdChannels_.erase(channelFd);
}

void EventLoop::run()
{
    running_ = true;
    tid_ = CurrentThread::tid();
    while (running_) {
        std::vector<std::shared_ptr<EventChannel>> activeChannels;
        TimeStamp pollTime = poller_->pollOnce(activeChannels, -1);
        for (auto &channel : activeChannels) {
            if (channel != nullptr) {
                channel->handleEvent(pollTime);
            }
        }
    }
}

void EventLoop::assertInLoopThread()
{
    if (CurrentThread::tid() != tid_) {
        // TODO: ERR LOG
        abort();
    }
}

void EventLoop::assertNotInLoopThread()
{
    if (CurrentThread::tid() == tid_) {
        // TODO: ERR LOG
        abort();
    }
}

void EventLoop::wakeUp()
{
    uint64_t buf = 1;
    int len = TEMP_FAILURE_RETRY(::write(wakeUpChannel_->fd(), &buf, sizeof(buf)));
    if (len != sizeof(buf)) {
        // TODO: ERR LOG
    }
}

void EventLoop::wakeUpCallback()
{
    uint64_t buf = 0;
    int len = TEMP_FAILURE_RETRY(::read(wakeUpChannel_->fd(), &buf, sizeof(buf)));
    if (len != sizeof(buf)) {
        // TODO: ERR LOG
    }
}

EventLoop *EventLoop::eventLoopOfCurrThread()
{
    return t_currLoop;
}
} // namespace wind
