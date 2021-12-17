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

#include "TestHelper.h"
#include "UniqueFd.h"

#include <sys/socket.h>

namespace wind {
TEST(UniqueFdTest, InitTest)
{
    WIND_TEST_BEGIN(UniqueFdTest, InitTest)
    UniqueFd fd(::socket(AF_INET, SOCK_STREAM, 0));
    EXPECT_NE(fd.get(), INVALID_FD());
}

TEST(UniqueFdTest, CastTest)
{
    WIND_TEST_BEGIN(UniqueFdTest, CastTest)
    UniqueFd fd(::socket(AF_INET, SOCK_STREAM, 0));
    std::cout << "Fd is: " << int(fd) << std::endl;
    EXPECT_NE(int(fd), INVALID_FD());
}

TEST(UniqueFdTest, ResetTest)
{
    WIND_TEST_BEGIN(UniqueFdTest, ResetTest)
    UniqueFd fd(::socket(AF_INET, SOCK_STREAM, 0));
    std::cout << "Fd is: " << fd.get() << std::endl;
    std::cout << "---------------------" << std::endl;
    fd.reset(::socket(AF_INET, SOCK_STREAM, 0));
    std::cout << "After reset(socket(...)):\n";
    std::cout << "Fd is: " << fd.get() << std::endl;
    std::cout << "---------------------" << std::endl;
    EXPECT_NE(fd.get(), INVALID_FD());
    fd.reset();
    std::cout << "After reset():\n";
    std::cout << "Fd is: " << fd.get() << std::endl;
}

TEST(UniqueFdTest, ReleaseTest)
{
    WIND_TEST_BEGIN(UniqueFdTest, ReleaseTest)
    UniqueFd fd(::socket(AF_INET, SOCK_STREAM, 0));
    std::cout << "fd is: " << fd.get() << std::endl;
    std::cout << "---------------------" << std::endl;
    int fd2 = fd.release();
    std::cout << "After fd2 = fd.release():\n";
    std::cout << "fd is: " << fd.get() << std::endl;
    std::cout << "fd2 is: " << fd2 << std::endl;
    EXPECT_EQ(fd.get(), INVALID_FD());
    EXPECT_NE(fd2, INVALID_FD());
    TEMP_FAILURE_RETRY(::close(fd2));
}

TEST(UniqueFdTest, MoveTest)
{
    WIND_TEST_BEGIN(UniqueFdTest, MoveTest)
    UniqueFd fd1(::socket(AF_INET, SOCK_STREAM, 0));
    EXPECT_NE(fd1.get(), INVALID_FD());
    std::cout << "fd1 is: " << fd1.get() << std::endl;
    UniqueFd fd2(std::move(fd1));
    EXPECT_EQ(fd1.get(), INVALID_FD());
    EXPECT_NE(fd2.get(), INVALID_FD());
    std::cout << "After fd2(move(fd1)):\n";
    std::cout << "fd1 is: " << fd1.get() << std::endl;
    std::cout << "fd2 is: " << fd2.get() << std::endl;
    std::cout << "---------------------" << std::endl;
    UniqueFd fd3;
    std::cout << "fd3 is: " << fd3.get() << std::endl;
    std::cout << "---------------------" << std::endl;

    fd3 = std::move(fd2);
    EXPECT_EQ(fd2.get(), INVALID_FD());
    EXPECT_NE(fd3.get(), INVALID_FD());
    std::cout << "After fd3 = move(fd2):\n";
    std::cout << "fd2 is: " << fd2.get() << std::endl;
    std::cout << "fd3 is: " << fd3.get() << std::endl;
    std::cout << "---------------------" << std::endl;
}
} // namespace wind