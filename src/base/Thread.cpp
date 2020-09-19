//
// Created by tracy on 2020/9/19.
//

#include "Thread.h"

#include <atomic>

namespace wind {

    namespace details {
        struct StartThreadHelper {
            using Functor = wind::Thread::ThreadFunctor;
            Functor func;
            std::string name;
            pid_t *tid;

        };
    } // namespace details

    std::atomic<int> Thread::m_numThread = 0;

    Thread::Thread(ThreadFunctor func, std::string name)
        : m_func(std::move(func)), m_name(std::move(name)) {
        setThreadName();
    }

    Thread::~Thread() {
        if (m_started && !m_joined) {
            pthread_detach(m_pthreadId);
        }
    }

    void Thread::start() {
        if (m_started) {
            return;
        }
    }

    void Thread::setThreadName() {
        if (m_name.empty()) {
            m_name = "Thread" + std::to_string(m_numThread);
        }
        ++m_numThread;
    }


} // namespace wind