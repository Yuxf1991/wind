//
// Created by tracy on 2020/10/15.
//

#include "CountDownLatch.h"

namespace wind {
    int CountDownLatch::getCount() const {
        LockGuard<Mutex> lock(m_mutex);
        return m_count;
    }

    void CountDownLatch::countDown() {
        LockGuard<Mutex> lock(m_mutex);
        m_count--;
        if (m_count == 0) {
            m_condition.notifyAll();
        }
    }

    void CountDownLatch::wait() {
        UniqueLock<Mutex> lock(m_mutex);
        m_condition.wait(lock, [this]() { return m_count <= 0; });
    }
} // namespace wind