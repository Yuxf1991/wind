//
// Created by tracy on 2021/11/10.
//

#include "CurrentThread.h"

#include <sys/syscall.h>

namespace wind {
namespace detail {
ThreadId getThreadId()
{
    return static_cast<ThreadId>(::syscall(SYS_gettid));
}

void mainThreadInit()
{
    CurrentThread::cacheTid();
    CurrentThread::t_tls.name = "main";
}

struct MainThreadInitializer {
    MainThreadInitializer() noexcept
    {
        mainThreadInit();
    }
};

[[maybe_unused]] MainThreadInitializer mainThreadInitializer;
} // namespace detail

namespace CurrentThread {
__thread TLS t_tls = {.tid = 0, .name = "unknown"};

void cacheTid()
{
    if (t_tls.tid == 0) {
        t_tls.tid = detail::getThreadId();
    }
}

} // namespace CurrentThread
} // namespace wind