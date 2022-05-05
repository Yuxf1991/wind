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

#include "EventLoopThread.h"

#include "Log.h"

namespace wind {
namespace base {
EventLoopThread::EventLoopThread() : EventLoopThread("WindEventLoopThread") {}

EventLoopThread::EventLoopThread(std::string name) : name_(std::move(name)) {}

EventLoopThread::~EventLoopThread() noexcept
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (loop_ != nullptr) {
            loop_->stop();
        }
    }

    if (thread_.joinable()) {
        thread_.join();
    }

    LOG_DEBUG << name_ << " Stopped.";
}

EventLoop *EventLoopThread::start()
{
    thread_ = make_thread(name_, [this]() { loopThreadFunc(); });

    std::unique_lock<std::mutex> lock(mutex_);
    cond_.wait(lock, [this]() -> bool { return loop_ != nullptr; });
    return loop_;
}

void EventLoopThread::loopThreadFunc()
{
    EventLoop loop;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }

    loop.start();

    {
        std::lock_guard<std::mutex> lock(mutex_);
        loop_ = nullptr;
    }
}
} // namespace base
} // namespace wind
