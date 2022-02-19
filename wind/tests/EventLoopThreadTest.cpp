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

#define LOG_TAG "EventLoopThreadTest"
#include "EventLoopThread.h"

#include "SockAddrInet.h"

#include <iostream>
#include <sys/socket.h>

using namespace wind;

EventLoop *g_loop;

void echoFunc(int fd, TimeStamp receivedTime)
{
    char buf[1024] = {0};
    int len = TEMP_FAILURE_RETRY(read(fd, buf, sizeof(buf)));
    if (len < 0) {
        LOG_INFO << " recv msg err from client " << fd << ": " << strerror(errno);
    } else if (len == 0) {
        LOG_INFO << receivedTime.toFormattedString() << " client " << fd << " closed, remove this channel.";
        if (g_loop != nullptr) {
            g_loop->removeChannel(fd);
        }
    } else {
        LOG_INFO << receivedTime.toFormattedString() << " recv msg from client " << fd << ": " << buf;
        int ret = TEMP_FAILURE_RETRY(write(fd, buf, len));
        if (ret < 0) {
            LOG_INFO << receivedTime.toFormattedString() << " send msg err from client " << fd << ": "
                     << strerror(errno);
        } else {
            LOG_INFO << receivedTime.toFormattedString() << " send msg from client " << fd << ": " << buf;
        }
    }
}

void acceptFunc(int fd, TimeStamp receivedTime)
{
    SockAddrInet clientAddr;
    socklen_t len;
    int clientFd = ::accept(fd, clientAddr.data(), &len);
    if (clientFd < 0) {
        return;
    }

    if (getpeername(clientFd, clientAddr.data(), &len) != 0) {
        return;
    }

    LOG_INFO << receivedTime.toFormattedString() << " accept client: fd(" << clientFd << "), addr("
             << clientAddr.toString() << ").";

    auto echoChannel = std::make_shared<EventChannel>(clientFd, g_loop);
    echoChannel->setReadCallback([clientFd](TimeStamp receivedTime) { echoFunc(clientFd, receivedTime); });
    g_loop->updateChannel(echoChannel);
}

int main()
{
    LOG_INFO << "EventLoopThreadTest started.";

    EventLoopThread eventLoopThread("EventLoopThreadTest");
    g_loop = eventLoopThread.start();
    if (g_loop == nullptr) {
        LOG_ERROR << "Failed to create EventLoopThread!";
        return -1;
    }

    // tick every 5s, delayed 1s.
    g_loop->runEvery(
        []() { LOG_INFO << "main tick."; }, 5000 * MICRO_SECS_PER_MILLISECOND, 1000 * MICRO_SECS_PER_MILLISECOND);

    // run after 5s.
    g_loop->runAfter([]() { LOG_INFO << "hahahahaha."; }, 5000 * MICRO_SECS_PER_MILLISECOND);

    // init socket
    auto fd = ::socket(AF_INET, SOCK_STREAM, 0);
    SockAddrInet secvAddr(INADDR_ANY, 4567);
    int ret = TEMP_FAILURE_RETRY(::bind(fd, secvAddr.get(), secvAddr.len()));
    if (ret < 0) {
        LOG_INFO << "bind: " << fd << "err!";
        return -1;
    }
    listen(fd, 5);

    auto acceptChannel = std::make_shared<EventChannel>(fd, g_loop);
    acceptChannel->setReadCallback([fd](TimeStamp receivedTime) { acceptFunc(fd, receivedTime); });
    g_loop->updateChannel(acceptChannel);

    while (true) {
        std::string tmp;
        getline(std::cin, tmp);
        if (tmp.size() == 1 && std::toupper(tmp[0]) == 'Q') {
            break;
        }
        g_loop->runAfter([]() { LOG_INFO << "sub tick."; }, 100 * MICRO_SECS_PER_MILLISECOND);
    }

    return 0;
}
