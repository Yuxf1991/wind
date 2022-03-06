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

#include "LogStream.h"

// If you want to define your custom log tag,
// plese define it before including Log.h, here is a sample:
// -------------------------------------------
//      #define LOG_TAG "MyLogTag"
//      #include "Log.h"
// -------------------------------------------
#ifndef LOG_TAG
#define LOG_TAG "Wind"
#endif // LOG_TAG

namespace wind {
namespace base {
enum class LogLevel {
    TRACE = 1,
    DEBUG,
    INFO,
    WARN,
    ERROR,
};

class SourceFileName {
public:
    // not explicit cause we use its implicit constructor.
    SourceFileName(const char *s)
    {
        size_t len = ::strlen(s);
        append(s, len);
    }
    // not explicit, too.
    template <size_t len>
    SourceFileName(const char (&s)[len])
    {
        append(s, len);
    }
    ~SourceFileName() noexcept = default;

    const char *data() const
    {
        return buf_;
    }

    size_t length() const
    {
        return len_;
    }

private:
    void append(const char *data, size_t len)
    {
        const char *p = ::strrchr(data, '/') + 1;
        if (WIND_UNLIKELY(p == NULL)) {
            return;
        }

        size_t bytesToWrite = len - (p - data);
        ASSERT(sizeof(buf_) > bytesToWrite);
        ::memcpy(buf_, p, bytesToWrite);
        len_ = bytesToWrite;
    }

    char buf_[256] = {0};
    size_t len_ = 0;
};

class Logger : NonCopyable {
public:
    // Read log level from env: WIND_LOG_LEVEL
    // "TRACE" or 1 --> LogLevel::TRACE
    // "DEBUG" or 2 --> LogLevel::DEBUG
    // "INFO" or 3 --> LogLevel::INFO
    // "WARN" or 4 --> LogLevel::WARN
    // "ERROR" or 5 --> LogLevel::ERROR
    // else --> defaultLogLevel: DEBUG in Debug mode, and INFO in release mode.
    static LogLevel currentLogLevel();
    static void setLogLevel(LogLevel level);

    Logger(SourceFileName fileName, int line, LogLevel level, std::string tag = LOG_TAG, bool isFatal = false);
    ~Logger() noexcept;
    LogStream &stream()
    {
        return stream_;
    }

private:
    LogStream stream_;
    SourceFileName fileName_;
    int line_;
    bool isFatal_;
};
} // namespace base
} // namespace wind

#define LOG_TRACE                                                                                                      \
    if (wind::base::Logger::currentLogLevel() <= wind::base::LogLevel::TRACE)                                          \
    wind::base::Logger(__FILE__, __LINE__, wind::base::LogLevel::TRACE, LOG_TAG).stream()
#define LOG_DEBUG                                                                                                      \
    if (wind::base::Logger::currentLogLevel() <= wind::base::LogLevel::DEBUG)                                          \
    wind::base::Logger(__FILE__, __LINE__, wind::base::LogLevel::DEBUG, LOG_TAG).stream()
#define LOG_INFO                                                                                                       \
    if (wind::base::Logger::currentLogLevel() <= wind::base::LogLevel::INFO)                                           \
    wind::base::Logger(__FILE__, __LINE__, wind::base::LogLevel::INFO, LOG_TAG).stream()
#define LOG_WARN                                                                                                       \
    if (wind::base::Logger::currentLogLevel() <= wind::base::LogLevel::WARN)                                           \
    wind::base::Logger(__FILE__, __LINE__, wind::base::LogLevel::WARN, LOG_TAG).stream()
#define LOG_ERROR wind::base::Logger(__FILE__, __LINE__, wind::base::LogLevel::ERROR, LOG_TAG).stream()
#define LOG_SYS_FATAL wind::base::Logger(__FILE__, __LINE__, wind::base::LogLevel::ERROR, LOG_TAG, true).stream()
#define LOG_FATAL_IF(expr)                                                                                             \
    if ((expr))                                                                                                        \
    wind::base::Logger(__FILE__, __LINE__, wind::base::LogLevel::ERROR, LOG_TAG, true).stream()
