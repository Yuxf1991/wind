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

#ifdef ENABLE_EXCEPTION
#include <stdexcept>
#endif // ENABLE_EXCEPTION

#include "Log.h"

namespace wind {
namespace base {
ThreadPool::TaskWorker::TaskWorker(std::string name) : name_(std::move(name)), readyTime_(TimeStamp::now()) {}

ThreadPool::TaskWorker::~TaskWorker() noexcept
{
    stop();
}

void ThreadPool::TaskWorker::stop() noexcept
{
    if (!running_) {
        return;
    }

    running_ = false;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        notEmptyCond_.notify_all();
    }

    if (thread_.joinable()) {
        thread_.join();
    }
}

void ThreadPool::TaskWorker::setTaskCapacity(std::size_t capacity)
{
    if (!running_) {
        taskQueueCapacity_ = capacity;
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    auto oldCapacity = taskQueueCapacity_;
    taskQueueCapacity_ = capacity;
    if (taskQueueCapacity_ > oldCapacity) {
        notFullCond_.notify_one();
    }
}

ThreadId ThreadPool::TaskWorker::start()
{
    if (running_) {
        return tid_;
    }

    running_ = true;
    thread_ = make_thread(name_, [this]() { threadMain(); });

    std::unique_lock<std::mutex> lock(mutex_);
    initCond_.wait(lock, [this]() { return tid_ != 0; });
    readyTime_ = TimeStamp::now();
    return tid_;
}

void ThreadPool::TaskWorker::postTask(Task task)
{
    std::unique_lock<std::mutex> lock(mutex_);
    while (isFullLocked()) {
        notFullCond_.wait(lock);
    }

    tasks_.push(std::move(task));
    lock.unlock();

    notEmptyCond_.notify_one();
}

ThreadPool::Task ThreadPool::TaskWorker::fetchTask()
{
    std::unique_lock<std::mutex> lock(mutex_);

    if (isEmptyLocked() && running_) {
        notEmptyCond_.wait(lock);
    }

    if (!isEmptyLocked()) {
        auto task = std::move(tasks_.front());
        tasks_.pop();
        return task;
    }

    return nullptr;
}

void ThreadPool::TaskWorker::threadMain()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        tid_ = CurrentThread::tid();
        initCond_.notify_all();
    }

    while (true) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!running_) {
                break;
            }
        }

        auto task = fetchTask();
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!isFullLocked()) {
                readyTime_ = TimeStamp::now();
                notFullCond_.notify_one();
            }
        }

        if (task != nullptr) {
#ifdef ENABLE_EXCEPTION
            try {
                task();
            } catch (const std::exception &e) {
                LOG_ERROR << "Thread " << name_ << " execute task failed: " << e.what() << ".";
            } catch (...) {
                LOG_ERROR << "Thread " << name_ << " execute task failed.";
                throw; // rethrow
            }
#else
            task();
#endif // ENABLE_EXCEPTION
        }
    }

    LOG_INFO << "Thread " << name_ << " stopped.";
}

ThreadPool::ThreadPool() : ThreadPool("WindThreadPool") {}

ThreadPool::ThreadPool(std::string name) : name_(std::move(name)) {}

ThreadPool::~ThreadPool() noexcept
{
    stop();
}

void ThreadPool::stop() noexcept
{
    if (!isRunning()) {
        return;
    }

    running_ = false;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        workers_.clear();
    }

    LOG_INFO << "ThreadPool(name: " << name_ << ") stopped.";
}

void ThreadPool::setThreadNum(std::size_t threadNum)
{
    assertIsNotRunning();

    std::lock_guard<std::mutex> lock(mutex_);
    threadNum_ = threadNum;
}

void ThreadPool::setTaskQueueCapacity(std::size_t capacity)
{
    std::lock_guard<std::mutex> lock(mutex_);
    taskQueueCapacity_ = capacity;

    for (auto &[_, worker] : workers_) {
        worker->setTaskCapacity(taskQueueCapacity_);
    }
}

void ThreadPool::assertIsRunning() const
{
    LOG_FATAL_IF(!isRunning()) << "ThreadPool(name: " << name_ << ") is not running!";
}

void ThreadPool::assertIsNotRunning() const
{
    LOG_FATAL_IF(isRunning()) << "ThreadPool(name: " << name_ << ") is already started!";
}

void ThreadPool::addEmptyWorker(ThreadId workerId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    addEmptyWorkerLocked(workerId);
}

void ThreadPool::addEmptyWorkerLocked(ThreadId workerId)
{
    emptyWorkers_.push(workerId);
}

// inner interface: guarantee all the workers are valid.
ThreadId ThreadPool::getNextWorker()
{
    ThreadId selectedWorker = 0;
    if (!emptyWorkers_.empty()) {
        selectedWorker = emptyWorkers_.front();
        emptyWorkers_.pop();
        return selectedWorker;
    }

    double minLoad = 100.0;
    for (const auto &[id, worker] : workers_) {
        double workerLoad =
            static_cast<double>(worker->getQueueSize()) * 1.0 / static_cast<double>(worker->getQueueCapacity()) * 100.0;
        if (workerLoad < minLoad) {
            minLoad = workerLoad;
            selectedWorker = id;
        }
    }

    // Maybe all the workers' load are 100%
    // TODO: select most hungry worker.
    if (selectedWorker == 0) {
        LOG_INFO << "ThreadPool::" << __func__ << ": all the workers' load are 100%, use the first worker by default.";
        selectedWorker = workers_.begin()->first;
    }

    return selectedWorker;
}

void ThreadPool::runTask(Task task)
{
    assertIsRunning();

    std::lock_guard<std::mutex> lock(mutex_);
    if (workers_.empty()) {
        LOG_INFO << "There is no any thread in threadPool(name: " << name_
                 << "), run the task in current thread immediately.";
        task();
    } else {
        auto workerId = getNextWorker();
        workers_.at(workerId)->postTask(std::move(task));
    }
}

void ThreadPool::start()
{
    if (isRunning()) {
        LOG_WARN << "ThreadPool(name: " << name_ << ") is already started!";
        return;
    }

    running_ = true;

    for (std::size_t i = 0; i != threadNum_; ++i) {
        auto worker = std::make_unique<TaskWorker>(name_ + "_" + std::to_string(i));
        worker->setTaskCapacity(taskQueueCapacity_);
        auto tid = worker->start();
        LOG_FATAL_IF(tid <= 0) << "Failed to start a worker in ThreadPool(name: " << name_ << ")!";

        std::lock_guard<std::mutex> lock(mutex_);
        workers_[tid] = std::move(worker);
        addEmptyWorkerLocked(tid);
    }
}

std::size_t ThreadPool::taskSize() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::size_t taskNum = 0;
    for (const auto &[_, worker] : workers_) {
        taskNum += worker->getQueueSize();
    }
    return taskNum;
}

void ThreadPool::dump(std::string &out) const
{
    out += "    name        | queue size | queue capacity |   load(%)  |    ready time  \n";
    for (const auto &[_, worker] : workers_) {
        auto queueSize = worker->getQueueSize();
        auto queueCapacity = worker->getQueueCapacity();
        double load = static_cast<double>(queueSize) * 1.0 / static_cast<double>(queueCapacity) * 100.0;
        out +=
            (worker->name() + "  |   " + std::to_string(queueSize) + "  |   " + std::to_string(queueCapacity) +
             "   |   " + std::to_string(load) + "    |   " + worker->readyTime().toFormattedString());
        out += "\n";
    }
}
} // namespace base
} // namespace wind
