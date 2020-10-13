//
// Created by tracy on 2020/9/19.
//

#include "Thread.h"

#include <atomic>
#include <sys/prctl.h>

#include "CountDownLatch.h"
#include "Utils.h"

namespace wind {
    namespace threadHelper {
        void afterFork() {
            CurrentThread::t_TLS.tid = 0;
            CurrentThread::t_TLS.threadName = "main";
            CurrentThread::cacheTid();
        }

        void beforeFork() {
        }

        struct MainThreadInitializer {
            MainThreadInitializer() {
                CurrentThread::t_TLS.threadName = "main";
                CurrentThread::cacheTid();
                pthread_atfork(&beforeFork,nullptr, &afterFork);
            }
            ~MainThreadInitializer() = default;
        };
        [[maybe_unused]] MainThreadInitializer initMainThread;

        void Helper::run() {
            tid = CurrentThread::tid();
            latch.countDown();
            CurrentThread::t_TLS.threadName = (name.empty() ? defaultThreadName : name.c_str());
            ::prctl(PR_SET_NAME, CurrentThread::t_TLS.threadName);
            try {
                func();
                CurrentThread::t_TLS.threadName = "finished";
            } catch (std::exception &e) {
                CurrentThread::t_TLS.threadName = "crashed";
                fprintf(stderr, "exception caught in Thread %s\n", name.c_str());
                fprintf(stderr, "reason: %s\n", e.what());
                fprintf(stderr, "stack trace: %s\n", CurrentThread::stackTrace().c_str());
                abort();
            } catch (...) {
                CurrentThread::t_TLS.threadName = "crashed";
                fprintf(stderr, "unknown exception caught in Thread %s\n", name.c_str());
                throw; // rethrow
            }
        }

        void *startThread(void *data) {
            auto *helper = (Helper *)(data);
            helper->run();
            return nullptr;
        }
    } // namespace threadHelper

    std::atomic<int> Thread::m_numThread = 0;

    Thread::Thread(ThreadFunctor func, std::string name)
        : m_func(std::move(func)), m_name(std::move(name)), m_latch(1) {
        setThreadName();
    }

    Thread::~Thread() {
        if (m_started && !m_joined && !m_detached) {
            detach();
        }
    }

    void Thread::start() {
        assert(!m_started);
        if (m_started) {
            return;
        }
        m_started = true;
        m_startHelper = std::make_unique<threadHelper::Helper>(m_func, m_name, m_tid, m_latch);
        if (pthread_create(&m_pthreadId, nullptr, &threadHelper::startThread, m_startHelper.get()) != 0) {
            m_started = false;
            // TODO: use our logging
            fprintf(stderr, "%s: Failed in pthread_create.\n", m_name.c_str());
        } else {
            m_latch.wait();
            // TODO: use our logging
            fprintf(stdout, "%s: thread started, tid is %d.\n", m_name.c_str(), m_tid);
        }
    }

    void Thread::detach() {
        assert(m_started);
        assert(!m_joined);
        assert(!m_detached);
        if (m_started && !m_joined && !m_detached) {
            m_detached = true;
            PTHREAD_CHECK(pthread_detach(m_pthreadId));
        }
    }

    void Thread::join() {
        assert(m_started);
        assert(!m_joined);
        assert(!m_detached);
        if (m_started && !m_joined && !m_detached) {
            m_joined = true;
            PTHREAD_CHECK(pthread_join(m_pthreadId, nullptr));
        }
    }

    void Thread::setThreadName() {
        if (m_name.empty()) {
            m_name = "Thread" + std::to_string(m_numThread);
        }
        ++m_numThread;
    }
} // namespace wind