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

#include "ThreadPool.h"

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "WindThreadPool"
#endif // LOG_TAG
#include "Log.h"

namespace wind {
ThreadPool::ThreadPool() : ThreadPool("WindThreadPool") {}

ThreadPool::ThreadPool(string name) : name_(name) {}

ThreadPool::~ThreadPool() noexcept
{
    stop();
}

void ThreadPool::stop() noexcept
{
    if (!isRunning()) {
        return;
    }

    running_.store(false, std::memory_order::memory_order_acquire);

    {
        std::lock_guard<std::mutex> lock(mutex_);
        notEmptyCond_.notify_all();
    }

    for (auto &t : threads_) {
        if (t.joinable()) {
            t.join();
        }
    }

    LOG_INFO << "ThreadPool(name: " << name_ << ") stopped.";
}

void ThreadPool::setThreadNum(size_t threadNum)
{
    assertIsNotRunning();
    threads_.resize(threadNum);
}

void ThreadPool::setTaskCapacity(size_t taskCapacity)
{
    assertIsNotRunning();
    taskCapacity_ = taskCapacity;
}

void ThreadPool::assertIsRunning() const
{
    LOG_FATAL_IF(!isRunning()) << "ThreadPool(name: " << name_ << ") is not running!";
}

void ThreadPool::assertIsNotRunning() const
{
    LOG_FATAL_IF(isRunning()) << "ThreadPool(name: " << name_ << ") is already started!";
}

// inner interface: guarantee the tasks_ is not full.
void ThreadPool::postTaskLocked(Task &&task)
{
    tasks_.push(std::move(task));
}

void ThreadPool::runTask(Task task)
{
    assertIsRunning();

    if (threads_.empty()) {
        LOG_INFO << "There is no any thread in threadPool(name: " << name_
                 << "), run the task in current thread immediately.";
        task();
    } else {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!isFull()) {
            notFullCond_.wait(lock, [this]() { return !isFull(); });
        }

        postTaskLocked(std::move(task));
        notEmptyCond_.notify_one();
    }
}

void ThreadPool::start()
{
    if (isRunning()) {
        LOG_WARN << "ThreadPool(name: " << name_ << ") is already started!";
        return;
    }

    running_.store(true, std::memory_order::memory_order_acquire);

    auto cnt = threads_.size();
    for (decltype(cnt) i = 0; i < cnt; ++i) {
        threads_[i] = make_thread(name_ + "_" + std::to_string(i), [this]() { threadMain(); });
    }
}

void ThreadPool::threadMain()
{
    while (isRunning()) {
        Task task = fetchTask();
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!isFull()) {
                notFullCond_.notify_one();
            }
        }

        if (task != nullptr) {
#ifdef ENABLE_EXCEPTION
            try {
                task();
            } catch (const std::exception &e) {
                LOG_ERROR << "Thread " << CurrentThread::name() << " execute task failed: " << e.what() << ".";
            } catch (...) {
                LOG_ERROR << "Thread " << CurrentThread::name() << " execute task failed.";
                throw; // rethrow
            }
#else
            task();
#endif // ENABLE_EXCEPTION
        }
    }

    LOG_INFO << "Thread " << CurrentThread::name() << " stopped.";
}

// inner interface: guarantee the tasks_ is not empty.
Task ThreadPool::fetchTaskLocked()
{
    LOG_FATAL_IF(isEmpty()) << "ThreadPool(name: " << name_ << "): tasks_ is empty!";
    Task task = std::move(tasks_.front());
    tasks_.pop();
    return task;
}

Task ThreadPool::fetchTask()
{
    Task task;
    std::unique_lock<std::mutex> lock(mutex_);

    if (isEmpty() && isRunning()) {
        notEmptyCond_.wait(lock);
    }

    if (!isEmpty()) {
        return fetchTaskLocked();
    }

    return nullptr;
}
} // namespace wind
