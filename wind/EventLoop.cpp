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
#include "Timer.h"
#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "EventLoop"
#endif // LOG_TAG
#include "Log.h"

namespace wind {
namespace detail {
EventCallbackWithStamp toTimerCb(EventCallback cb, int timerFd)
{
    auto newCb = [oldCb(std::move(cb)), timerFd](TimeStamp) {
        uint64_t one;
        int len = TEMP_FAILURE_RETRY(read(timerFd, &one, sizeof(one)));
        if (len != sizeof(one)) {
            LOG_WARN << "Read from timerfd " << timerFd << " " << len << " bytes, should be " << sizeof(one)
                     << " bytes.";
        }
        oldCb();
    };
    return newCb;
}
} // namespace detail

__thread EventLoop *t_currLoop = nullptr; // current thread's event_loop

EventLoop::EventLoop() :
    poller_(std::make_unique<EventPoller>(this)),
    wakeUpChannel_(std::make_shared<EventChannel>(utils::createEventFd(), this)), tid_(CurrentThread::tid())
{
    wakeUpChannel_->setReadCallback([this](TimeStamp) { wakeUpCallback(); });
    updateChannel(wakeUpChannel_);
    t_currLoop = this;
}

EventLoop::~EventLoop() noexcept
{
    stop();

    for (auto &[fd, channel] : holdChannels_) {
        poller_->removeChannel(fd);
    }
    holdChannels_.clear();
}

void EventLoop::stop()
{
    if (!isInLoopThread()) {
        running_ = false;
        wakeUp();
    }
}

void EventLoop::updateChannel(const std::shared_ptr<EventChannel> &channel)
{
    if (channel == nullptr) {
        LOG_WARN << "EventLoop::" << __func__ << ": channel is null!";
        return;
    }

    int channelFd = channel->fd();
    if (holdChannels_.count(channelFd) > 0 && holdChannels_[channelFd] != channel) {
        // remove old channel first
        removeChannel(channelFd);
    }

    holdChannels_[channelFd] = channel;
    poller_->updateChannel(channel);

    if (!isInLoopThread()) {
        wakeUp();
    }
}

void EventLoop::removeChannel(int channelFd)
{
    poller_->removeChannel(channelFd);
    holdChannels_.erase(channelFd);
}

void EventLoop::runAt(EventCallback cb, TimeType delay)
{
    auto newTimer = std::make_shared<Timer>(this, delay);
    newTimer->setReadCallback(detail::toTimerCb(cb, newTimer->fd()));
    updateChannel(newTimer);
}

void EventLoop::runEvery(EventCallback cb, TimeType interval, TimeType delay)
{
    auto newTimer = std::make_shared<Timer>(this, delay, interval);
    newTimer->setReadCallback(detail::toTimerCb(cb, newTimer->fd()));
    updateChannel(newTimer);
}

void EventLoop::run()
{
    assertInLoopThread();

    running_ = true;
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

bool EventLoop::isInLoopThread()
{
    return CurrentThread::tid() == tid_;
}

void EventLoop::assertInLoopThread()
{
    LOG_FATAL_IF(!isInLoopThread()) << "assertInLoopThread failed!";
}

void EventLoop::assertNotInLoopThread()
{
    LOG_FATAL_IF(isInLoopThread()) << "assertInLoopThread failed!";
}

void EventLoop::wakeUp()
{
    uint64_t buf = 1;
    int len = TEMP_FAILURE_RETRY(::write(wakeUpChannel_->fd(), &buf, sizeof(buf)));
    if (len != sizeof(buf)) {
        LOG_WARN << "should write " << sizeof(buf) << " bytes, but " << len << " wrote.";
    }
}

void EventLoop::wakeUpCallback()
{
    uint64_t buf = 0;
    int len = TEMP_FAILURE_RETRY(::read(wakeUpChannel_->fd(), &buf, sizeof(buf)));
    if (len != sizeof(buf)) {
        LOG_WARN << "should read " << sizeof(buf) << " bytes, but " << len << " read.";
    }
}

EventLoop *EventLoop::eventLoopOfCurrThread()
{
    return t_currLoop;
}
} // namespace wind
