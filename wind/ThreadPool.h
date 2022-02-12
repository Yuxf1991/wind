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

#ifndef WIND_THREAD_POOL_H
#define WIND_THREAD_POOL_H

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>

#include "NonCopyable.h"
#include "Thread.h"

namespace wind {
using Task = std::function<void()>;

class ThreadPool : NonCopyable {
public:
    ThreadPool();
    ThreadPool(string name);
    ~ThreadPool() noexcept;

    bool isRunning() const { return running_.load(std::memory_order_acquire); }

    void setThreadNum(size_t threadNum);
    // Set max tasks this threadPool can hold.
    void setTaskCapacity(size_t taskCapacity);

    size_t getQueueSize() const { return tasks_.size(); }
    bool isFull() const { return taskCapacity_ == getQueueSize(); }
    bool isEmpty() const { return tasks_.empty(); }

    // Start all the threads in the thread pool.
    void start();
    // Stop all the threads in the thread pool and join them.
    void stop() noexcept;

    // Put task to run in the threadPool.
    // This func can only be called after calling ThreadPool::start(),
    // and can not be called after calling ThreadPool::stop(),
    void runTask(Task task);

private:
    void assertIsRunning() const;
    void assertIsNotRunning() const;

    void postTaskLocked(Task &&task);
    Task fetchTaskLocked();

    Task fetchTask();
    void threadMain();

    string name_;

    mutable std::mutex mutex_;
    std::condition_variable notEmptyCond_;
    std::condition_variable notFullCond_;

    std::atomic<bool> running_ = false;

    std::vector<Thread> threads_;
    std::queue<Task> tasks_;

    static constexpr size_t DEFAULT_TASK_CAPACITY = 10;
    size_t taskCapacity_ = DEFAULT_TASK_CAPACITY;
};
} // namespace wind
#endif // WIND_THREAD_POOL_H
