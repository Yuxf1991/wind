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

#include <unistd.h>
#define LOG_TAG "ThreadPoolTest"
#include "ThreadPool.h"

#include "UniqueFd.h"
using namespace wind;
using namespace wind::base;

namespace utils {
void writeDebugInfo(const std::string &info)
{
    static std::mutex mutex;
    std::lock_guard lock(mutex);
    LOG_INFO << info;
}
} // namespace utils

// test1: no thread in thread pool.
void ThreadPoolTest1()
{
    ThreadPool threadPool;
    threadPool.start();
    threadPool.runTask([]() { LOG_INFO << "hello world!"; });
}

// test2: no thread in thread pool, custom thread pool's name.
void ThreadPoolTest2()
{
    ThreadPool threadPool("ThreadPool_custom");
    threadPool.start();
    threadPool.runTask([]() { LOG_INFO << "hello world!"; });
}

// test3: wrong usages of thread pool.
void ThreadPoolTest3()
{
    ThreadPool threadPool;
    threadPool.start();
    // threadPool.setThreadNum(4);
    // threadPool.setTaskCapacity(20);
    threadPool.runTask([]() { LOG_INFO << "hello world!"; });
    // threadPool.stop();
    threadPool.runTask([]() { LOG_INFO << "hello world!"; });
    threadPool.start();
}

void Task1()
{
    ::utils::writeDebugInfo("task1 start.");
    sleep(1);
    ::utils::writeDebugInfo("task1 end.");
}

void Task2()
{
    ::utils::writeDebugInfo("task2 start.");
    sleep(2);
    ::utils::writeDebugInfo("task2 end.");
}

void Task3()
{
    ::utils::writeDebugInfo("task3 start.");
    sleep(3);
    ::utils::writeDebugInfo("task3 end.");
}

void ThreadPoolTest4()
{
    ThreadPool threadPool;
    threadPool.setThreadNum(8);
    threadPool.setTaskQueueCapacity(128);
    threadPool.start();
    for (int i = 0; i < 10000; ++i) {
        threadPool.runTask([]() { Task1(); });
        threadPool.runTask([]() { Task2(); });
        threadPool.runTask([]() { Task3(); });
        ::utils::writeDebugInfo("Put 3 tasks into the ThreadPool(name: " + threadPool.name() + ").");
        std::string dumpInfo;
        dumpInfo += "\n";
        threadPool.dump(dumpInfo);
        ::utils::writeDebugInfo(dumpInfo);
    }

    while (true) {
        if (threadPool.empty()) {
            break;
        }
        std::string dumpInfo;
        dumpInfo += "\n";
        threadPool.dump(dumpInfo);
        ::utils::writeDebugInfo(dumpInfo);
        sleep(5);
    }
}

int main()
{
    ThreadPoolTest1();
    ThreadPoolTest2();
    ThreadPoolTest3();
    ThreadPoolTest4();
    return 0;
}
