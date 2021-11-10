//
// Created by tracy on 2020/9/21.
//

#ifndef WIND_TIMESTAMP_H
#define WIND_TIMESTAMP_H

#include <string>
#include <ctime>

namespace wind {
constexpr int MILLI_SECS_PER_SECOND = 1000;
constexpr int MICRO_SECS_PER_MILLISECOND = 1000;
constexpr int MICRO_SECS_PER_SECOND = MICRO_SECS_PER_MILLISECOND * MILLI_SECS_PER_SECOND;
constexpr int NANO_SECS_PER_MICROSECOND = 1000;
constexpr int NANO_SECS_PER_MILLISECOND = NANO_SECS_PER_MICROSECOND * MICRO_SECS_PER_MILLISECOND;
constexpr int NANO_SECS_PER_SECOND = NANO_SECS_PER_MILLISECOND * MILLI_SECS_PER_SECOND;

using TimeType = std::time_t;

enum class TimePrecision { SECOND, MILLI, MICRO, NANO };

// copyable
class TimeStamp {
public:
    TimeStamp() noexcept = default;
    ~TimeStamp() noexcept = default;

    constexpr explicit TimeStamp(TimeType nanoSecondsSinceEpoch) noexcept :
        nanoSecondsSinceEpoch_(nanoSecondsSinceEpoch)
    {}

    static TimeStamp now() noexcept;
    TimeType get() const noexcept
    {
        return nanoSecondsSinceEpoch_;
    }

    [[nodiscard]] std::string toString(TimePrecision precision = TimePrecision::MILLI) const noexcept;
    [[nodiscard]] std::string toFormattedString(TimePrecision precision = TimePrecision::MILLI) const noexcept;

private:
    TimeType nanoSecondsSinceEpoch_;
};
} // namespace wind

#endif // WIND_TIMESTAMP_H
