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
namespace base {
namespace detail {
void defaultOutput(const char *buf, size_t len)
{
    (void)::fwrite(buf, sizeof(char), len, stdout);
}

void defaultFlush()
{
    (void)::fflush(stdout);
}

constexpr size_t MAX_NUMERIC_SIZE = 32;

inline char digitToChar(int idx)
{
    static char digits[20] = "9876543210123456789";
    static int zeroIdx = 9;
    static_assert(sizeof(digits) == 20, "digits number err!");
    ASSERT((idx > -10) && (idx < 10));
    return digits[zeroIdx + idx];
}

inline char hexDigitToChar(int idx, bool upper = false)
{
    static char hexDigits[17] = "0123456789abcdef";
    static char hexDigitsUpper[17] = "0123456789ABCDEF";
    static_assert(sizeof(hexDigits) == 17, "hexDigits number err!");
    static_assert(sizeof(hexDigitsUpper) == 17, "hexDigitsUpper number err!");
    ASSERT((idx >= 0) && (idx < 17));
    return upper ? hexDigitsUpper[idx] : hexDigits[idx];
}

// ensure the out buffer's capacity is bigger than MAX_NUMERIC_SIZE.
// return the real buf size after converting.
template <typename T>
size_t convertDigitToString(char *outBuf, T inValue)
{
    static_assert(std::is_integral<T>::value, "input type is not an integral type!");
    char *start = outBuf;
    char *end = outBuf;
    auto tmp = inValue;
    do {
        int remainder = tmp % 10;
        tmp /= 10;
        *end++ = digitToChar(remainder);
    } while (tmp != 0);
    if (inValue < 0) {
        *end++ = '-';
    }
    *end = '\0';

    std::reverse(start, end);
    return end - start;
}

size_t convertPointerToString(char *outBuf, uintptr_t p)
{
    char *start = outBuf;
    char *end = outBuf;
    uintptr_t tmp = p;
    do {
        int remainder = static_cast<int>(tmp % 16);
        tmp /= 16;
        *end++ = hexDigitToChar(remainder);
    } while (tmp != 0);

    *end = '\0';
    std::reverse(start, end);
    return end - start;
}
} // namespace detail

OutputFunc LogStream::outputFunc_ = &detail::defaultOutput;
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

void LogStream::output(LogStream &stream)
{
    stream.output();
}

void LogStream::output()
{
    if (WIND_UNLIKELY(outputFunc_ == nullptr)) {
        return;
    }

    outputFunc_(buf_.data(), buf_.length());
    buf_.reset();
}

void LogStream::flush(LogStream &stream)
{
    stream.flush();
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

LogStream &LogStream::operator<<(LogStream &(*func)(LogStream &))
{
    return func(self());
}

void LogStream::preProcessWithNumericInput()
{
    ASSERT(buf_.capacity() > detail::MAX_NUMERIC_SIZE);
    if (buf_.available() < detail::MAX_NUMERIC_SIZE) {
        output();
    }
}

LogStream &LogStream::operator<<(int64_t val)
{
    preProcessWithNumericInput();
    auto len = detail::convertDigitToString(buf_.curr(), val);
    buf_.grow(len);
    return self();
}

LogStream &LogStream::operator<<(uint64_t val)
{
    preProcessWithNumericInput();
    auto len = detail::convertDigitToString(buf_.curr(), val);
    buf_.grow(len);
    return self();
}

LogStream &LogStream::operator<<(const void *pointer)
{
    append("0x", 2);
    preProcessWithNumericInput();
    auto len = detail::convertPointerToString(buf_.curr(), reinterpret_cast<uintptr_t>(pointer));
    buf_.grow(len);
    return self();
}

LogStream &LogStream::operator<<(double val)
{
    preProcessWithNumericInput();
    size_t len = snprintf(buf_.curr(), buf_.available(), "%.12g", val);
    buf_.grow(len);
    return self();
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
} // namespace base
} // namespace wind
