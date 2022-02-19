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

#include "SockAddrInet.h"

#include <arpa/inet.h>

#include "Log.h"

namespace wind {
SockAddrInet::SockAddrInet() : SockAddrInet(INADDR_ANY, 0) {}

SockAddrInet::SockAddrInet(uint32_t ip, uint16_t port)
{
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = htonl(ip);
    addr_.sin_port = htons(port);
}

SockAddrInet::SockAddrInet(const string &ip, uint16_t port)
{
    addr_.sin_family = AF_INET;
    int ret = inet_aton(ip.c_str(), &addr_.sin_addr);
    LOG_FATAL_IF(ret == 0) << "The IP(" << ip << ") is not valid!";
    addr_.sin_port = htons(port);
}

string SockAddrInet::toString() const
{
    string res;
    res += inet_ntoa(addr_.sin_addr);
    res += ":";
    res += std::to_string(ntohs(addr_.sin_port));
    return res;
}
} // namespace wind
