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

#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <sys/eventfd.h>

#define ENABLE_EXCEPTION 1

#ifdef NDEBUG
#define ASSERT(exp)
#else
#define ASSERT(exp) assert((exp))
#endif

#define UNUSED(val) (void)val;

#ifdef __cplusplus
#define WIND_LIKELY(x) (__builtin_expect(!!(x), true))
#define WIND_UNLIKELY(x) (__builtin_expect(!!(x), false))
#else
#define WIND_LIKELY(x) (__builtin_expect(!!(x), 1))
#define WIND_UNLIKELY(x) (__builtin_expect(!!(x), 0))
#endif

namespace wind {
namespace base {
namespace utils {
inline void memZero(void *data, size_t len)
{
    ::memset(data, 0, len);
}

int createEventFd();
} // namespace utils
} // namespace base
} // namespace wind
