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
TEST(BufferTest, BufferInitTest)
{
    Buffer buf;
    EXPECT_EQ(buf.bytesReadable(), 0);
    EXPECT_EQ(buf.bytesPrepend(), Buffer::PREPEND_SIZE);
    EXPECT_EQ(buf.bytesWritable(), Buffer::INITIAL_SIZE);
    EXPECT_EQ(buf.begin() + Buffer::PREPEND_SIZE, buf.peek());
    EXPECT_EQ(buf.peek(), buf.writeBegin());
}

TEST(BufferTest, BufferAppendTest)
{
    Buffer buf;
    string tmp(10, 'y');
    buf.append(tmp.c_str(), tmp.size());
    EXPECT_EQ(buf.bytesReadable(), tmp.size());
    tmp = buf.readAll();
    EXPECT_EQ(buf.bytesReadable(), 0);
}
} // namespace conn
} // namespace wind
