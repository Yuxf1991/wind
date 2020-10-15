//
// Created by tracy on 2020/9/20.
//

#ifndef WIND_COUNTDOWNLATCH_H
#define WIND_COUNTDOWNLATCH_H

#include "Mutex.h"
#include "base/Condition.h"

namespace wind {
    class CountDownLatch : NonCopyable {
    public:
        explicit CountDownLatch(int count) : m_count(count) {}
        ~CountDownLatch() = default;

        void wait();
        int getCount() const;
        void countDown();

    private:
        mutable Mutex m_mutex;
        int m_count GUARDED_BY(m_mutex);
        Condition m_condition GUARDED_BY(m_mutex);
    };
} // namespace wind


#endif//WIND_COUNTDOWNLATCH_H
