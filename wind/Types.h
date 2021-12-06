//
// Created by tracy on 2021/11/10.
//

#ifndef WIND_TYPES_H
#define WIND_TYPES_H

#include <string>
#include <cstring>
#include <ctime>

namespace wind {
using TimeType = std::time_t;
using std::string;
using std::size_t;

inline void memZero(uint8_t *data, size_t len)
{
    ::memset(data, 0, len);
}

#define WIND_LIKELY(x) __builtin_expect(!!(x), 1)
#define WIND_UNLIKELY(x) __builtin_expect(!!(x), 0)
} // namespace wind
#endif // WIND_TYPES_H
