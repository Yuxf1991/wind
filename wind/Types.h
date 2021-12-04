//
// Created by tracy on 2021/11/10.
//

#ifndef WIND_TYPES_H
#define WIND_TYPES_H

#include <string>
#include <ctime>

namespace wind {
using TimeType = std::time_t;
using std::string;

#define WIND_LIKELY(x) __builtin_expect(!!(x), 1)
#define WIND_UNLIKELY(x) __builtin_expect(!!(x), 0)
} // namespace wind
#endif //WIND_TYPES_H
