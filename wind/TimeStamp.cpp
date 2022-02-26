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

#include "TimeStamp.h"

#include <chrono>
#include <iomanip>

namespace wind {
namespace detail {
TimeType microSecondsSinceEpoch()
{
    auto tp = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now());
    return tp.time_since_epoch().count();
}
} // namespace detail

TimeStamp TimeStamp::now() noexcept
{
    return TimeStamp(detail::microSecondsSinceEpoch());
}

string TimeStamp::toString(TimePrecision precision) const noexcept
{
    TimeType seconds = microSecondsSinceEpoch_ / MICRO_SECS_PER_SECOND;
    TimeType micros = microSecondsSinceEpoch_ % MICRO_SECS_PER_SECOND;

    std::stringstream ss;
    ss << seconds << "." << std::setfill('0');
    switch (precision) {
        case TimePrecision::SECOND: {
            ss << std::setw(3) << 0;
            break;
        }
        case TimePrecision::MILLI: {
            TimeType millis = micros / MICRO_SECS_PER_MILLISECOND;
            ss << std::setw(3) << millis;
            break;
        }
        case TimePrecision::MICRO: {
            ss << std::setw(6) << micros;
            break;
        }
        default: break;
    }

    return ss.str();
}

string TimeStamp::toFormattedString(TimePrecision precision) const noexcept
{
    TimeType seconds = microSecondsSinceEpoch_ / MICRO_SECS_PER_SECOND;
    TimeType micros = microSecondsSinceEpoch_ % MICRO_SECS_PER_SECOND;

    std::stringstream ss;
    ss << std::put_time(::localtime(&seconds), "%F %X") << "." << std::setfill('0');
    switch (precision) {
        case TimePrecision::SECOND: {
            ss << std::setw(3) << 0;
            break;
        }
        case TimePrecision::MILLI: {
            TimeType millis = micros / MICRO_SECS_PER_MILLISECOND;
            ss << std::setw(3) << millis;
            break;
        }
        case TimePrecision::MICRO: {
            ss << std::setw(6) << micros;
            break;
        }
        default: break;
    }

    return ss.str();
}
} // namespace wind
