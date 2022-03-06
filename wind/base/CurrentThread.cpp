// MIT License

// Copyright (c) 2020 Tracy

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "CurrentThread.h"

#include <sys/syscall.h>

namespace wind {
namespace base {

namespace detail {
static ProcessId pid = 0;
static char pidString[16];

inline ThreadId getThreadId()
{
    return static_cast<ThreadId>(::syscall(SYS_gettid));
}

void mainThreadInit()
{
    CurrentThread::cacheTid();
    CurrentThread::t_tls.name = "main";
    pid = CurrentThread::pid();
    int len = ::snprintf(pidString, sizeof(pidString), "%d", pid);
    ASSERT(static_cast<size_t>(len) < sizeof(pidString));
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

ProcessId pid()
{
    return ::getpid();
}

void cacheTid()
{
    if (t_tls.tid == 0) {
        t_tls.tid = detail::getThreadId();
    }

    int len = ::snprintf(t_tls.tidString, sizeof(t_tls.tidString), "%d", t_tls.tid);
    ASSERT(static_cast<size_t>(len) < sizeof(t_tls.tidString));
}

const char *tidString()
{
    (void)tid();
    return t_tls.tidString;
}

const char *pidString()
{
    return detail::pidString;
}
} // namespace CurrentThread
} // namespace base
} // namespace wind
