// MIT License

// Copyright (c) 2020 Tracy

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "Buffer.h"
#include "TestHelper.h"

namespace wind {
TEST(BufferTest, InitTest)
{
    WIND_TEST_BEGIN(BufferTest, InitTest);

    // DefaultFixedBuffer initialize state test
    DefaultFixedBuffer buf0;
    EXPECT_EQ(buf0.capacity(), DEFAULT_BUFFER_SIZE);
    EXPECT_EQ(buf0.available(), DEFAULT_BUFFER_SIZE);
    EXPECT_EQ(buf0.curr(), buf0.data());
    EXPECT_EQ(buf0.end(), buf0.data() + DEFAULT_BUFFER_SIZE);
    EXPECT_EQ(buf0.length(), 0);

    FixedSizeBuffer<10> buf1;
    EXPECT_EQ(buf1.capacity(), 10);
    EXPECT_EQ(buf1.available(), 10);
    EXPECT_EQ(buf1.curr(), buf1.data());
    EXPECT_EQ(buf1.end(), buf1.data() + 10);
    EXPECT_EQ(buf1.length(), 0);
}

TEST(BufferTest, AppendAndResetTest)
{
    WIND_TEST_BEGIN(BufferTest, AppendAndResetTest);

    // DefaultFixedBuffer initialize state test
    DefaultFixedBuffer buf0;
    string tmp0("Hello world!");
    buf0.append(tmp0);
    EXPECT_EQ(buf0.capacity(), DEFAULT_BUFFER_SIZE);
    EXPECT_EQ(buf0.length(), tmp0.length());
    EXPECT_EQ(buf0.available(), DEFAULT_BUFFER_SIZE - buf0.length());
    EXPECT_EQ(buf0.curr(), buf0.data() + buf0.length());
    EXPECT_EQ(buf0.end(), buf0.curr() + buf0.available());

    std::cout << "After appending tmp0, the buf0 is " << buf0.toString()
              << std::endl;
    EXPECT_EQ(buf0.toString(), tmp0);

    buf0.reset();
    EXPECT_EQ(buf0.capacity(), DEFAULT_BUFFER_SIZE);
    EXPECT_EQ(buf0.available(), DEFAULT_BUFFER_SIZE);
    EXPECT_EQ(buf0.curr(), buf0.data());
    EXPECT_EQ(buf0.end(), buf0.data() + DEFAULT_BUFFER_SIZE);
    EXPECT_EQ(buf0.length(), 0);

    string tmp1(", my name is robot.");
    buf0.append(tmp0);
    buf0.append(tmp1);
    std::cout << "After appending tmp0 + tmp1, the buf0 is " << buf0.toString()
              << std::endl;
    EXPECT_EQ(buf0.toString(), tmp0 + tmp1);
    EXPECT_EQ(buf0.capacity(), DEFAULT_BUFFER_SIZE);
    EXPECT_EQ(buf0.available(),
              DEFAULT_BUFFER_SIZE - tmp0.length() - tmp1.length());
    EXPECT_EQ(buf0.curr(), buf0.data() + tmp0.length() + tmp1.length());
    EXPECT_EQ(buf0.end(), buf0.data() + DEFAULT_BUFFER_SIZE);
    EXPECT_EQ(buf0.length(), tmp0.length() + tmp1.length());
}

TEST(BufferTest, AppendExceedTest)
{
    WIND_TEST_BEGIN(BufferTest, AppendExceedTest);

    FixedSizeBuffer<10> buf;
    string tmp = "hello world!";
    buf.append(tmp);
    std::cout << "After appending tmp, the buf is " << buf.toString()
              << std::endl;
    EXPECT_EQ(buf.toString(), tmp.substr(0, 10));
    EXPECT_EQ(buf.capacity(), 10);
    EXPECT_EQ(buf.available(), 0);
    EXPECT_EQ(buf.curr(), buf.data() + 10);
    EXPECT_EQ(buf.end(), buf.data() + 10);
    EXPECT_EQ(buf.length(), 10);
}
} // namespace wind
