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

#include "EventPoller.h"

#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>

using namespace wind;

std::unique_ptr<EventPoller> g_poller;

void echoFunc(int fd, TimeStamp receivedTime)
{
    char buf[1024] = {0};
    int len = TEMP_FAILURE_RETRY(read(fd, buf, sizeof(buf)));
    if (len < 0) {
        std::cout << receivedTime.toFormattedString() << " recv msg err: " << strerror(errno) << std::endl;
    } else if (len == 0) {
        std::cout << receivedTime.toFormattedString() << " client " << fd << " closed, remove this channel."
                  << std::endl;
        if (g_poller != nullptr) {
            g_poller->removeChannel(fd);
        }
    } else {
        std::cout << receivedTime.toFormattedString() << " recv msg: " << buf << std::endl;
        int ret = TEMP_FAILURE_RETRY(write(fd, buf, len));
        if (ret < 0) {
            std::cout << receivedTime.toFormattedString() << " send msg err: " << strerror(errno) << std::endl;
        } else {
            std::cout << receivedTime.toFormattedString() << " send msg: " << buf << std::endl;
        }
    }
}

void acceptFunc(int fd, TimeStamp receivedTime)
{
    sockaddr_in clientSockAddr = {};
    socklen_t len;
    int clientFd = ::accept(fd, (sockaddr *)(&clientSockAddr), &len);
    std::cout << receivedTime.toFormattedString() << " accept client: " << clientFd << std::endl;

    auto channel = std::make_shared<EventChannel>(clientFd);
    channel->setReadCallback([clientFd](TimeStamp receivedTime) { echoFunc(clientFd, receivedTime); });
    if (g_poller != nullptr) {
        g_poller->updateChannel(channel);
    }
}

int main()
{
    auto fd = ::socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in sockAddr = {};
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_addr.s_addr = INADDR_ANY;
    sockAddr.sin_port = htons(4567);
    int ret = TEMP_FAILURE_RETRY(::bind(fd, (sockaddr *)(&sockAddr), sizeof(sockAddr)));
    if (ret < 0) {
        std::cout << "bind: " << fd << "err!" << std::endl;
        return -1;
    }

    listen(fd, 5);

    g_poller = std::make_unique<EventPoller>();
    auto channel = std::make_shared<EventChannel>(fd);
    channel->setReadCallback([fd](TimeStamp receivedTime) { acceptFunc(fd, receivedTime); });
    channel->enableReading();
    g_poller->updateChannel(channel);

    while (true) {
        std::vector<std::shared_ptr<EventChannel>> activeChannels;
        auto pollTime = g_poller->pollOnce(activeChannels, -1);
        for (const auto &channel : activeChannels) {
            if (channel != nullptr) {
                channel->handleEvent(pollTime);
            }
        }
    };

    return 0;
}
