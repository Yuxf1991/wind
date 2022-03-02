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

#ifndef WIND_TIMER_H
#define WIND_TIMER_H

#include <functional>

#include "NonCopyable.h"
#include "TimeStamp.h"

namespace wind {
namespace base {
using TimerCallback = std::function<void()>;
using TimerId = uint64_t;

class Timer : NonCopyable {
public:
    // @callback: TimerCallback
    // @expireTime: expire TimeStamp
    // @interval: interval in micro seconds, 0 for only run once.
    Timer(TimerCallback callback, TimeStamp expireTime, TimeType interval = 0);
    ~Timer() noexcept = default;

    TimerId id() const { return id_; }
    bool isRepeat() const { return repeat_; }

    TimeStamp expireTime() const { return expireTime_; }

    void execute();

private:
    // only valid when the timer is repeated.
    void restart();

    TimerCallback cb_;
    TimeStamp expireTime_;
    TimeType interval_ = 0;
    bool repeat_ = false;
    TimerId id_;
};
} // namespace base
} // namespace wind
#endif // WIND_TIMER_H
