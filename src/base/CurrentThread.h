//
// Created by tracy on 2020/9/19.
//

#ifndef WIND_CURRENTTHREAD_H
#define WIND_CURRENTTHREAD_H

#include <string>

namespace wind {
    namespace CurrentThread {
        // Thread Local Storage
        typedef struct {
            pid_t tid = 0;
            const char *threadName = "null";
        } TLS;

        extern __thread TLS t_TLS;

        void cacheTid();

        inline pid_t tid() {
            if (__builtin_expect(t_TLS.tid == 0, false)) {
                cacheTid();
            }
            return t_TLS.tid;
        }

        bool isMainThread();

        std::string stackTrace(bool demangle=false);
    } // namespace CurrentThread
} // namespace wind

#endif // WIND_CURRENTTHREAD_H
