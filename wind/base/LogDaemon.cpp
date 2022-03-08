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

#include "LogDaemon.h"

#include "TimeStamp.h"

namespace wind {
namespace base {
namespace fs = std::filesystem;

LogDaemon::LogDaemon(string baseName, string logDir, FileSize rollSize, int flushInterval)
    : baseName_(std::move(baseName)), logDir_(std::move(logDir)), rollSize_(rollSize), flushInterval_(flushInterval),
      frontBuffer_(std::make_unique<LogBuffer>()), reservedBuffer_(std::make_unique<LogBuffer>())
{
    frontBuffer_->clear();
    reservedBuffer_->clear();
    if (!fs::is_directory(logDir_)) {
        logDir_ = logDir_.parent_path();
    }

    LogStream::setOutputFunc([this](const char *data, size_t len) {
        append(data, len);
    });
    LogStream::setFlushFunc([this]() {
        flush();
    });
}

LogDaemon::~LogDaemon() noexcept
{
    running_ = false;
    cond_.notify_one();
    if (thread_.joinable()) {
        thread_.join();
    }
}

string LogDaemon::generateLogFileName()
{
    TimeStamp now = TimeStamp::now();
    string fileName = logDir_.append(baseName_).string() + now.toString();
    return fileName;
}

void LogDaemon::start()
{
    if (running_) {
        return;
    }

    thread_ = make_thread("WindLogDaemon", [this]() {
        threadMain();
    });
}

void LogDaemon::threadMain()
{
    std::vector<LogBufferPtr> buffersToWrite;
    LogBufferPtr tmpBuffer = std::make_unique<LogBuffer>();
    tmpBuffer->clear();
    while (running_) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (stagingBuffers_.empty()) {
                cond_.wait_for(lock, std::chrono::seconds(flushInterval_));
            }
            if (stagingBuffers_.empty()) {
                stagingBuffers_.push_back(std::move(frontBuffer_));
                frontBuffer_ = std::move(tmpBuffer);
            }

            buffersToWrite.swap(stagingBuffers_);
            ASSERT(!buffersToWrite.empty());
        }

    }
}

void LogDaemon::append(const char *data, size_t len)
{
    std::lock_guard<std::mutex> lock(mutex_);
    appendLocked(data, len);
}

void LogDaemon::appendLocked(const char *data, size_t len)
{
    if (frontBuffer_->available() < len) {
        stagingBuffers_.push_back(std::move(frontBuffer_));
        if (reservedBuffer_ != nullptr) {
            frontBuffer_ = std::move(reservedBuffer_);
        } else {
            frontBuffer_ = std::make_unique<LogBuffer>();
            frontBuffer_->clear();
        }
    } else {
        frontBuffer_->append(data, len);
    }
}

void LogDaemon::flush()
{
    std::lock_guard<std::mutex> lock(mutex_);
    flushLocked();
}

void LogDaemon::flushLocked()
{
}
}
}
