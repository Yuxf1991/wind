// MIT License

// Copyright (c) 2020 Tracy

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "TimeStamp.h"

#include <chrono>
#include <cinttypes>

namespace wind {
namespace detail {
template <typename Duration>
using SysTime = std::chrono::time_point<std::chrono::system_clock, Duration>;
using SysNanoSeconds = SysTime<std::chrono::nanoseconds>;

TimeType nanosecondsSinceEpoch()
{
    SysNanoSeconds tmp = std::chrono::system_clock::now();
    TimeType t = tmp.time_since_epoch().count();
    return t;
}
} // namespace detail

TimeStamp TimeStamp::now() noexcept
{
    return TimeStamp(detail::nanosecondsSinceEpoch());
}

std::string TimeStamp::toString(TimePrecision precision) const noexcept
{
    char buf[32] = {};
    TimeType seconds = nanoSecondsSinceEpoch_ / NANO_SECS_PER_SECOND;
    switch (precision) {
    case TimePrecision::SECOND: {
        snprintf(
            buf, sizeof(buf) - 1, "%" PRId64 ".%03" PRId64 "", seconds, 0l);
        break;
    }
    case TimePrecision::MILLI: {
        TimeType millis = (nanoSecondsSinceEpoch_ % NANO_SECS_PER_SECOND)
                          / NANO_SECS_PER_MILLISECOND;
        snprintf(
            buf, sizeof(buf) - 1, "%" PRId64 ".%03" PRId64 "", seconds, millis);
        break;
    }
    case TimePrecision::MICRO: {
        TimeType micros = (nanoSecondsSinceEpoch_ % NANO_SECS_PER_SECOND)
                          / NANO_SECS_PER_MICROSECOND;
        snprintf(
            buf, sizeof(buf) - 1, "%" PRId64 ".%06" PRId64 "", seconds, micros);
        break;
    }
    case TimePrecision::NANO: {
        TimeType nanos = (nanoSecondsSinceEpoch_ % NANO_SECS_PER_SECOND);
        snprintf(
            buf, sizeof(buf) - 1, "%" PRId64 ".%09" PRId64 "", seconds, nanos);
        break;
    }
    default: break;
    }
    return buf;
}

std::string TimeStamp::toFormattedString(TimePrecision precision) const noexcept
{
    char buf[64] = {};
    TimeType seconds = nanoSecondsSinceEpoch_ / NANO_SECS_PER_SECOND;
    struct tm *tm_time = std::localtime(&seconds);
    switch (precision) {
    case TimePrecision::SECOND: {
        snprintf(buf,
                 sizeof(buf),
                 "%4d-%02d-%02d %02d:%02d:%02d.%03d",
                 tm_time->tm_year + 1900,
                 tm_time->tm_mon + 1,
                 tm_time->tm_mday,
                 tm_time->tm_hour,
                 tm_time->tm_min,
                 tm_time->tm_sec,
                 0);
        break;
    }
    case TimePrecision::MILLI: {
        int millis =
            static_cast<int>(nanoSecondsSinceEpoch_ % NANO_SECS_PER_SECOND)
            / NANO_SECS_PER_MILLISECOND;
        snprintf(buf,
                 sizeof(buf),
                 "%4d-%02d-%02d %02d:%02d:%02d.%03d",
                 tm_time->tm_year + 1900,
                 tm_time->tm_mon + 1,
                 tm_time->tm_mday,
                 tm_time->tm_hour,
                 tm_time->tm_min,
                 tm_time->tm_sec,
                 millis);
        break;
    }
    case TimePrecision::MICRO: {
        int micros =
            static_cast<int>(nanoSecondsSinceEpoch_ % NANO_SECS_PER_SECOND)
            / NANO_SECS_PER_MICROSECOND;
        snprintf(buf,
                 sizeof(buf),
                 "%4d-%02d-%02d %02d:%02d:%02d.%06d",
                 tm_time->tm_year + 1900,
                 tm_time->tm_mon + 1,
                 tm_time->tm_mday,
                 tm_time->tm_hour,
                 tm_time->tm_min,
                 tm_time->tm_sec,
                 micros);
        break;
    }
    case TimePrecision::NANO: {
        TimeType nanos = (nanoSecondsSinceEpoch_ % NANO_SECS_PER_SECOND);
        snprintf(buf,
                 sizeof(buf),
                 "%4d-%02d-%02d %02d:%02d:%02d.%09ld",
                 tm_time->tm_year + 1900,
                 tm_time->tm_mon + 1,
                 tm_time->tm_mday,
                 tm_time->tm_hour,
                 tm_time->tm_min,
                 tm_time->tm_sec,
                 nanos);
        break;
    }
    default: break;
    }
    return buf;
}
} // namespace wind
