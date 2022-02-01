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

#ifndef WIND_LOGSTREAM_H
#define WIND_LOGSTREAM_H

#include <functional>

#include "Buffer.h"

namespace wind {
using OutputFunc = std::function<void(const char *buf, size_t len)>;
using FlushFunc = std::function<void()>;

class Fmt {
public:
    template <typename... Args>
    Fmt(const char *fmt, Args... args)
    {
        int len = ::snprintf(buf_.curr(), buf_.available(), fmt, args...);
        ASSERT(static_cast<size_t>(len) <= buf_.available());
        buf_.grow(len);
    }

    ~Fmt() noexcept = default;

    const char *data() const { return buf_.data(); }

    size_t length() const { return buf_.length(); }

private:
    DefaultFixedBuffer buf_;
};

class LogStream : NonCopyable {
public:
    LogStream() = default;
    ~LogStream() noexcept;

    LogStream &self() { return *this; }
    LogStream &operator<<(const string &s);
    LogStream &operator<<(const char *s);
    LogStream &operator<<(const Fmt &fmt);

    static void setOutputFunc(OutputFunc func);
    static void setFlushFunc(FlushFunc func);

private:
    void append(const char *data, size_t len);
    void output();
    void flush();
    DefaultFixedBuffer buf_;
    // FixedSizeBuffer<8> buf_; // use a small size of buffer to test LogStream::append()

    static OutputFunc outputFunc_;
    static FlushFunc flushFunc_;
};
} // namespace wind
#endif // WIND_LOGSTREAM_H
