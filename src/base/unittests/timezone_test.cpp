#include "base/TimeZone.h"

#include <iostream>

template <typename Duration>
using sys_time = std::chrono::time_point<std::chrono::system_clock, Duration>;
using sys_nanoseconds = sys_time<std::chrono::nanoseconds>;

std::time_t getTimeStamp()
{
    sys_nanoseconds t = std::chrono::system_clock::now();
    time_t timestamp = t.time_since_epoch().count();
    return timestamp;
}


std::tm* gettm()
{
    std::time_t t = getTimeStamp() / 1000 / 1000 / 1000;
    std::tm* now = std::gmtime(&t);
    printf("%4d年%02d月%02d日 %02d:%02d:%02d %s\n",
           now->tm_year+1900,
           now->tm_mon+1,
           now->tm_mday,
           now->tm_hour,
           now->tm_min,
           now->tm_sec,
           now->tm_zone);
    return now;
}


int main() {
    std::tm *t = gettm();
}