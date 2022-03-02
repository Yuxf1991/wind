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

#include "Thread.h"
#include "TimeStamp.h"

namespace wind {
namespace base {
class ThreadPool : NonCopyable {
    using Task = std::function<void()>;
    static constexpr size_t DEFAULT_TASK_QUEUE_CAPACITY = 16;

public:
    ThreadPool();
    ThreadPool(string name);
    ~ThreadPool() noexcept;

    string name() const { return name_; }

    bool isRunning() const { return running_.load(std::memory_order_acquire); }

    void setThreadNum(size_t threadNum);
    void setTaskQueueCapacity(size_t capacity);

    bool empty() const { return taskSize() == 0; }

    size_t taskSize() const;

    // Start all the threads in the thread pool.
    void start();
    // Stop all the threads in the thread pool and join them.
    void stop() noexcept;

    // Put task to run in the threadPool.
    // This func can only be called after calling ThreadPool::start(),
    // and can not be called after calling ThreadPool::stop(),
    void runTask(Task task);

    // dump all workers' infomation.
    void dump(std::string &out) const;

private:
    void assertIsRunning() const;
    void assertIsNotRunning() const;

    string name_;
    std::atomic<bool> running_ = false;

    void addEmptyWorkerLocked(ThreadId workerId);
    void addEmptyWorker(ThreadId workerId);
    ThreadId getNextWorker();

    mutable std::mutex mutex_;

    class TaskWorker : NonCopyable {
    public:
        TaskWorker(string name, ThreadPool &pool);
        ~TaskWorker() noexcept;

        string name() const { return name_; }

        // Can shrink and expand capacity.
        void setTaskCapacity(size_t capacity);
        size_t getQueueSize() const
        {
            std::lock_guard<std::mutex> lock(mutex_);
            return getQueueSizeLocked();
        }
        size_t getQueueCapacity() const
        {
            std::lock_guard<std::mutex> lock(mutex_);
            return getQueueCapacityLocked();
        }
        bool isEmpty() const
        {
            std::lock_guard<std::mutex> lock(mutex_);
            return isEmptyLocked();
        }
        bool isFull() const
        {
            std::lock_guard<std::mutex> lock(mutex_);
            return isFullLocked();
        }

        ThreadId start();
        void postTask(Task task);

        TimeStamp readyTime() const { return readyTime_; }

    private:
        void stop() noexcept;
        Task fetchTask();
        void threadMain();

        bool isFullLocked() const { return getQueueSizeLocked() >= getQueueCapacityLocked(); }
        bool isEmptyLocked() const { return tasks_.empty(); }
        size_t getQueueSizeLocked() const { return tasks_.size(); }
        size_t getQueueCapacityLocked() const { return taskQueueCapacity_; }

        string name_;
        ThreadPool &pool_;

        mutable std::mutex mutex_;
        std::condition_variable initCond_;
        std::condition_variable notFullCond_;
        std::condition_variable notEmptyCond_;

        std::atomic<bool> running_;
        ThreadId tid_ = 0;
        Thread thread_;

        std::queue<Task> tasks_;
        size_t taskQueueCapacity_ = ThreadPool::DEFAULT_TASK_QUEUE_CAPACITY;

        TimeStamp readyTime_;
    };
    friend class TaskWorker;
    std::unordered_map<ThreadId, std::unique_ptr<TaskWorker>> workers_;
    std::queue<ThreadId> emptyWorkers_;

    size_t threadNum_ = 0;
    size_t taskQueueCapacity_ = DEFAULT_TASK_QUEUE_CAPACITY;
};
} // namespace base
} // namespace wind
#endif // WIND_THREAD_POOL_H
