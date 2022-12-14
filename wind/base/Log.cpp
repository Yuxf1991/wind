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

#include "Log.h"

#include <unordered_map>

#include "CurrentThread.h"
#include "TimeStamp.h"

namespace wind {
namespace base {
namespace detail {
inline LogLevel defaultLogLevel()
{
#ifdef NDEBUG
    return LogLevel::INFO;
#else
    return LogLevel::DEBUG;
#endif
}

inline std::string logLevelToString(LogLevel level)
{
    switch (level) {
        case LogLevel::TRACE:
            return "TRACE";
        case LogLevel::DEBUG:
            return "DEBUG";
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::WARN:
            return "WARN";
        case LogLevel::ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

inline LogLevel getLogLevelFromEnv()
{
    static std::unordered_map<std::string, LogLevel> stringToLogLevelMap = {
        {"TRACE", LogLevel::TRACE},
        {"INFO", LogLevel::INFO},
        {"DEBUG", LogLevel::DEBUG},
        {"WARN", LogLevel::WARN},
        {"ERROR", LogLevel::ERROR}};
    char *env = ::getenv("WIND_LOG_LEVEL");
    if (env == NULL) {
        return defaultLogLevel();
    }

    if (stringToLogLevelMap.count(env) == 0) {
        int envVal = ::atoi(env);
        if (envVal < static_cast<int>(LogLevel::TRACE) || envVal > static_cast<int>(LogLevel::ERROR)) {
            return defaultLogLevel();
        }

        return static_cast<LogLevel>(envVal);
    }

    return stringToLogLevelMap[env];
}

LogLevel g_logLevel = getLogLevelFromEnv();
} // namespace detail

LogLevel Logger::currentLogLevel()
{
    return detail::g_logLevel;
}

void Logger::setLogLevel(LogLevel level)
{
    detail::g_logLevel = level;
}

Logger::Logger(SourceFileName fileName, int line, LogLevel level, std::string tag, bool isFatal)
    : fileName_(fileName), line_(line), isFatal_(isFatal)
{
    stream_ << TimeStamp::now().toFormattedString() << " " << CurrentThread::pidString() << " "
            << CurrentThread::tidString() << " " << tag << " " << detail::logLevelToString(level) << ": ";
}

Logger::~Logger() noexcept
{
#ifndef LOG_HIDE_FILE_LINE
    stream_ << " -- ";
    stream_.append(fileName_.data(), fileName_.length());
    stream_ << ":" << line_ << "\n";
#else
    UNUSED(line_);
    stream_ << "\n";
#endif // NDEBUG

    if (isFatal_) {
        LogStream::flush(stream_);
        abort();
    }
}
} // namespace base
} // namespace wind
