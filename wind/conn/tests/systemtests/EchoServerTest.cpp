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

#define LOG_TAG "EchoServerTest"

#include "TcpServer.h"

using namespace wind;
using namespace wind::base;
using namespace wind::conn;

class EchoServer : public TcpServer {
public:
    EchoServer(EventLoop *loop, const SockAddrInet &listenAddr, std::string name)
        : TcpServer(loop, listenAddr, std::move(name))
    {
        setConnectionCallback([this](const TcpConnectionPtr &conn) { onConnection(conn); });
        setMessageCallback(
            [this](const TcpConnectionPtr &conn, TimeStamp receivedTime) { onMessage(conn, receivedTime); });
    }
    ~EchoServer() noexcept override = default;

private:
    void onConnection(const TcpConnectionPtr &conn)
    {
        LOG_INFO << "Connection " << conn->name() << " " << conn->stateString() << ".";
    }
    void onMessage(const TcpConnectionPtr &conn, TimeStamp receivedTime)
    {
        Buffer *buffer = conn->recvBuffer();
        std::string message = buffer->readAll();
        LOG_INFO << "Connection " << conn->name() << " received message: " << message << " at "
                 << receivedTime.toFormattedString();
        conn->send(message);
    }
};

int main()
{
    LOG_INFO << "EchoServerTest started.";
    EventLoop loop;
    SockAddrInet listenAddr(4567);
    EchoServer server(&loop, listenAddr, "EchoServer");
    server.setThreadNum(4);
    server.start();
    loop.start();

    return 0;
}
