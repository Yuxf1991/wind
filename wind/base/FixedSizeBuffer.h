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

#pragma once

#include "NonCopyable.h"
#include "Types.h"
#include "Utils.h"

namespace wind {
namespace base {
constexpr size_t DEFAULT_BUFFER_SIZE = 4096;
constexpr size_t LARGE_BUFFER_SIZE = DEFAULT_BUFFER_SIZE * 1024;

template <size_t BUF_SIZE>
class FixedSizeBuffer : NonCopyable {
public:
    FixedSizeBuffer() = default;
    ~FixedSizeBuffer() noexcept = default;

    const char *data() const
    {
        return data_;
    }

    // expose curr_ to modify outside.
    char *curr() const
    {
        return curr_;
    }
    void grow(size_t len)
    {
        ASSERT(len < available());
        curr_ += len;
    }

    const char *end() const
    {
        return data_ + sizeof(data_);
    }
    size_t available() const
    {
        return static_cast<size_t>(end() - curr());
    }
    size_t length() const
    {
        return static_cast<size_t>(curr() - data());
    }
    constexpr size_t capacity() const
    {
        return sizeof(data_);
    }

    void reset()
    {
        curr_ = data_;
    }

    // Set all bits to zero.
    void clean()
    {
        utils::memZero(data_, sizeof(data_));
        reset();
    }

    void append(const char *s, size_t len)
    {
        if (WIND_UNLIKELY(len == 0) || WIND_UNLIKELY(s == nullptr)) {
            return;
        }
        size_t bytesToWrite = (available() > len ? len : available());
        ::memcpy(curr_, s, bytesToWrite);
        curr_ += bytesToWrite;
    }
    void append(const char *s)
    {
        if (WIND_UNLIKELY(s == nullptr)) {
            return;
        }
        append(s, ::strlen(s));
    }
    void append(const string &s)
    {
        append(s.c_str(), s.size());
    }
    string toString() const
    {
        return string(data_, length());
    }

private:
    char data_[BUF_SIZE] = {0}; // Maybe we do not need to initialize this buffer?
    char *curr_ = data_;
};

typedef FixedSizeBuffer<DEFAULT_BUFFER_SIZE> DefaultFixedBuffer;
} // namespace base
} // namespace wind
