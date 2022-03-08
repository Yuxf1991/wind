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

#pragma once

#include <set>
#include <unordered_map>

#include "EventChannel.h"
#include "Timer.h"
#include "UniqueFd.h"

namespace wind {
namespace base {
class EventLoop;

using TimerPtr = std::unique_ptr<Timer>;
using TimerMap = std::unordered_map<TimerId, TimerPtr>; // To hold timers, manager the timers' onwership.
using TimerEntry = std::pair<TimeStamp, TimerId>;       // Make sure every TimerEntry is unique.
using TimerEntrySet = std::set<TimerEntry>;             // To sort timers ordered by expireTime
class TimerManager : NonCopyable {
public:
    TimerManager(EventLoop *loop);
    ~TimerManager() noexcept;

    // @callback: TimerCallback
    // @expireTime: expire TimeStamp
    // @interval: interval in micro seconds, 0 for only run once.
    // @return: TimerId
    TimerId addTimer(TimerCallback callback, TimeStamp expireTime, TimeType interval = 0);

    // @timerId: TimerId to cancel
    // can only be called in loop thread
    void cancelTimer(const TimerId &timerId);

private:
    void assertInLoopThread();

    void addTimerInLoop(std::unique_ptr<Timer> &&timer);
    void cancelTimerInLoop(const TimerId &timerId);
    std::vector<TimerEntry> getExpiredTimers(TimeStamp receivedTime);

    void handleRead(TimeStamp receivedTime);
    void timerfdRead();
    void timerfdReset(TimeStamp expireTime);

    EventLoop *loop_ = nullptr;
    UniqueFd timerfd_;
    std::shared_ptr<EventChannel> timerfdChannel_;

    TimerMap timers_;
    TimerEntrySet timerEntries_;
};
} // namespace base
} // namespace wind
