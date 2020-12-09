//
// Created by tracy on 2020/12/9.
//

#include "Utils.h"

#include "errno.h"
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>

namespace wind {
    namespace utils {
        void closeFd(int &fd) {
            if (fd < 0) {
                return;
            }

            SYSCALL_CHECK(::close(fd));

            fd = -1;
        }
    }// namespace utils
}// namespace wind