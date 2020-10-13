//
// Created by tracy on 2020/9/19.
//

#include "Thread.h"
#include "CountDownLatch.h"

#include <atomic>

namespace wind {
    namespace threadHelper {
        void Helper::run() {
            tid = CurrentThread::tid();
            latch.countDown();
            CurrentThread::t_TLS.threadName = (name.empty() ? defaultThreadName : name.c_str());

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
        if (pthread_create(&m_pthreadId, nullptr, &threadHelper::startThread, m_startHelper.get())
            != 0) {
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
            if (pthread_detach(m_pthreadId) != 0) {
                // TODO: use our logging
                fprintf(stderr, "%s: Failed in pthread_detach.\n", m_name.c_str());
            } else {
                // TODO: use our logging
                fprintf(stdout, "%s: thread detached, tid is %d.\n", m_name.c_str(), m_tid);
            }
        }
    }

    void Thread::join() {
        assert(m_started);
        assert(!m_joined);
        assert(!m_detached);
        if (m_started && !m_joined && !m_detached) {
            m_joined = true;
            if (pthread_join(m_pthreadId, nullptr) != 0) {
                // TODO: use our logging
                fprintf(stderr, "%s: Failed in pthread_join.\n", m_name.c_str());
            } else {
                // TODO: use our logging
                fprintf(stdout, "%s: thread joined, tid is %d.\n", m_name.c_str(), m_tid);
            }
        }
    }

    void Thread::setThreadName() {
        if (m_name.empty()) {
            m_name = "Thread" + std::to_string(m_numThread);
        }
        ++m_numThread;
    }
} // namespace wind