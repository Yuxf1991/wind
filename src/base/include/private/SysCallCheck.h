//
// Created by tracy on 2020/10/15.
//

#ifndef WIND_SYSCALLCHECK_H
#define WIND_SYSCALLCHECK_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Only check the syscall functions that ret < 0 when failed
// and should abort() immediately
#define SYSCALL_CHECK(call)                                                        \
    do {                                                                           \
        __typeof__(call) errnum = (call);                                          \
        if (__builtin_expect(errnum < 0, false)) {                                 \
            /* TODO: use our logging */                                            \
            wind::utils::AutoFreeBuf buf(strlen(#call) + strlen(__FILE__) + 1024); \
            sprintf(buf.buf(),                                                     \
                    "in file \"%s\", line %d,\n failed system call: %s\n"          \
                    "  errno: %d\n"                                                \
                    "  message",                                                   \
                    __FILE__, __LINE__, #call, errno);                             \
            perror(buf.buf());                                                     \
        }                                                                          \
    } while (0)

#endif//WIND_SYSCALLCHECK_H
