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

#include "Socket.h"

#include <iostream>

using namespace wind;
using namespace wind::base;
using namespace wind::conn;

int main()
{
    Socket sock(sockets::createNonBlockSocketOrDie(AF_INET, SOCK_STREAM, 0));
    SockAddrInet addr1(4587);
    sock.bindOrDie(addr1);
    SockAddrInet addr2 = sockets::getLocalAddrInet(sock.fd());
    std::cout << addr1.toString() << std::endl;
    std::cout << addr2.toString() << std::endl;

    Socket sock1(sockets::createNonBlockSocketOrDie(AF_UNIX, SOCK_STREAM, 0));
    SockAddrUnix addr3("/tmp/yxf");
    sock1.bindOrDie(addr3);
    SockAddrUnix addr4 = sockets::getLocalAddrUnix(sock1.fd());
    std::cout << addr3.toString() << std::endl;
    std::cout << addr4.toString() << std::endl;
}
