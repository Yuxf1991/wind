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

#pragma once

#include "Types.h"
#include "Utils.h"

namespace wind {
namespace base {
namespace CurrentThread {
// Thread Local Storage
struct TLS {
    ThreadId tid;
    const char *name;
    char tidString[16]; // current tid string
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

ProcessId pid();

inline bool isMainThread()
{
    return tid() == pid();
}

inline const char *name()
{
    return t_tls.name;
}

const char *tidString();
const char *pidString();
} // namespace CurrentThread
} // namespace base
} // namespace wind
