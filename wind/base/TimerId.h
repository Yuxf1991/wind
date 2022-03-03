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

#ifndef WIND_TIMER_ID_H
#define WIND_TIMER_ID_H

#include "Types.h"

namespace wind {
namespace base {
class Timer;
struct TimerId {
    TimerId(uint64_t id, Timer *timer) : id(id), timer(timer) {}
    uint64_t id = 0;
    Timer *timer = nullptr;
    bool operator==(const TimerId &other) const { return id == other.id && timer == other.timer; }
    bool operator<(const TimerId &other) const { return id < other.id || timer < other.timer; }
};
} // namespace base
} // namespace wind

namespace std {
template <>
struct hash<typename wind::base::TimerId> {
    size_t operator()(wind::base::TimerId timerId) const
    {
        return static_cast<size_t>(timerId.id ^ reinterpret_cast<uintptr_t>(timerId.timer));
    }
};
} // namespace std
#endif // WIND_TIMER_ID_H
