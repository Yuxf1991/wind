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

#include "LogStream.h"

namespace wind {
namespace detail {
void defaultOuput(const char *buf, size_t len)
{
    (void)::fwrite(buf, sizeof(char), len, stdout);
}

void defaultFlush()
{
    (void)::fflush(stdout);
}
} // namespace detail

OutputFunc LogStream::outputFunc_ = &detail::defaultOuput;

FlushFunc LogStream::flushFunc_ = &detail::defaultFlush;

void LogStream::setOutputFunc(OutputFunc func)
{
    outputFunc_ = std::move(func);
}

void LogStream::setFlushFunc(FlushFunc func)
{
    flushFunc_ = std::move(func);
}

LogStream::~LogStream() noexcept
{
    output();
}

void LogStream::output()
{
    if (WIND_UNLIKELY(outputFunc_ == nullptr)) {
        return;
    }

    outputFunc_(buf_.data(), buf_.length());
    buf_.reset();
}

void LogStream::flush()
{
    // put the buf to dst stream before we flush it, it is necessary sometimes.
    output();

    if (WIND_UNLIKELY(flushFunc_ == nullptr)) {
        return;
    }

    flushFunc_();
}

void LogStream::append(const char *data, size_t len)
{
    if (WIND_LIKELY(len <= buf_.available())) {
        buf_.append(data, len);
    } else {
        size_t offset = 0;
        do {
            auto writeBytes = buf_.available();
            buf_.append(data + offset, writeBytes);
            offset += writeBytes;
            output();
        } while (buf_.available() < len - offset);

        if (len - offset > 0) {
            buf_.append(data + offset, len - offset);
        }
    }
}

LogStream &LogStream::operator<<(const char *s)
{
    append(s, strlen(s));
    return self();
}

LogStream &LogStream::operator<<(const string &s)
{
    append(s.data(), s.length());
    return self();
}

LogStream &LogStream::operator<<(const Fmt &fmt)
{
    append(fmt.data(), fmt.length());
    return self();
}
} // namespace wind
