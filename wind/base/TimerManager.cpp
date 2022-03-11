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
#include "Log.h"

namespace wind {
namespace base {
namespace detail {
int createTimerFd()
{
    int fd = TEMP_FAILURE_RETRY(::timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC));
    if (isInvalidFd(fd)) {
        LOG_SYS_FATAL << "Create timerfd error: " << strerror(errno) << "!";
    }

    return fd;
}

itimerspec generateTimerSpec(TimeStamp dstTime)
{
    itimerspec newValue;
    utils::memZero(&newValue, sizeof(newValue));

    auto diffMicros = timeDiff(dstTime, TimeStamp::now());
    // set minimum diffMicros to 1 us to make sure the timer can be triggerd.
    diffMicros = std::max(decltype(diffMicros)(1), diffMicros);

    newValue.it_value.tv_sec = diffMicros / MICRO_SECS_PER_SECOND;
    newValue.it_value.tv_nsec = (diffMicros * NANO_SECS_PER_MICROSECOND) % NANO_SECS_PER_SECOND;
    return newValue;
}
} // namespace detail

TimerManager::TimerManager(EventLoop *loop)
    : loop_(loop),
      timerfd_(detail::createTimerFd()),
      timerfdChannel_(std::make_shared<EventChannel>(timerfd_.get(), loop_))
{
    timerfdChannel_->setReadCallback([this](TimeStamp t) { handleRead(t); });
    timerfdChannel_->enableReading();
}

TimerManager::~TimerManager() noexcept
{
    timerfdChannel_->disableAll();
}

TimerId TimerManager::addTimer(TimerCallback callback, TimeStamp expireTime, TimeType interval)
{
    auto future = loop_->schedule([=, cb(std::move(callback))]() mutable -> TimerId {
        auto newTimer = std::make_unique<Timer>(std::move(cb), expireTime, interval);
        TimerId id = newTimer->id();
        addTimerInLoop(std::move(newTimer));
        return id;
    });
    return future.get();
}

void TimerManager::addTimerInLoop(std::unique_ptr<Timer> &&timer)
{
    ASSERT(timer != nullptr);
    assertInLoopThread();

    TimeStamp expireTime = timer->expireTime();
    bool needToResetTimerFd = (timerEntries_.empty() || expireTime < timerEntries_.cbegin()->first);

    TimerId timerId = timer->id();
    timerEntries_.insert(std::make_pair(expireTime, timerId));
    timers_[timerId] = std::move(timer);

    if (needToResetTimerFd) {
        timerfdReset(expireTime);
    }
}

void TimerManager::cancelTimer(const TimerId &timerId)
{
    loop_->runInLoop([=]() { cancelTimerInLoop(timerId); });
}

void TimerManager::cancelTimerInLoop(const TimerId &timerId)
{
    assertInLoopThread();

    auto timer = timerId.timer;
    if (timer == nullptr) {
        return;
    }

    auto timerEntry = std::make_pair(timer->expireTime(), timerId);
    if (timers_.count(timerId) == 0 || timerEntries_.count(timerEntry) == 0) {
        return;
    }

    LOG_DEBUG << "Cancel Timer(id: " << timerId.id << ").";
    bool needToResetTimerFd = (timerEntry == *timerEntries_.cbegin());
    timers_.erase(timerId);
    timerEntries_.erase(timerEntry);

    if (needToResetTimerFd) {
        timerfdReset(timerEntries_.cbegin()->first);
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

    TimerEntry pivot = std::make_pair(receivedTime, TimerId(0, nullptr));
    auto it = timerEntries_.lower_bound(pivot);
    ASSERT(receivedTime <= it->first || it == timerEntries_.cend());
    std::copy(timerEntries_.begin(), it, std::back_inserter(expiredTimers));
    timerEntries_.erase(timerEntries_.begin(), it);

    return expiredTimers;
}

void TimerManager::handleRead(TimeStamp receivedTime)
{
    assertInLoopThread();
    timerfdRead();

    {
        auto expiredTimers = getExpiredTimers(receivedTime);
        for (const auto &[_, timerId] : expiredTimers) {
            ASSERT(timers_.count(timerId) > 0);
            auto &timer = timers_.at(timerId);
            ASSERT(timer != nullptr);
            timer->execute(); // Will implicitly restart the timer if it is repeat.

            if (timer->isRepeat()) {
                timerEntries_.insert(std::make_pair(timer->expireTime(), timerId));
            } else {
                timers_.erase(timerId);
            }
        }
    }

    auto nextExpiredTime = timerEntries_.cbegin()->first;
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
} // namespace base
} // namespace wind
