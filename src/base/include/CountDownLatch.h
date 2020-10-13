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

        int getCount() const {
            LockGuard<Mutex> lock(m_mutex);
            return m_count;
        }

        void countDown() {
            LockGuard<Mutex> lock(m_mutex);
            m_count--;
            if (m_count == 0) {
                m_condition.notifyAll();
            }
        }

        void wait() {
            UniqueLock<Mutex> lock(m_mutex);
            m_condition.wait(lock, [this]() { return m_count <= 0; });
        }

    private:
        mutable Mutex m_mutex;
        int m_count GUARDED_BY(m_mutex);
        Condition m_condition GUARDED_BY(m_mutex);
    };
} // namespace wind


#endif//WIND_COUNTDOWNLATCH_H
