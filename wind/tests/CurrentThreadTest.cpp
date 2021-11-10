//
// Created by tracy on 2021/11/10.
//

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
} // namespace wind