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

LogDaemon::LogDaemon(std::string baseName, std::string logDir, FileSize rollSize, uint32_t flushInterval)
    : baseName_(std::move(baseName)),
      logDir_(std::move(logDir)),
      file_(std::make_unique<LogFile>(generateLogFileName())),
      rollSize_(rollSize),
      flushInterval_(flushInterval),
      frontBuffer_(std::make_unique<LogBuffer>()),
      reservedBuffer_(std::make_unique<LogBuffer>())
{
    if (!fs::is_directory(logDir_)) {
        logDir_ = logDir_.parent_path();
    }

    LogStream::setOutputFunc([this](const char *data, std::size_t len) { append(data, len); });
    LogStream::setFlushFunc([this]() { flush(); });
}

LogDaemon::~LogDaemon() noexcept
{
    running_ = false;
    cond_.notify_one();
    if (thread_.joinable()) {
        thread_.join();
    }
}

std::string LogDaemon::generateLogFileName()
{
    // BaseName
    std::stringstream ss;
    ss << logDir_.append(baseName_).string();

    // TimeStamp
    TimeType microSecondsSinceEpoch = TimeStamp::now().get();
    TimeType seconds = microSecondsSinceEpoch / MICRO_SECS_PER_SECOND;
    ss << "." << std::put_time(::localtime(&seconds), "%Y%m%d-%H%M%S");

    // HostName
    char buf[256] = {0};
    (void)::gethostname(buf, sizeof(buf));
    ss << "." << buf;

    // Pid
    ss << "." << CurrentThread::pidString();

    // Suffix
    ss << ".log";
    return ss.str();
}

void LogDaemon::start()
{
    if (running_) {
        return;
    }

    running_ = true;
    thread_ = make_thread("WindLogDaemon", [this]() { threadMain(); });
}

void LogDaemon::threadMain()
{
    std::vector<LogBufferPtr> buffersToWrite;
    LogBufferPtr tmpBuffer1 = std::make_unique<LogBuffer>();
    LogBufferPtr tmpBuffer2 = std::make_unique<LogBuffer>();
    while (running_) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (stagingBuffers_.empty()) {
                cond_.wait_for(
                    lock, std::chrono::seconds(flushInterval_), [this]() { return !stagingBuffers_.empty(); });
            }
            if (stagingBuffers_.empty()) {
                stagingBuffers_.push_back(std::move(frontBuffer_));
                frontBuffer_ = std::move(tmpBuffer1);
            }

            buffersToWrite.swap(stagingBuffers_);
            ASSERT(!buffersToWrite.empty());
        }

        for (const auto &buf : buffersToWrite) {
            file_->write(buf->data(), buf->length());
            if (file_->size() >= rollSize_) {
                file_ = std::make_unique<LogFile>(generateLogFileName());
            }
        }

        if (tmpBuffer1 == nullptr) {
            tmpBuffer1 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
        }

        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (reservedBuffer_ == nullptr) {
                reservedBuffer_ = std::move(tmpBuffer2);
            }
        }

        if (tmpBuffer2 == nullptr) {
            if (buffersToWrite.empty()) {
                tmpBuffer2 = std::make_unique<LogBuffer>();
            } else {
                tmpBuffer2 = std::move(buffersToWrite.back());
                buffersToWrite.pop_back();
            }
        }
    }
}

void LogDaemon::append(const char *data, std::size_t len)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (frontBuffer_->available() < len) {
        stagingBuffers_.push_back(std::move(frontBuffer_));
        if (reservedBuffer_ != nullptr) {
            frontBuffer_ = std::move(reservedBuffer_);
        } else {
            frontBuffer_ = std::make_unique<LogBuffer>();
        }
        cond_.notify_one();
    }

    frontBuffer_->append(data, len);
}

void LogDaemon::flush()
{
    std::lock_guard<std::mutex> lock(mutex_);
    cond_.notify_one();
}
} // namespace base
} // namespace wind
