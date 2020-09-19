//
// Created by tracy on 2020/9/19.
//

#ifndef WIND_THREAD_H
#define WIND_THREAD_H

#include <atomic>
#include <functional>
#include <pthread.h>

#include "NonCopyable.h"

namespace wind {
    class Thread : NonCopyable{
    public:
        using ThreadFunctor = std::function<void()>;

        explicit Thread(ThreadFunctor func, std::string name=std::string{});
        ~Thread();

        void start();

    private:
        void setThreadName();

        ThreadFunctor m_func;
        std::string m_name;

        bool m_started = false;
        bool m_joined = false;

        pthread_t m_pthreadId;
        pid_t m_tid;

        static std::atomic<int> m_numThread;
    };
} // namespace wind

#endif //WIND_THREAD_H
