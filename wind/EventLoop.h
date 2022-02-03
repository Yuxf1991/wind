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

#ifndef WIND_EVENT_LOOP_H
#define WIND_EVENT_LOOP_H

#include <atomic>
#include <mutex>

#include "EventPoller.h"

namespace wind {
using Functor = std::function<void()>;

class EventLoop : NonCopyable {
public:
    EventLoop();
    ~EventLoop() noexcept;
    void start();
    void stop();
    void updateChannel(const std::shared_ptr<EventChannel> &channel);
    void removeChannel(int channelFd);

    void runInLoop(Functor func);

    void runAt(Functor func, TimeStamp dstTime);
    // delay in micro seconds, 0 means run immediately
    // interval in micro seconds, -1 means only run once.
    void runAfter(Functor func, TimeType delay);
    void runEvery(Functor func, TimeType interval, TimeType delay = 0);

    static EventLoop *eventLoopOfCurrThread();

    void assertInLoopThread();
    void assertNotInLoopThread();

private:
    bool isInLoopThread();
    void wakeUp();
    void wakeUpCallback();

    void queueToPendingFunctors(Functor func);
    void execPendingFunctors();

    mutable std::mutex mutex_;
    std::atomic<bool> running_ = false;

    std::unique_ptr<EventPoller> poller_;
    std::shared_ptr<EventChannel> wakeUpChannel_;

    ThreadId tid_ = -1; // indicates which thread is this loop in.
    std::unordered_map<int, std::shared_ptr<EventChannel>> holdChannels_;

    std::atomic<bool> executingPendingFunctors_ = false;
    std::vector<Functor> pendingFunctors_;
};
} // namespace wind
#endif // WIND_EVENT_LOOP_H
