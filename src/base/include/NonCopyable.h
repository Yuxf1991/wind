//
// Created by tracy on 2020/9/11.
//

#ifndef WIND_NONCOPYABLE_H
#define WIND_NONCOPYABLE_H

namespace wind {
class NonCopyable {
public:
    NonCopyable(const NonCopyable &other) = delete;
    NonCopyable& operator=(const NonCopyable &other) = delete;

protected:
    NonCopyable() = default;
    ~NonCopyable() = default;
};
} // namespace wind;

#endif //WIND_NONCOPYABLE_H
