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
#include "base/EventLoop.h"
#include "base/Log.h"

#include "Socket.h"

using namespace wind;
using namespace wind::base;
using namespace wind::conn;

class Connection {
public:
    Connection(int sockFd, EventLoop *loop)
        : sock_(sockFd), loop_(loop), channel_(std::make_shared<EventChannel>(sockFd, loop_))
    {}
    ~Connection() noexcept
    {
        channel_->disableAll();
    }

    std::shared_ptr<EventChannel> getChannel() const
    {
        return channel_;
    }

private:
    Socket sock_;
    EventLoop *loop_;
    std::shared_ptr<EventChannel> channel_;
};

class EchoServer {
public:
    EchoServer() : servSock_(sockets::createNonBlockSocketOrDie(AF_INET, SOCK_STREAM, 0)), loop_() {}

    ~EchoServer() noexcept
    {
        acceptChannel_->disableAll();
    }

    void run(uint16_t port)
    {
        // tick every 5s, delayed 1s.
        loop_.runEvery(
            []() { LOG_INFO << "main tick."; }, 5000 * MICRO_SECS_PER_MILLISECOND, 1000 * MICRO_SECS_PER_MILLISECOND);

        // run after 5s.
        loop_.runAfter([]() { LOG_INFO << "hah."; }, 5000 * MICRO_SECS_PER_MILLISECOND);

        // init socket
        SockAddrInet servAddr(port);
        servSock_.bindOrDie(servAddr);
        servSock_.listenOrDie();

        acceptChannel_ = std::make_shared<EventChannel>(servSock_.fd(), &loop_);
        acceptChannel_->setReadCallback([this](TimeStamp receivedTime) { acceptFunc(receivedTime); });
        acceptChannel_->enableReading();

        loop_.start();
    }

private:
    void acceptFunc(TimeStamp receivedTime)
    {
        SockAddrInet clientAddr;
        int clientFd = servSock_.accept(clientAddr);
        LOG_INFO << receivedTime.toFormattedString() << " accept client: fd(" << clientFd << "), addr("
                 << clientAddr.toString() << ").";

        auto newConn = std::make_shared<Connection>(clientFd, &loop_);
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
            LOG_INFO << " receive msg err from client " << fd << ": " << strerror(errno);
        } else if (len == 0) {
            LOG_INFO << receivedTime.toFormattedString() << " client " << fd << " closed, remove this conn.";
            removeClient(fd);
        } else {
            LOG_INFO << receivedTime.toFormattedString() << " receive msg from client " << fd << ": " << buf;
            int ret = TEMP_FAILURE_RETRY(write(fd, buf, len));
            if (ret < 0) {
                LOG_INFO << receivedTime.toFormattedString() << " send msg err from client " << fd << ": "
                         << strerror(errno);
            } else {
                LOG_INFO << receivedTime.toFormattedString() << " send msg from client " << fd << ": " << buf;
            }
        }
    }

    void removeClient(int fd)
    {
        clients_.erase(fd);
    }

    Socket servSock_;
    EventLoop loop_;

    std::shared_ptr<EventChannel> acceptChannel_;
    std::unordered_map<int, std::shared_ptr<Connection>> clients_;
};

int main()
{
    LOG_INFO << "EventLoopThreadTest started.";

    EchoServer server;
    server.run(4567);

    return 0;
}
