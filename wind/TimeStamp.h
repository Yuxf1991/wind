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

#ifndef WIND_TIMESTAMP_H
#define WIND_TIMESTAMP_H

#include "Types.h"

namespace wind {
constexpr int MILLI_SECS_PER_SECOND = 1000;
constexpr int MICRO_SECS_PER_MILLISECOND = 1000;
constexpr int MICRO_SECS_PER_SECOND = MICRO_SECS_PER_MILLISECOND * MILLI_SECS_PER_SECOND;
constexpr int NANO_SECS_PER_MICROSECOND = 1000;
constexpr int NANO_SECS_PER_MILLISECOND = NANO_SECS_PER_MICROSECOND * MICRO_SECS_PER_MILLISECOND;
constexpr int NANO_SECS_PER_SECOND = NANO_SECS_PER_MILLISECOND * MILLI_SECS_PER_SECOND;

enum class TimePrecision { SECOND, MILLI, MICRO };

// copyable
class TimeStamp {
public:
    TimeStamp() noexcept = default;
    ~TimeStamp() noexcept = default;

    constexpr explicit TimeStamp(TimeType microSecondsSinceEpoch) noexcept
        : microSecondsSinceEpoch_(microSecondsSinceEpoch)
    {}

    static TimeStamp now() noexcept;
    static TimeStamp invalid() noexcept { return TimeStamp(); }

    TimeType get() const noexcept { return microSecondsSinceEpoch_; }
    TimeType micros() const noexcept { return get(); }
    TimeType millis() const noexcept { return micros() / MICRO_SECS_PER_MILLISECOND; }
    TimeType seconds() const noexcept { return millis() / MILLI_SECS_PER_SECOND; }

    string toString(TimePrecision precision = TimePrecision::MILLI) const noexcept;
    string toFormattedString(TimePrecision precision = TimePrecision::MILLI) const noexcept;

private:
    TimeType microSecondsSinceEpoch_ = 0;
};

inline bool operator==(TimeStamp lhs, TimeStamp rhs)
{
    return lhs.get() == rhs.get();
}

inline bool operator<(TimeStamp lhs, TimeStamp rhs)
{
    return lhs.get() < rhs.get();
}

inline bool operator<=(TimeStamp lhs, TimeStamp rhs)
{
    return lhs.get() <= rhs.get();
}

inline bool operator>(TimeStamp lhs, TimeStamp rhs)
{
    return lhs.get() > rhs.get();
}

inline bool operator>=(TimeStamp lhs, TimeStamp rhs)
{
    return lhs.get() >= rhs.get();
}

inline bool operator!=(TimeStamp lhs, TimeStamp rhs)
{
    return lhs.get() != rhs.get();
}

// return two timeStamps' diff in micros
inline int64_t timeDiff(TimeStamp lhs, TimeStamp rhs)
{
    return lhs.get() - rhs.get();
}

// @micros: add micros seconds to timestamp t.
// @return: timestamp t + micro seconds
inline TimeStamp timeAdd(TimeStamp t, TimeType micros)
{
    return TimeStamp(t.micros() + micros);
}
} // namespace wind
#endif // WIND_TIMESTAMP_H
