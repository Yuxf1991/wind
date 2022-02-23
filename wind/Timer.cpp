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

#include <atomic>

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "WindTimer"
#endif // LOG_TAG
#include "Log.h"

namespace wind {
namespace detail {
uint64_t genSequenceId()
{
    static std::atomic<uint64_t> id(0);
    return id.fetch_add(1, std::memory_order_relaxed);
}
} // namespace detail

Timer::Timer(TimerCallback callback, TimeStamp expireTime, TimeType interval)
    : cb_(callback), expireTime_(expireTime), interval_(interval), repeat_(interval > 0), id_(detail::genSequenceId())
{}

void Timer::execute()
{
    if (cb_ != nullptr) {
        cb_();
    }

    if (isRepeat()) {
        restart();
    }
}

void Timer::restart()
{
    ASSERT(isRepeat());
    expireTime_ = timeAdd(TimeStamp::now(), interval_);
}
} // namespace wind
