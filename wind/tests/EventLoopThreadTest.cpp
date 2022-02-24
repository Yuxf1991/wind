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

#include "Socket.h"

#include <iostream>

using namespace wind;

class Connection {
public:
    Connection(int sockfd, EventLoop *loop)
        : sock_(sockfd), loop_(loop), channel_(std::make_shared<EventChannel>(sockfd, loop_))
    {}
    ~Connection() noexcept
    {
        channel_->disableAll();
    }

    std::shared_ptr<EventChannel> getChannel() const { return channel_; }

private:
    Socket sock_;
    EventLoop *loop_;
    std::shared_ptr<EventChannel> channel_;
};

class EchoServer {
public:
    EchoServer()
        : servSock_(Socket::createNonBlockSocket(AF_INET, SOCK_STREAM, 0)), loopThread_("EchoServerEventLoopThread")
    {}

    ~EchoServer() noexcept
    {
        acceptChannel_->disableAll();
    }

    void run(uint16_t port)
    {
        loop_ = loopThread_.start();
        if (loop_ == nullptr) {
            LOG_ERROR << "Failed to create EventLoopThread!";
            return;
        }

        // tick every 5s, delayed 1s.
        loop_->runEvery(
            []() { LOG_INFO << "main tick."; }, 5000 * MICRO_SECS_PER_MILLISECOND, 1000 * MICRO_SECS_PER_MILLISECOND);

        // run after 5s.
        loop_->runAfter([]() { LOG_INFO << "hahahahaha."; }, 5000 * MICRO_SECS_PER_MILLISECOND);

        // init socket
        SockAddrInet secvAddr(INADDR_ANY, port);
        servSock_.bind(secvAddr);
        servSock_.listen();

        acceptChannel_ = std::make_shared<EventChannel>(servSock_.fd(), loop_);
        acceptChannel_->setReadCallback([this](TimeStamp receivedTime) { acceptFunc(receivedTime); });
        acceptChannel_->enableReading();

        while (true) {
            std::string tmp;
            getline(std::cin, tmp);
            if (tmp.size() == 1 && std::toupper(tmp[0]) == 'Q') {
                break;
            }
            loop_->runAfter([]() { LOG_INFO << "sub tick."; }, 100 * MICRO_SECS_PER_MILLISECOND);
        }
    }

private:
    void acceptFunc(TimeStamp receivedTime)
    {
        SockAddrInet clientAddr;
        int clientFd = servSock_.accept(clientAddr);
        LOG_INFO << receivedTime.toFormattedString() << " accept client: fd(" << clientFd << "), addr("
                 << clientAddr.toString() << ").";

        auto newConn = std::make_shared<Connection>(clientFd, loop_);
        auto channel = newConn->getChannel();
        channel->setReadCallback([this, clientFd](TimeStamp receivedTime) { echoFunc(clientFd, receivedTime); });
        channel->enableReading();
        clients_[clientFd] = newConn;
    }

    void echoFunc(int fd, TimeStamp receivedTime)
    {
        char buf[1024] = {0};
        int len = TEMP_FAILURE_RETRY(read(fd, buf, sizeof(buf)));
        if (len < 0) {
            LOG_INFO << " recv msg err from client " << fd << ": " << strerror(errno);
        } else if (len == 0) {
            LOG_INFO << receivedTime.toFormattedString() << " client " << fd << " closed, remove this conn.";
            removeClient(fd);
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

    void removeClient(int fd) { clients_.erase(fd); }

    Socket servSock_;
    EventLoopThread loopThread_;
    EventLoop *loop_ = nullptr;

    std::shared_ptr<EventChannel> acceptChannel_;
    std::unordered_map<int, std::shared_ptr<Connection>> clients_;
};

int main()
{
    LOG_INFO << "EventLoopThreadTest started.";

    EchoServer server;
    server.run(5679);

    return 0;
}
