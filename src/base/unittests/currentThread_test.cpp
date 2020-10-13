//
// Created by tracy on 2020/9/20.
//

#include <thread>

#include <gtest/gtest.h>

#include "base/CurrentThread.h"

namespace wind {
    namespace test {
        TEST(CurrentThread, IsMainThreadTest) {
            // main thread
            EXPECT_EQ(CurrentThread::isMainThread(), true);

            // other thread
            bool res = true; // set to true deliberately.
            // TODO: use our own thread class.
            std::thread t{ [&res]() { res = CurrentThread::isMainThread(); }};
            t.join();
            EXPECT_EQ(res, false);
        }

        TEST(CurrentThread, CacheTidTest) {
            // main thread
            printf("main thread's tid is %d\n", CurrentThread::tid());
            EXPECT_GT(CurrentThread::tid(), 0);

            // other thread
            pid_t tid = 0;
            // TODO: use our own thread class.
            std::thread t{ [&tid]() { tid = CurrentThread::tid(); }};
            t.join();
            printf("other thread's tid is %d\n", tid);
            EXPECT_GT(tid, 0);
        }
    } // namespace test
} // namespace wind
