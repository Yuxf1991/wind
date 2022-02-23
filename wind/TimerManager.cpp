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

#include "TimerManager.h"

#include <sys/timerfd.h>

#include "EventLoop.h"

namespace wind {
namespace detail {
int createTimerFd()
{
    int fd = ::timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);
    if (INVALID_FD(fd)) {
        LOG_SYS_FATAL << "Create timerfd error: " << strerror(errno) << "!";
    }

    return fd;
}

itimerspec generateTimerSpec(TimeStamp dstTime)
{
    itimerspec newValue;
    utils::memZero(&newValue, sizeof(newValue));

    auto diffNanos = timeDiff(dstTime, TimeStamp::now());
    if (diffNanos < 1000) {
        // set minimum nanos to 1000 nanoseconds(1 us) to make sure the timer can be triggerd.
        diffNanos = 1000;
    }

    newValue.it_value.tv_sec = diffNanos / NANO_SECS_PER_SECOND;
    newValue.it_value.tv_nsec = diffNanos % NANO_SECS_PER_SECOND;
    return newValue;
}
} // namespace detail

TimerManager::TimerManager(EventLoop *loop)
    : loop_(loop), timerfd_(detail::createTimerFd()),
      timerfdChannel_(std::make_shared<EventChannel>(timerfd_.get(), loop_))
{
    timerfdChannel_->setReadCallback([this](TimeStamp t) { handleRead(t); });
    timerfdChannel_->enableReading();
}

TimerManager::~TimerManager() noexcept
{
    loop_->runInLoop([this]() { timerfdChannel_->disableAll(); });
}

TimerId TimerManager::addTimer(TimerCallback callback, TimeStamp expireTime, TimeType interval)
{
    return loop_
        ->schedule([=, cb(std::move(callback))]() mutable -> TimerId {
            auto newTimer = std::make_shared<Timer>(std::move(cb), expireTime, interval);
            TimerId id = newTimer->id();
            addTimerInLoop(newTimer);
            return id;
        })
        .get();
}

void TimerManager::addTimerInLoop(std::shared_ptr<Timer> timer)
{
    if (timer == nullptr) {
        return;
    }

    assertInLoopThread();
    TimeStamp dst = timer->expireTime();

    bool needToResetTimerFd = false;
    TimeStamp expireTime;
    if (timers_.empty()) {
        needToResetTimerFd = true;
        expireTime = dst;
    } else {
        auto firstExpireTime = timers_.cbegin()->first;
        if (firstExpireTime > dst) {
            needToResetTimerFd = true;
            expireTime = dst;
        } else {
            expireTime = firstExpireTime;
        }
    }

    timers_.insert(std::make_pair(dst, std::move(timer)));

    if (needToResetTimerFd) {
        timerfdReset(expireTime);
    }
}

void TimerManager::assertInLoopThread()
{
    ASSERT(loop_ != nullptr);
    loop_->assertInLoopThread();
}

std::vector<TimerEntry> TimerManager::getExpiredTimers(TimeStamp receivedTime)
{
    std::vector<TimerEntry> expiredTimers;
    TimerEntry pivot = std::make_pair(receivedTime, std::make_shared<Timer>(nullptr, TimeStamp()));
    auto it = timers_.lower_bound(pivot);
    ASSERT(receivedTime < it->first || it == timers_.cend());
    std::move(timers_.begin(), it, back_inserter(expiredTimers));
    timers_.erase(timers_.begin(), it);

    return expiredTimers;
}

void TimerManager::handleRead(TimeStamp receivedTime)
{
    assertInLoopThread();
    timerfdRead();

    {
        auto expiredTimers = getExpiredTimers(receivedTime);
        for (auto &[expiredTime, timer] : expiredTimers) {
            ASSERT(timer != nullptr);
            timer->execute();
            if (timer->isRepeat()) {
                TimeStamp expiredTime = timer->expireTime();
                timers_.insert(std::make_pair(expiredTime, timer));
            }
        }
    }

    auto nextExpiredTime = timers_.cbegin()->first;
    timerfdReset(nextExpiredTime);
}

void TimerManager::timerfdRead()
{
    uint64_t one = 0;
    int len = TEMP_FAILURE_RETRY(::read(timerfd_.get(), &one, sizeof(one)));
    if (len != sizeof(one)) {
        LOG_WARN << "Read from timerfd " << timerfd_.get() << " " << len << " bytes, should be " << sizeof(one)
                 << " bytes.";
    }
}

void TimerManager::timerfdReset(TimeStamp expireTime)
{
    auto newValue = detail::generateTimerSpec(expireTime);
    int ret = TEMP_FAILURE_RETRY(::timerfd_settime(timerfd_.get(), 0, &newValue, nullptr));
    if (ret != 0) {
        LOG_SYS_FATAL << "timerfd_settime error: " << strerror(errno) << "!";
    }
}
} // namespace wind
