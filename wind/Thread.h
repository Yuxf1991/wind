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

#ifndef WIND_THREAD_H
#define WIND_THREAD_H

#include <thread>

#include "CurrentThread.h"
#include "Log.h"

namespace wind {
using Thread = std::thread;

template <typename Func, typename... Args>
inline Thread make_thread(string name, Func &&func, Args &&... args)
{
    auto hookedFunc = [name(std::move(name)),
                       func(std::forward<Func &&>(func)),
                       args(std::make_tuple(std::forward<Args &&>(args)...))]() mutable {
        if (!name.empty()) {
            CurrentThread::t_tls.name = name.c_str();
        } else {
            CurrentThread::t_tls.name = "wind_thread";
        }

#ifdef ENABLE_EXCEPTION
        try {
            std::apply([&](auto &&... args) { func(args...); }, std::move(args));
            CurrentThread::t_tls.name = "finished";
        } catch (const std::exception &e) {
            CurrentThread::t_tls.name = "crashed";
            LOG_ERROR << "Thread " << CurrentThread::name() << " failed: " << e.what() << ".";
        } catch (...) {
            CurrentThread::t_tls.name = "crashed";
            throw; // rethrow
        }
#else
        std::apply([&](auto &&... args) { func(args...); }, std::move(args));
        CurrentThread::t_tls.name = "finished";
#endif // ENABLE_EXCEPTION
    };

    return Thread{std::move(hookedFunc)};
}
} // namespace wind
#endif // WIND_THREAD_H
