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

#include "EPoller.h"
#include "Channel.h"

#include <memory>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>

using namespace wind;

void echoFunc(int fd)
{
    char buf[1024] = {0};
    int len = read(fd, buf, sizeof(buf));
    write(fd, buf, len);
}

void acceptFunc(int fd, EPoller *poller)
{
    sockaddr_in clientSockAddr = {};
    socklen_t len;
    int clientFd = ::accept(fd, (sockaddr *)(&clientSockAddr), &len);
    std::cout << "accept client: " << clientFd << std::endl;

    auto channel = std::make_shared<Channel>(clientFd, [clientFd]() {
        echoFunc(clientFd);
    });
    poller->updateChannel(channel);
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

    EPoller epoller;
    auto channel = std::make_shared<Channel>(fd, [fd, &epoller]() {
        acceptFunc(fd, &epoller);
    });
    epoller.updateChannel(channel);

    while (true) {
        std::vector<std::weak_ptr<Channel>> activeChannels;
        epoller.pollOnce(100, activeChannels);
        for (const auto &channel : activeChannels) {
            auto channelSp = channel.lock();
            if (channelSp != nullptr) {
                channelSp->dispatch();
            }
        }
    };

    return 0;
}
