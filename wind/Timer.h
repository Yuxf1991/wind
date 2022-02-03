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

#include "EventChannel.h"

namespace wind {
class Timer : public EventChannel {
public:
    // delay in micro seconds, 0 means run immediately
    // interval in micro seconds, 0 means only run once.
    Timer(EventLoop *eventLoop, TimeType delay = 0, TimeType interval = 0);
    ~Timer() noexcept override = default;

    void handleEvent(TimeStamp receivedTime) override;

private:
    TimeType delay_ = 0;
    TimeType interval_ = -1;
};
} // namespace wind
#endif // WIND_TIMER_H
