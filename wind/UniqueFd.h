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
