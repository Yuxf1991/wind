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

#include "Buffer.h"
#include <gtest/gtest.h>

namespace wind {
namespace conn {
TEST(BufferTest, BufferInit1Test)
{
    Buffer buf;
    EXPECT_EQ(buf.bytesReadable(), 0);
    EXPECT_EQ(buf.prependBytes(), Buffer::PREPEND_SIZE);
    EXPECT_EQ(buf.bytesWritable(), Buffer::INITIAL_SIZE);
    EXPECT_EQ(buf.begin() + Buffer::PREPEND_SIZE, buf.peek());
    EXPECT_EQ(buf.peek(), buf.writeBegin());
}

TEST(BufferTest, BufferInit2Test)
{
    Buffer buf(8, 0);
    EXPECT_EQ(buf.bytesReadable(), 0);
    EXPECT_EQ(buf.prependBytes(), 0);
    EXPECT_EQ(buf.bytesWritable(), 8);
    EXPECT_EQ(buf.begin(), buf.peek());
    EXPECT_EQ(buf.peek(), buf.writeBegin());
}

TEST(BufferTest, BufferAppendTest)
{
    Buffer buf;
    std::string tmp(10, 'y');
    buf.append(tmp.c_str(), tmp.size());
    EXPECT_EQ(buf.bytesReadable(), tmp.size());
    tmp = buf.readAll();
    EXPECT_EQ(buf.bytesReadable(), 0);
}

TEST(BufferTest, BufferAppendAutoGrowTest)
{
    Buffer buf;
    std::string tmp(2048, 'y');
    buf.append(tmp.c_str(), tmp.size());
    EXPECT_EQ(buf.bytesReadable(), tmp.size());
    tmp = buf.readAll();
    EXPECT_EQ(buf.bytesReadable(), 0);
}

TEST(BufferTest, BufferReadWriteIntegerTest)
{
    Buffer buf;
    buf.writeInt8(-1);
    EXPECT_EQ(buf.bytesReadable(), 1);
    EXPECT_EQ(buf.peekInt8(), -1);
    int8_t int8Read = buf.readInt8();
    EXPECT_EQ(int8Read, -1);
    EXPECT_EQ(buf.bytesReadable(), 0);

    buf.writeUInt8(UINT8_MAX);
    EXPECT_EQ(buf.bytesReadable(), 1);
    EXPECT_EQ(buf.peekUInt8(), UINT8_MAX);
    uint8_t uint8Read = buf.readUInt8();
    EXPECT_EQ(uint8Read, UINT8_MAX);
    EXPECT_EQ(buf.bytesReadable(), 0);

    buf.writeInt16(-1);
    EXPECT_EQ(buf.bytesReadable(), 2);
    EXPECT_EQ(buf.peekInt16(), -1);
    int16_t int16Read = buf.readInt16();
    EXPECT_EQ(int16Read, -1);
    EXPECT_EQ(buf.bytesReadable(), 0);

    buf.writeUInt16(UINT16_MAX);
    EXPECT_EQ(buf.bytesReadable(), 2);
    EXPECT_EQ(buf.peekUInt16(), UINT16_MAX);
    uint16_t uint16Read = buf.readUInt16();
    EXPECT_EQ(uint16Read, UINT16_MAX);
    EXPECT_EQ(buf.bytesReadable(), 0);

    buf.writeInt32(-1);
    EXPECT_EQ(buf.bytesReadable(), 4);
    EXPECT_EQ(buf.peekInt32(), -1);
    int32_t int32Read = buf.readInt32();
    EXPECT_EQ(int32Read, -1);
    EXPECT_EQ(buf.bytesReadable(), 0);

    buf.writeUInt32(UINT32_MAX);
    EXPECT_EQ(buf.bytesReadable(), 4);
    EXPECT_EQ(buf.peekUInt32(), UINT32_MAX);
    uint32_t uint32Read = buf.readUInt32();
    EXPECT_EQ(uint32Read, UINT32_MAX);
    EXPECT_EQ(buf.bytesReadable(), 0);

    buf.writeInt64(-1);
    EXPECT_EQ(buf.bytesReadable(), 8);
    EXPECT_EQ(buf.peekInt64(), -1);
    int64_t int64Read = buf.readInt64();
    EXPECT_EQ(int64Read, -1);
    EXPECT_EQ(buf.bytesReadable(), 0);

    buf.writeUInt64(UINT64_MAX);
    EXPECT_EQ(buf.bytesReadable(), 8);
    EXPECT_EQ(buf.peekUInt64(), UINT64_MAX);
    uint64_t uint64Read = buf.readUInt64();
    EXPECT_EQ(uint64Read, UINT64_MAX);
    EXPECT_EQ(buf.bytesReadable(), 0);

    buf.writeInt8(INT8_MAX);
    buf.writeUInt8(UINT8_MAX);
    buf.writeInt16(INT16_MAX);
    buf.writeUInt16(UINT16_MAX);
    buf.writeInt32(INT32_MAX);
    buf.writeUInt32(UINT32_MAX);
    buf.writeInt64(INT64_MAX);
    buf.writeUInt64(UINT64_MAX);
    EXPECT_EQ(buf.bytesReadable(), 30); // 2 + 4 + 8 + 16 bytes
    EXPECT_EQ(buf.readInt8(), INT8_MAX);
    EXPECT_EQ(buf.readUInt8(), UINT8_MAX);
    EXPECT_EQ(buf.readInt16(), INT16_MAX);
    EXPECT_EQ(buf.readUInt16(), UINT16_MAX);
    EXPECT_EQ(buf.readInt32(), INT32_MAX);
    EXPECT_EQ(buf.readUInt32(), UINT32_MAX);
    EXPECT_EQ(buf.readInt64(), INT64_MAX);
    EXPECT_EQ(buf.readUInt64(), UINT64_MAX);
    EXPECT_EQ(buf.bytesReadable(), 0);
}

TEST(BufferTest, BufferMixReadWriteTest)
{
    Buffer buf;
    std::string tmp = "Hello, wind!";
    buf.writeInt32(static_cast<int32_t>(tmp.size()));
    buf.append(tmp.c_str(), tmp.size());
    EXPECT_EQ(buf.bytesReadable(), sizeof(int32_t) + tmp.size());
    int32_t strSize = buf.readInt32();
    EXPECT_EQ(strSize, static_cast<int32_t>(tmp.size()));
    std::string str = buf.read(strSize);
    EXPECT_EQ(str, tmp);
    EXPECT_EQ(buf.bytesReadable(), 0);
}
} // namespace conn
} // namespace wind
