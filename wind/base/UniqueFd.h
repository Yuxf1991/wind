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

#include "Log.h"

namespace wind {
namespace base {
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
        LOG_DEBUG << "DefaultFdCloser::operator(int): closing... fd " << fd;
        TEMP_FAILURE_RETRY(::close(fd));
    }
};

template <typename Closer>
class UniqueFdImpl : NonCopyable {
public:
    UniqueFdImpl() = default;
    explicit UniqueFdImpl(int fd) noexcept : fd_(fd) {}
    ~UniqueFdImpl() noexcept { reset(); }

    // movable
    UniqueFdImpl(UniqueFdImpl &&other) noexcept : fd_(other.release()) {}
    UniqueFdImpl &operator=(UniqueFdImpl &&other) noexcept
    {
        reset(other.release()); // can avoid self-move.
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

using UniqueFd = UniqueFdImpl<DefaultFdCloser>;
} // namespace base
} // namespace wind
#endif // WIND_UNIQUEFD_H
