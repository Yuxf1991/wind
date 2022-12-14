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

#pragma once

#include <netinet/in.h>
#include <stddef.h>
#include <sys/un.h>

#include "base/Types.h"

namespace wind {
namespace conn {
namespace sockets {
bool fromIpPortV4(const std::string &ip, in_port_t port, sockaddr_in *outAddr);
bool fromIpPortV6(const std::string &ip, in_port_t port, sockaddr_in6 *outAddr);
std::string toIpStringV4(const sockaddr_in &addrV4);
std::string toIpStringV6(const sockaddr_in6 &addrV6);
constexpr char SOCKET_LOCK_SUFFIX[] = ".lock";
} // namespace sockets

// copyable
template <typename SockType>
class SockAddr {
public:
    const sockaddr *getSockAddr() const
    {
        return constObjPtr()->SockType::getSockAddr();
    }
    sockaddr *data()
    {
        return objPtr()->SockType::data();
    }
    socklen_t len() const
    {
        return constObjPtr()->SockType::len();
    }
    socklen_t capacity() const
    {
        return constObjPtr()->SockType::capacity();
    }
    sa_family_t family() const
    {
        return constObjPtr()->SockType::family();
    }
    std::string toString() const
    {
        return constObjPtr()->SockType::toString();
    }

private:
    const SockType *constObjPtr() const
    {
        return static_cast<const SockType *>(this);
    }
    SockType *objPtr() const
    {
        return const_cast<SockType *>(constObjPtr());
    }
};

// copyable
// For both ipv4 and ipv6
class SockAddrInet : public SockAddr<SockAddrInet> {
public:
    // Mostly for listening address.
    explicit SockAddrInet(in_port_t port = 0, bool ipv6 = false, bool onlyLoopBack = false) noexcept;
    SockAddrInet(const std::string &ip, in_port_t port, bool ipv6 = false);
    ~SockAddrInet() noexcept = default;

    const sockaddr *getSockAddr() const
    {
        return reinterpret_cast<const sockaddr *>(&addr_);
    }
    sockaddr *data()
    {
        return const_cast<sockaddr *>(getSockAddr());
    }
    socklen_t len() const
    {
        return static_cast<socklen_t>(sizeof(addr_));
    }
    socklen_t capacity() const
    {
        return len();
    }
    sa_family_t family() const
    {
        return getSockAddr()->sa_family;
    }
    std::string ip() const;
    in_port_t port() const;
    std::string portString() const;
    std::string toString() const;

    bool operator==(const SockAddrInet &other) const
    {
        return family() == other.family() && ip() == other.ip() && port() == other.port();
    }

private:
    union Addr {
        sockaddr_in v4;
        sockaddr_in6 v6;
    } addr_{};
};

// copyable
// For unix domain socket
class SockAddrUnix : public SockAddr<SockAddrUnix> {
public:
    SockAddrUnix();
    explicit SockAddrUnix(const std::string &path);
    ~SockAddrUnix() noexcept = default;

    const sockaddr *getSockAddr() const
    {
        return reinterpret_cast<const sockaddr *>(&addr_);
    }
    sockaddr *data()
    {
        return const_cast<sockaddr *>(getSockAddr());
    }
    socklen_t len() const
    {
        return static_cast<socklen_t>(offsetof(sockaddr_un, sun_path) + strlen(addr_.sun_path));
    }
    socklen_t capacity() const
    {
        return static_cast<socklen_t>(sizeof(addr_));
    }
    sa_family_t family() const
    {
        return getSockAddr()->sa_family;
    }
    std::string toString() const
    {
        return addr_.sun_path;
    }
    bool operator==(const SockAddrUnix &other) const
    {
        return family() == other.family() && toString() == other.toString();
    }

private:
    sockaddr_un addr_{};
};
} // namespace conn
} // namespace wind
