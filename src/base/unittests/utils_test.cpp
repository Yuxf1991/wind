//
// Created by tracy on 2020/10/15.
//

#include <sys/socket.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "Utils.h"

void testSysCheck() {
    int fd;
    SYSCALL_CHECK(fd = socket(AF_LOCAL, SOCK_STREAM, 0)); // should success.
    SYSCALL_CHECK(close(fd)); // should success
    SYSCALL_CHECK(fd = socket(23, SOCK_STREAM, 7)); // should fail
}

int main() {
    testSysCheck();
    return 0;
}