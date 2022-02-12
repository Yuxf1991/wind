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

#define LOG_TAG "ThreadPoolTest"
#include "ThreadPool.h"

using namespace wind;

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
    for (int i = 0; i < 10000; ++i) {
        ::utils::writeDebugInfo("task1: " + std::to_string(i));
    }
}

void Task2()
{
    for (int i = 0; i < 10000; ++i) {
        ::utils::writeDebugInfo("task2: " + std::to_string(i));
    }
}

void ThreadPoolTest4()
{
    ThreadPool threadPool;
    threadPool.setThreadNum(10);
    threadPool.setTaskCapacity(200);
    threadPool.start();
    for (int i = 0; i < 1000; ++i) {
        threadPool.runTask([]() { Task1(); });
        threadPool.runTask([]() { Task2(); });
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
