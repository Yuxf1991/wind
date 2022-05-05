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

#include <iostream>
#include <thread>

#define LOG_TAG "LogTest"
#include "Log.h"

using namespace wind;

bool g_stop = false;

void subThreadFunc()
{
    while (!g_stop) {
        LOG_TRACE << "Hello this is a trace log";
        LOG_DEBUG << "Hello this is a debug log";
        LOG_INFO << "Hello this is an info log";
        LOG_WARN << "Hello this is a warning log";
        LOG_ERROR << "Hello this is an error log";
        ::sleep(2);
    }
}

int main()
{
    std::thread t{subThreadFunc};

    while (true) {
        LOG_TRACE << "Hello this is a trace log";
        LOG_DEBUG << "Hello this is a debug log";
        LOG_INFO << "Hello this is an info log";
        LOG_WARN << "Hello this is a warning log";
        LOG_ERROR << "Hello this is an error log";
        // LOG_SYS_FATAL << "Hello this is a sys fatal log";

        std::string tmp;
        std::getline(std::cin, tmp);
        if (tmp.size() == 1 && std::toupper(tmp[0]) == 'Q') {
            g_stop = true;
            break;
        }
    }

    if (t.joinable()) {
        t.join();
    }

    return 0;
}
