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

#include <atomic>
#include <condition_variable>
#include <filesystem>
#include <memory>

#include "LogFile.h"
#include "Thread.h"

namespace wind {
namespace base {
// Async logging, will be running at background thread.
class LogDaemon : NonCopyable {
    using LogBuffer = FixedSizeBuffer<LARGE_BUFFER_SIZE>;
    using LogBufferPtr = std::unique_ptr<LogBuffer>;

public:
    // @baseName: logFile's baseName.
    // @logDir: current dir by default.
    // @rollSize: size for per logFile, 100MB by default.
    // @flushInterval: flush every interval seconds, 5s by default.
    explicit LogDaemon(
        string baseName,
        string logDir = "./",
        FileSize rollSize = 100 * 1024 * 1024,
        uint32_t flushInterval = 5);
    ~LogDaemon() noexcept;

    void start();

private:
    string generateLogFileName();
    void append(const char *data, size_t len);
    void flush();

    void threadMain();

    std::atomic<bool> running_ = false;
    Thread thread_;
    mutable std::mutex mutex_;
    std::condition_variable cond_;

    string baseName_;
    std::filesystem::path logDir_;
    std::unique_ptr<LogFile> file_;

    FileSize rollSize_ = 100 * 1024 * 1024; // 100MB for per log file.
    uint32_t flushInterval_ = 5;            // flush every 5 seconds by default.

    LogBufferPtr frontBuffer_;                 // Frontend threads will write logs to this buffer.
    LogBufferPtr reservedBuffer_;              // Backstore of the front buffer
    std::vector<LogBufferPtr> stagingBuffers_; // Backend thread will swap these buffers and write them to a file.
};
} // namespace base
} // namespace wind
