//
// Created by tracy on 2020/9/19.
//

#include "CurrentThread.h"

#include <sys/syscall.h>
#include <sys/unistd.h>

#include "Utils.h"

namespace wind {
    namespace CurrentThread {
        __thread TLS t_TLS;

        void cacheTid() {
            if (t_TLS.tid == 0) {
                t_TLS.tid = ::syscall(SYS_gettid);
            }
        }

        bool isMainThread() {
            return tid() == ::getpid();
        }

        // TODO: get stackTrace for currentThread
        std::string stackTrace(bool demangle) {
            UNUSED_VALUE(demangle);
            std::string stackTrace;
            return stackTrace;
        }
    } // namespace CurrentThread
} // namespace wind