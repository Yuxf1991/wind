//
// Created by tracy on 2021/12/4.
//

#ifndef WIND_UNIQUEFD_H
#define WIND_UNIQUEFD_H

#include <unistd.h>

#include "NonCopyable.h"

// TODO: use our own logging
#include <iostream>

namespace wind {
inline constexpr bool INVALID_FD(int fd)
{
    return fd < 0;
}

inline constexpr int INVALID_FD()
{
    return -1;
}

struct DefaultFdCloser {
    void operator()(int fd)
    {
#ifndef NDEBUG
        // TODO: use our own logging
        std::cout << "DefaultFdCloser::operator(int): closing... fd " << fd << std::endl;
#endif
        TEMP_FAILURE_RETRY(::close(fd));
    }
};

template <typename Closer = DefaultFdCloser>
class UniqueFd : NonCopyable {
public:
    UniqueFd() = default;
    explicit UniqueFd(int fd) noexcept : fd_(fd) {}
    ~UniqueFd() noexcept { reset(); }

    // movable
    UniqueFd(UniqueFd &&other) noexcept : fd_(other.release()) {}
    UniqueFd &operator=(UniqueFd &&other) noexcept
    {
        reset(other.release());
        return *this;
    }

    void reset(int fd = INVALID_FD()) noexcept
    {
        close();
        fd_ = fd;
    }

    int release()
    {
        int fd = fd_;
        fd_ = INVALID_FD();
        return fd;
    }

    [[nodiscard]] int get() const noexcept { return fd_; }

    explicit operator int() const { return fd_; }

private:
    void close()
    {
        static Closer closer;
        if (!INVALID_FD(fd_)) {
            closer(fd_);
        }
    }

    int fd_ = INVALID_FD();
};
} // namespace wind
#endif // WIND_UNIQUEFD_H
