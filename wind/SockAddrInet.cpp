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

#ifdef ENABLE_EXCEPTION
#include <exception>
#endif

#include "Log.h"

namespace wind {
namespace sockets {
bool fromIpPortV4(const string &ip, in_port_t port, sockaddr_in *outAddr)
{
    ASSERT(outAddr != nullptr);
    outAddr->sin_family = AF_INET;
    outAddr->sin_port = ::htons(port);
    if (::inet_pton(AF_INET, ip.c_str(), &outAddr->sin_addr) <= 0) {
        LOG_ERROR << "fromIpPortV4 error for ip: " << ip << "!";
        return false;
    }

    return true;
}

bool fromIpPortV6(const string &ip, in_port_t port, sockaddr_in6 *outAddr)
{
    ASSERT(outAddr != nullptr);
    outAddr->sin6_family = AF_INET6;
    outAddr->sin6_port = ::htons(port);
    if (::inet_pton(AF_INET6, ip.c_str(), &outAddr->sin6_addr) <= 0) {
        LOG_ERROR << "fromIpPortV6 error for ip: " << ip << "!";
        return false;
    }

    return true;
}

string toIpStringV4(const sockaddr_in &addrV4)
{
    char buf[INET_ADDRSTRLEN];
    (void)::inet_ntop(AF_INET, &addrV4.sin_addr, buf, static_cast<socklen_t>(sizeof(buf)));
    return buf;
}

string toIpStringV6(const sockaddr_in6 &addrV6)
{
    char buf[INET6_ADDRSTRLEN];
    (void)::inet_ntop(AF_INET6, &addrV6.sin6_addr, buf, static_cast<socklen_t>(sizeof(buf)));
    return buf;
}
} // namespace sockets

SockAddrInet::SockAddrInet(in_port_t port, bool ipv6, bool onlyLoopBack) noexcept
{
    utils::memZero(&addr_, sizeof(addr_));
    if (ipv6) {
        addr_.v6.sin6_family = AF_INET6;
        addr_.v6.sin6_port = ::htons(port);
        addr_.v6.sin6_addr = (onlyLoopBack ? in6addr_loopback : in6addr_any);
    } else {
        addr_.v4.sin_family = AF_INET;
        addr_.v4.sin_port = ::htons(port);
        addr_.v4.sin_addr.s_addr = (onlyLoopBack ? ::htonl(INADDR_LOOPBACK) : ::htonl(INADDR_ANY));
    }
}

SockAddrInet::SockAddrInet(const string &ip, in_port_t port, bool ipv6)
{
    utils::memZero(&addr_, sizeof(addr_));
    if (ipv6) {
#ifdef ENABLE_EXCEPTION
        if (!sockets::fromIpPortV6(ip, port, &addr_.v6)) {
            throw std::invalid_argument("Invalid ip: " + ip + "!");
        }
#else
        (void)sockets::fromIpPortV6(ip, port, &addr_.v6);
#endif
    } else {
#ifdef ENABLE_EXCEPTION
        if (!sockets::fromIpPortV4(ip, port, &addr_.v4)) {
            throw std::invalid_argument("Invalid ip: " + ip + "!");
        }
#else
        (void)sockets::fromIpPortV4(ip, port, &addr_.v4);
#endif
    }
}

string SockAddrInet::ip() const
{
    return family() == AF_INET ? sockets::toIpStringV4(addr_.v4) : sockets::toIpStringV6(addr_.v6);
}

in_port_t SockAddrInet::port() const
{
    return family() == AF_INET ? addr_.v4.sin_port : addr_.v6.sin6_port;
}

string SockAddrInet::portString() const
{
    return std::to_string(::ntohs(port()));
}

string SockAddrInet::ipPortString() const
{
    return ip() + ":" + portString();
}
} // namespace wind
