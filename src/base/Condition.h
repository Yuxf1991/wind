//
// Created by tracy on 2020/9/20.
//

#ifndef WIND_CONDITION_H
#define WIND_CONDITION_H

#include <functional>
#include <cassert>

#include "Mutex.h"
#include "NonCopyable.h"

namespace wind {
    class Condition : NonCopyable {
    public:
        Condition() {
            PTHREAD_CHECK(pthread_cond_init(&m_pCondition, nullptr));
        }

        ~Condition() {
            PTHREAD_CHECK(pthread_cond_destroy(&m_pCondition));
        }

        template <typename MutexType, typename Predictor>
        void wait(UniqueLock<MutexType> &lock, Predictor func) {
            while (!func()) {
                UniqueLock<Mutex>::UnAssignGuard tmp(lock);
                PTHREAD_CHECK(pthread_cond_wait(&m_pCondition, lock.getPthreadMutex()));
            }
        }

        void notify() {
            PTHREAD_CHECK(pthread_cond_signal(&m_pCondition));
        }

        void notifyAll() {
            PTHREAD_CHECK(pthread_cond_broadcast(&m_pCondition));
        }

    private:
        pthread_cond_t m_pCondition{};
    };
} // namespace wind

#endif//WIND_CONDITION_H
