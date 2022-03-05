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

#include <future>
#include <type_traits>

#include "EventPoller.h"
#include "TimerManager.h"

namespace wind {
namespace base {
using Functor = std::function<void()>;

namespace detail {
template <typename Callable>
class PackagedTask : NonCopyable {
    using Return = std::invoke_result_t<Callable>;

public:
    ~PackagedTask() noexcept = default;

    static auto Create(Callable &&cb)
    {
        std::shared_ptr<PackagedTask<Callable>> t(new PackagedTask(std::forward<Callable &&>(cb)));
        return std::make_pair(t, t->cb_.get_future());
    }

    void Run()
    {
        cb_();
    }

private:
    explicit PackagedTask(Callable &&cb) : cb_(std::move(cb)) {}

    std::packaged_task<Return()> cb_;
};
} // namespace detail

class EventLoop : NonCopyable {
public:
    EventLoop();
    ~EventLoop() noexcept;
    void start();
    void stop() noexcept;
    void updateChannel(std::shared_ptr<EventChannel> channel);
    void removeChannel(int channelFd);

    template <typename Task, typename Ret = std::invoke_result_t<Task>>
    std::future<Ret> schedule(Task task)
    {
        auto [packagedTask, future] = detail::PackagedTask<Task>::Create(std::move(task));
        runInLoop([t(std::move(packagedTask))]() { t->Run(); });
        return std::move(future);
    }
    void runInLoop(Functor func);

    TimerId runAt(Functor func, TimeStamp dstTime);
    // delay in micro seconds, 0 means run immediately
    // interval in micro seconds, 0 means only run once.
    TimerId runAfter(Functor func, TimeType delay);
    TimerId runEvery(Functor func, TimeType interval, TimeType delay = 0);
    void cancel(const TimerId &timerId);

    static EventLoop *eventLoopOfCurrThread();

    bool isInLoopThread() const;

    // will abort if not in loop thread.
    void assertInLoopThread() const;
    // will abort if in loop thread.
    void assertNotInLoopThread() const;

private:
    void wakeUp();
    void wakeUpCallback();

    void queueToPendingFunctors(Functor func);
    void execPendingFunctors();

    ThreadId tid_ = -1; // indicates which thread is this loop in.

    mutable std::mutex mutex_;
    std::atomic<bool> running_ = false;

    std::unique_ptr<EventPoller> poller_;

    UniqueFd wakeUpFd_;
    std::shared_ptr<EventChannel> wakeUpChannel_;

    std::atomic<bool> executingPendingFunctors_ = false;
    std::vector<Functor> pendingFunctors_;

    std::unique_ptr<TimerManager> timerManager_;
};
} // namespace base
} // namespace wind
