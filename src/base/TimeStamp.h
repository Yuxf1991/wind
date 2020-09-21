//
// Created by tracy on 2020/9/21.
//

#ifndef WIND_TIMESTAMP_H
#define WIND_TIMESTAMP_H

#include <string>
#include <time.h>

namespace wind {
    constexpr int NANO_SECS_PER_SECOND = 1e9;
    constexpr int NANO_SECS_PER_MILLISECOND = 1e6;
    constexpr int NANO_SECS_PER_MICROSECOND = 1e3;
    constexpr int MICRO_SECS_PER_SECOND = 1e6;
    constexpr int MICRO_SECS_PER_MILLISECOND = 1e3;
    constexpr int MILLI_SECS_PER_SECOND = 1e3;

    using wtime_t = ::time_t;

    enum class TimePrecision {
        SECOND,
        MILLI,
        MICRO,
        NANO
    };

    // copyable
    class TimeStamp {
    public:
        TimeStamp() = default;
        ~TimeStamp() = default;

        explicit TimeStamp(wtime_t nanoSecondsSinceEpoch)
            : m_nanoSecondsSinceEpoch(nanoSecondsSinceEpoch)
        {
        }

        static TimeStamp now();

        [[nodiscard]] std::string toString(TimePrecision precision = TimePrecision::MILLI) const;
        [[nodiscard]] std::string toFormattedString(TimePrecision precision = TimePrecision::MILLI) const;


    private:
        wtime_t m_nanoSecondsSinceEpoch;
    };
} // namespace wind;

#endif //WIND_TIMESTAMP_H
