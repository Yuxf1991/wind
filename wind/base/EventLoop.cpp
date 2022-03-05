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

#include "CurrentThread.h"
#include "Log.h"

namespace wind {
namespace base {
__thread EventLoop *t_currLoop = nullptr; // current thread's event_loop

EventLoop::EventLoop()
    : tid_(CurrentThread::tid()), poller_(std::make_unique<EventPoller>(this)), wakeUpFd_(utils::createEventFdOrDie()),
      wakeUpChannel_(std::make_shared<EventChannel>(wakeUpFd_.get(), this)),
      timerManager_(std::make_unique<TimerManager>(this))
{
    if (t_currLoop != nullptr) {
        LOG_SYS_FATAL << "Construct EventLoop failed: current thread already have a loop(" << &t_currLoop << ")!";
    }

    // wakeUpCallback do not need TimeStamp
    wakeUpChannel_->setReadCallback([this](TimeStamp) { wakeUpCallback(); });
    wakeUpChannel_->enableReading();
    t_currLoop = this;
}

EventLoop::~EventLoop() noexcept
{
    wakeUpChannel_->disableAll();
    stop();
    t_currLoop = nullptr;
}

void EventLoop::stop() noexcept
{
    running_ = false;

    if (!isInLoopThread()) {
        wakeUp();
    }
}

void EventLoop::updateChannel(std::shared_ptr<EventChannel> channel)
{
    if (channel == nullptr) {
        LOG_WARN << "EventLoop::" << __func__ << ": channel is null!";
        return;
    }

    runInLoop([this, channel]() { poller_->updateChannel(std::move(channel)); });
}

void EventLoop::removeChannel(int channelFd)
{
    runInLoop([=]() { poller_->removeChannel(channelFd); });
}

void EventLoop::execPendingFunctors()
{
    assertInLoopThread();

    executingPendingFunctors_ = true;
    std::vector<Functor> funcs;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        funcs.swap(pendingFunctors_);
    }

    for (const auto &func : funcs) {
        func();
    }
    executingPendingFunctors_ = false;
}

void EventLoop::queueToPendingFunctors(Functor func)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingFunctors_.push_back(std::move(func));
    }

    if (!isInLoopThread() || executingPendingFunctors_) {
        wakeUp();
    }
}

void EventLoop::runInLoop(Functor func)
{
    if (isInLoopThread()) {
        func();
    } else {
        queueToPendingFunctors(func);
    }
}

TimerId EventLoop::runAt(Functor func, TimeStamp dstTime)
{
    return timerManager_->addTimer(func, dstTime);
}

TimerId EventLoop::runAfter(Functor func, TimeType delay)
{
    return timerManager_->addTimer(func, timeAdd(TimeStamp::now(), delay));
}

TimerId EventLoop::runEvery(Functor func, TimeType interval, TimeType delay)
{
    return timerManager_->addTimer(func, timeAdd(TimeStamp::now(), delay), interval);
}

void EventLoop::cancel(const TimerId &timerId)
{
    runInLoop([this, &timerId]() { timerManager_->cancelTimerInLoop(timerId); });
}

void EventLoop::start()
{
    assertInLoopThread();

    running_ = true;
    while (running_) {
        std::vector<std::shared_ptr<EventChannel>> activeChannels;
        TimeStamp pollTime = poller_->pollOnce(activeChannels, -1);
        for (const auto &channel : activeChannels) {
            if (channel != nullptr) {
                channel->handleEvent(pollTime);
            }
        }

        execPendingFunctors();
    }
}

bool EventLoop::isInLoopThread() const
{
    return CurrentThread::tid() == tid_;
}

void EventLoop::assertInLoopThread() const
{
    if (WIND_UNLIKELY(!isInLoopThread())) {
        LOG_SYS_FATAL << "assertInLoopThread failed!";
    }
}

void EventLoop::assertNotInLoopThread() const
{
    if (WIND_UNLIKELY(isInLoopThread())) {
        LOG_SYS_FATAL << "assertNotInLoopThread failed!";
    }
}

void EventLoop::wakeUp()
{
    uint64_t buf = 1;
    int len = TEMP_FAILURE_RETRY(::write(wakeUpChannel_->fd(), &buf, sizeof(buf)));
    if (WIND_UNLIKELY(len != sizeof(buf))) {
        LOG_WARN << "should write " << sizeof(buf) << " bytes, but " << len << " wrote.";
    }
}

void EventLoop::wakeUpCallback()
{
    uint64_t buf = 0;
    int len = TEMP_FAILURE_RETRY(::read(wakeUpChannel_->fd(), &buf, sizeof(buf)));
    if (WIND_UNLIKELY(len != sizeof(buf))) {
        LOG_WARN << "should read " << sizeof(buf) << " bytes, but " << len << " read.";
    }
}

EventLoop *EventLoop::eventLoopOfCurrThread()
{
    return t_currLoop;
}
} // namespace base
} // namespace wind
