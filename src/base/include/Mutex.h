//
// Created by tracy on 2020/9/20.
//

#ifndef WIND_MUTEX_H
#define WIND_MUTEX_H

#include <cassert>
#include <pthread.h>

#include "base/CurrentThread.h"
#include "NonCopyable.h"
#include "PThreadCheck.h"

// Enable thread safety attributes only with clang.
#if defined(__clang__) && (!defined(SWIG))
#define THREAD_ANNOTATION_ATTRIBUTE__(x)   __attribute__((x))
#else
#define THREAD_ANNOTATION_ATTRIBUTE__(x)   // do nothing
#endif
#include "private/ClangTSA.h"

namespace wind {
    template <typename MutexType>
    class SCOPED_CAPABILITY UniqueLock : NonCopyable {
    public:
        explicit UniqueLock(MutexType &mutex) ACQUIRE(mutex)
                : m_mutex(mutex)
        {
            m_mutex.lock();
        }

        ~UniqueLock() RELEASE() { m_mutex.unlock(); }

    private:
        friend class Condition;
        class SCOPED_CAPABILITY UnAssignGuard : NonCopyable {
        public:
            explicit UnAssignGuard(UniqueLock<MutexType> &lock)
                    : m_lock(lock)
            {
                m_lock.m_mutex.unAssign();
            }

            ~UnAssignGuard() {
                m_lock.m_mutex.assign();
            }

        private:
            UniqueLock<MutexType> &m_lock;
        };

        pthread_mutex_t *getPthreadMutex() { return m_mutex.getPthreadMutex(); }

        MutexType &m_mutex;
    };

    template <typename MutexType>
    class SCOPED_CAPABILITY LockGuard : NonCopyable {
    public:
        explicit LockGuard(MutexType &mutex) ACQUIRE(mutex)
                : m_mutex(mutex)
        {
            m_mutex.lock();
        }

        ~LockGuard() RELEASE() { m_mutex.unlock(); }

    private:
        MutexType &m_mutex;
    };

    class CAPABILITY("mutex") Mutex : NonCopyable {
    public:
        Mutex() {
            PTHREAD_CHECK(pthread_mutex_init(&m_mutex, nullptr));
        }

        ~Mutex() {
            assert(m_holder == 0);
            PTHREAD_CHECK(pthread_mutex_destroy(&m_mutex));
        }

        void lock() ACQUIRE() {
            PTHREAD_CHECK(pthread_mutex_lock(&m_mutex));
            assign();
        }

        void unlock() RELEASE() {
            unAssign();
            PTHREAD_CHECK(pthread_mutex_unlock(&m_mutex));
        }

        bool isLockedByCurrentThread() {
            return m_holder == CurrentThread::tid();
        }

        pthread_mutex_t *getPthreadMutex() { return &m_mutex; }

    private:
        friend class UniqueLock<Mutex>;
        void assign() { m_holder = CurrentThread::tid(); }
        void unAssign() { m_holder = 0; }

        pthread_mutex_t m_mutex{};
        pid_t m_holder = 0;
    };
}

#endif//WIND_MUTEX_H
