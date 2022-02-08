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

#include "Timer.h"

#include <sys/timerfd.h>

#include "EventLoop.h"
#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "WindTimer"
#endif // LOG_TAG
#include "Log.h"

namespace wind {
namespace detail {
itimerspec generateTimerSpec(TimeType delay, TimeType interval)
{
    itimerspec newValue;
    if (delay == 0) {
        // set it_value to 1000 nanoseconds(1 us) to make sure it can be triggerd.
        newValue.it_value.tv_nsec = 1000;
    } else {
        int seconds = delay / MICRO_SECS_PER_SECOND;
        newValue.it_value.tv_sec = seconds;
        newValue.it_value.tv_nsec = (delay % MICRO_SECS_PER_SECOND) * NANO_SECS_PER_MICROSECOND;
    }

    int intervalSecs = interval / MICRO_SECS_PER_SECOND;
    newValue.it_interval.tv_sec = intervalSecs;
    newValue.it_interval.tv_nsec = (interval % MICRO_SECS_PER_SECOND) * NANO_SECS_PER_MICROSECOND;

    return newValue;
}
} // namespace detail

Timer::Timer(EventLoop *eventLoop, TimeType delay, TimeType interval)
    : EventChannel(::timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC), eventLoop), delay_(delay), interval_(interval)
{
    itimerspec newValue = detail::generateTimerSpec(delay_, interval_);
    int ret = TEMP_FAILURE_RETRY(::timerfd_settime(fd_.get(), 0, &newValue, NULL));
    LOG_FATAL_IF(ret == -1) << "timerfd_settime failed: " << strerror(errno) << ".";
}

void Timer::handleEvent(TimeStamp receivedTime)
{
    EventChannel::handleEvent(receivedTime);
    if (interval_ == 0) {
        LOG_DEBUG << "Remove timer fd " << fd_.get() << " from poller.";
        eventLoop_->removeChannel(fd_.get());
    }
}
} // namespace wind
