//
// Created by tracy on 2020/9/19.
//

#ifndef WIND_CURRENTTHREAD_H
#define WIND_CURRENTTHREAD_H

#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/unistd.h>

namespace wind {
    namespace CurrentThread {
        // Thread Local Storage
        typedef struct {
            int tid = 0;
            const char *threadName = "null";
        } TLS;

        extern __thread TLS t_TLS;

        inline pid_t gettid() {
            return ::syscall(SYS_gettid);
        }

        inline void cacheTid() {
            if (t_TLS.tid == 0) {
                t_TLS.tid = gettid();
            }
        }

        inline int tid() {
            if (__builtin_expect(t_TLS.tid == 0, false)) {
                cacheTid();
            }
            return t_TLS.tid;
        }

        inline bool isMainThread() {
            return tid() == ::getpid();
        }
    } // namespace CurrentThread
} // namespace wind

#endif // WIND_CURRENTTHREAD_H
