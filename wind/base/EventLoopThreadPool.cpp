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

#include "EventLoopThreadPool.h"

namespace wind {
namespace base {
EventLoopThreadPool::EventLoopThreadPool(EventLoop *mainLoop, std::string name)
    : mainLoop_(mainLoop), name_(std::move(name))
{
    if (mainLoop_ == nullptr) {
        LOG_SYS_FATAL << "EventLoopThreadPool::ctor: EventLoop is null!";
    }
}

EventLoopThreadPool::~EventLoopThreadPool() noexcept
{
    running_ = false;
    currLoopIdx_ = 0;
    threadNum_ = 0;
    loops_.clear();
    loopThreads_.clear();
}

void EventLoopThreadPool::start()
{
    if (running_) {
        return;
    }

    mainLoop_->assertInLoopThread();

    running_ = true;
    if (threadNum_ == 0) {
        return;
    }

    loopThreads_.resize(threadNum_);
    loops_.resize(threadNum_);
    for (size_t i = 0; i != threadNum_; ++i) {
        loopThreads_[i] = std::make_unique<EventLoopThread>(name_ + "_loop_" + std::to_string(i));
        loops_[i] = loopThreads_[i]->start();
    }
}

EventLoop *EventLoopThreadPool::getNextLoop()
{
    mainLoop_->assertInLoopThread();
    if (!running_ || threadNum_ == 0) {
        return mainLoop_;
    }

    ASSERT(currLoopIdx_ < threadNum_);
    ASSERT(threadNum_ == loops_.size());

    EventLoop *loop = loops_[currLoopIdx_++];
    if (currLoopIdx_ >= threadNum_) {
        currLoopIdx_ = 0;
    }
    return loop;
}
} // namespace base
} // namespace wind
