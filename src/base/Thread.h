//
// Created by tracy on 2020/9/19.
//

#ifndef WIND_THREAD_H
#define WIND_THREAD_H

#include <atomic>
#include <functional>
#include <memory>
#include <pthread.h>

#include "NonCopyable.h"
#include "CountDownLatch.h"

namespace wind {
    using ThreadFunctor = std::function<void()>;

    namespace threadHelper {
        constexpr char defaultThreadName[] = "windThread";

        struct Helper {
            Helper(ThreadFunctor func, std::string name, pid_t &tid, CountDownLatch &latch)
                    : func(std::move(func)), name(std::move(name)), tid(tid), latch(latch) {}
            ~Helper() = default;

            ThreadFunctor func;
            std::string name;
            pid_t &tid;
            CountDownLatch &latch;

            void run();
        };
    } // namespace threadHelper

    class Thread : NonCopyable{
    public:
        explicit Thread(ThreadFunctor func, std::string name=std::string{});
        ~Thread();

        void start();
        void detach();
        void join();
        bool joinable() const {
            return (m_started && m_func && !m_joined && !m_detached);
        }

    private:
        void setThreadName();

        ThreadFunctor m_func;
        std::string m_name;

        bool m_started = false;
        bool m_detached = false;
        bool m_joined = false;

        pthread_t m_pthreadId;
        pid_t m_tid;
        CountDownLatch m_latch;
        static std::atomic<int> m_numThread;

        std::unique_ptr<threadHelper::Helper> m_startHelper;
    };
} // namespace wind

#endif //WIND_THREAD_H
