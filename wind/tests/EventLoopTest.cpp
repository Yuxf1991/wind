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

#define LOG_TAG "EventLoopTest"
#include "EventLoop.h"

#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>

using namespace wind;

std::unique_ptr<EventLoop> g_loop;

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
    sockaddr_in clientSockAddr = {};
    socklen_t len;
    int clientFd = ::accept(fd, (sockaddr *)(&clientSockAddr), &len);
    LOG_INFO << receivedTime.toFormattedString() << " accept client: " << clientFd;

    auto channel = std::make_shared<EventChannel>(clientFd, g_loop.get());
    channel->setReadCallback([clientFd](TimeStamp receivedTime) { echoFunc(clientFd, receivedTime); });
}

int main()
{
    // testChannel may generate a core dump in EventChannel's constructor.
    // auto testChannel = std::make_shared<EventChannel>(2, g_loop.get());

    g_loop = std::make_unique<EventLoop>();

    // tick every 333 ms.
    g_loop->runEvery([]() { LOG_INFO << "tick."; }, 333 * MICRO_SECS_PER_MILLISECOND);

    // init socket
    auto fd = ::socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in sockAddr = {};
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_addr.s_addr = INADDR_ANY;
    sockAddr.sin_port = htons(4567);
    int ret = TEMP_FAILURE_RETRY(::bind(fd, (sockaddr *)(&sockAddr), sizeof(sockAddr)));
    if (ret < 0) {
        LOG_INFO << "bind: " << fd << "err!";
        return -1;
    }
    listen(fd, 5);

    auto acceptChannel = std::make_shared<EventChannel>(fd, g_loop.get());
    acceptChannel->setReadCallback([fd](TimeStamp receivedTime) { acceptFunc(fd, receivedTime); });

    g_loop->run();

    return 0;
}
