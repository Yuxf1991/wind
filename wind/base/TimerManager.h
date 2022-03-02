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

#ifndef WIND_TIMER_MANAGER_H
#define WIND_TIMER_MANAGER_H

#include "EventChannel.h"
#include "UniqueFd.h"

#include <set>

#include "Timer.h"

namespace wind {
namespace base {
using TimerEntry = std::pair<TimeStamp, std::shared_ptr<Timer>>;
using TimerSet = std::set<TimerEntry>;

class EventLoop;

class TimerManager : NonCopyable {
public:
    TimerManager(EventLoop *loop);
    ~TimerManager() noexcept;

    // @callback: TimerCallback
    // @expireTime: expire TimeStamp
    // @interval: interval in micro seconds, 0 for only run once.
    // @return: TimerId
    TimerId addTimer(TimerCallback callback, TimeStamp expireTime, TimeType interval = 0);

private:
    void assertInLoopThread();

    void addTimerInLoop(std::shared_ptr<Timer> timer);
    std::vector<TimerEntry> getExpiredTimers(TimeStamp receivedTime);

    void handleRead(TimeStamp receivedTime);
    void timerfdRead();
    void timerfdReset(TimeStamp expireTime);

    EventLoop *loop_;
    UniqueFd timerfd_;
    std::shared_ptr<EventChannel> timerfdChannel_;

    TimerSet timers_;
};
} // namespace base
} // namespace wind
#endif // WIND_TIMER_MANAGER_H
