//
// Created by tracy on 2021/11/10.
//

#ifndef WIND_CURRENTTHREAD_H
#define WIND_CURRENTTHREAD_H

#include <unistd.h>

#include "Types.h"

namespace wind {
typedef pid_t ThreadId;
typedef pid_t ProcessId;

namespace CurrentThread {
struct TLS {
    ThreadId tid;
    const char *name;
};
extern __thread TLS t_tls;

void cacheTid();

inline ThreadId tid()
{
    if (WIND_UNLIKELY(t_tls.tid == 0)) {
        cacheTid();
    }

    return t_tls.tid;
}

inline bool isMainThread()
{
    return tid() == ::getpid();
}

} // namespace CurrentThread
} // namespace wind
#endif // WIND_CURRENTTHREAD_H
