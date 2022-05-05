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

#include <unordered_map>
#include <vector>

#include "EventChannel.h"
#include "UniqueFd.h"

namespace wind {
namespace base {
class EventLoop;

class EventPoller : NonCopyable {
public:
    explicit EventPoller(EventLoop *eventLoop);
    ~EventPoller() noexcept;

    TimeStamp pollOnce(std::vector<std::shared_ptr<EventChannel>> &activeChannels, int timeOutMs);
    void updateChannel(std::shared_ptr<EventChannel> channel);
    void removeChannel(int fd);

private:
    void assertInLoopThread();

    void epollCtl(const std::shared_ptr<EventChannel> &channel, int operation);
    EventLoop *eventLoop_ = nullptr;
    UniqueFd epollFd_;
    static std::size_t eventSize_;
    std::vector<epoll_event> activeEvents_; // to receive events from epoll_wait.
    std::unordered_map<int, std::shared_ptr<EventChannel>> channels_;
};
} // namespace base
} // namespace wind
