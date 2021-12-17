// MIT License

// Copyright (c) 2020 Tracy

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "CurrentThread.h"
#include <gtest/gtest.h>

#include <iostream>
#include <thread>
using namespace std;

namespace wind {
TEST(CurrentThreadTest, TidTest)
{
    cout << "current thread tid is: " << CurrentThread::tid() << endl;
    EXPECT_GT(CurrentThread::tid(), 0);
    EXPECT_EQ(CurrentThread::isMainThread(), true);

    std::thread t{[]() {
        cout << "current thread tid is: " << CurrentThread::tid() << endl;
        EXPECT_GT(CurrentThread::tid(), 0);
        EXPECT_EQ(CurrentThread::isMainThread(), false);
    }};
    t.join();
}

TEST(CurrentThreadTest, MainThreadInitTest)
{
    cout << CurrentThread::name() << endl;
    EXPECT_EQ(strcmp(CurrentThread::name(), "main"), 0);
    std::thread t{[]() {
        cout << CurrentThread::name() << endl;
        EXPECT_EQ(strcmp(CurrentThread::name(), "unknown"), 0);
    }};
    t.join();
}
} // namespace wind