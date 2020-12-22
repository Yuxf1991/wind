//
// Created by tracy on 2020/9/21.
//

#include "TimeStamp.h"

#include <chrono>
#include <inttypes.h>

namespace wind {
    namespace detail {
        template <typename Duration>
        using SysTime = std::chrono::time_point<std::chrono::system_clock, Duration>;
        using SysNanoSeconds = SysTime<std::chrono::nanoseconds>;

        TimeInteger nanosecondsSinceEpoch()
        {
            SysNanoSeconds tmp = std::chrono::system_clock::now();
            TimeInteger t = tmp.time_since_epoch().count();
            return t;
        }
    }

    TimeStamp TimeStamp::now() {
        return TimeStamp(detail::nanosecondsSinceEpoch());
    }

    std::string TimeStamp::toString(TimePrecision precision) const {
        char buf[32] = {};
        TimeInteger seconds = m_nanoSecondsSinceEpoch / NANO_SECS_PER_SECOND;
        switch (precision) {
            case TimePrecision::SECOND: {
                snprintf(buf, sizeof(buf)-1, "%" PRId64 ".%03" PRId64 "", seconds, 0l);
                break;
            }
            case TimePrecision::MILLI: {
                TimeInteger millis = (m_nanoSecondsSinceEpoch % NANO_SECS_PER_SECOND)
                                 / NANO_SECS_PER_MILLISECOND;
                snprintf(buf, sizeof(buf)-1, "%" PRId64 ".%03" PRId64 "", seconds, millis);
                break;
            }
            case TimePrecision::MICRO: {
                TimeInteger micros = (m_nanoSecondsSinceEpoch % NANO_SECS_PER_SECOND)
                                 / NANO_SECS_PER_MICROSECOND;
                snprintf(buf, sizeof(buf)-1, "%" PRId64 ".%06" PRId64 "", seconds, micros);
                break;
            }
            case TimePrecision::NANO: {
                TimeInteger nanos = (m_nanoSecondsSinceEpoch % NANO_SECS_PER_SECOND);
                snprintf(buf, sizeof(buf)-1, "%" PRId64 ".%09" PRId64 "", seconds, nanos);
                break;
            }
            default:
                break;
        }
        return buf;
    }

    std::string TimeStamp::toFormattedString(TimePrecision precision) const {
        char buf[64] = {};
        TimeInteger seconds = m_nanoSecondsSinceEpoch / NANO_SECS_PER_SECOND;
        struct tm *tm_time = ::gmtime(&seconds);
        switch (precision) {
            case TimePrecision::SECOND: {
                snprintf(buf, sizeof(buf), "%4d-%02d-%02d %02d:%02d:%02d.%03d",
                         tm_time->tm_year + 1900, tm_time->tm_mon + 1, tm_time->tm_mday,
                         tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec, 0);
                break;
            }
            case TimePrecision::MILLI: {
                int millis = static_cast<int>(m_nanoSecondsSinceEpoch % NANO_SECS_PER_SECOND)
                                 / NANO_SECS_PER_MILLISECOND;
                snprintf(buf, sizeof(buf), "%4d-%02d-%02d %02d:%02d:%02d.%03d",
                         tm_time->tm_year + 1900, tm_time->tm_mon + 1, tm_time->tm_mday,
                         tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec, millis);
                break;
            }
            case TimePrecision::MICRO: {
                int micros = static_cast<int>(m_nanoSecondsSinceEpoch % NANO_SECS_PER_SECOND)
                                 / NANO_SECS_PER_MICROSECOND;
                snprintf(buf, sizeof(buf), "%4d-%02d-%02d %02d:%02d:%02d.%06d",
                         tm_time->tm_year + 1900, tm_time->tm_mon + 1, tm_time->tm_mday,
                         tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec, micros);
                break;
            }
            case TimePrecision::NANO: {
                TimeInteger nanos = (m_nanoSecondsSinceEpoch % NANO_SECS_PER_SECOND);
                snprintf(buf, sizeof(buf), "%4d-%02d-%02d %02d:%02d:%02d.%09ld",
                         tm_time->tm_year + 1900, tm_time->tm_mon + 1, tm_time->tm_mday,
                         tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec, nanos);
                break;
            }
            default:
                break;
        }
        return buf;
    }

} // namespace wind