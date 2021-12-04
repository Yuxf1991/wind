//
// Created by tracy on 2021/12/4.
//

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