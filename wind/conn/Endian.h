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

#include <endian.h>
#include <stdint.h>

namespace wind {
namespace conn {
namespace endian {
// As we all know, network's endian is big endian.

inline uint64_t hostToNet(uint64_t h64)
{
    return htobe64(h64);
}

inline int64_t hostToNet(int64_t h64)
{
    return htobe64(h64);
}

inline uint32_t hostToNet(uint32_t h32)
{
    return htobe32(h32);
}

inline int32_t hostToNet(int32_t h32)
{
    return htobe32(h32);
}

inline uint16_t hostToNet(uint16_t h16)
{
    return htobe16(h16);
}

inline int16_t hostToNet(int16_t h16)
{
    return htobe16(h16);
}

inline uint64_t netToHost(uint64_t be64)
{
    return be64toh(be64);
}

inline int64_t netToHost(int64_t be64)
{
    return be64toh(be64);
}

inline uint32_t netToHost(uint32_t be32)
{
    return be32toh(be32);
}

inline int32_t netToHost(int32_t be32)
{
    return be32toh(be32);
}

inline uint16_t netToHost(uint16_t be16)
{
    return be16toh(be16);
}

inline int16_t netToHost(int16_t be16)
{
    return be16toh(be16);
}
} // namespace endian
} // namespace conn
} // namespace wind
