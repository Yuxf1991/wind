//
// Created by tracy on 2020/10/13.
//

#ifndef WIND_UTILS_H
#define WIND_UTILS_H

// For checking pthread functions' return value.
#include "private/PThreadCheck.h"

// Enable thread safety attributes only with clang.
#if defined(__clang__) && (!defined(SWIG))
#define THREAD_ANNOTATION_ATTRIBUTE__(x)   __attribute__((x))
#else
#define THREAD_ANNOTATION_ATTRIBUTE__(x)   // do nothing
#endif
#include "private/ClangTSA.h"

#define UNUSED_VALUE(x) (void)(x);

// For checking syscalls' return value.
#include "private/SysCallCheck.h"

namespace wind {
    namespace utils {
        struct AutoFreeBuf {
            AutoFreeBuf(int size){
                if (size > 0) {
                    m_buf = new char[size];
                }
            }
            ~AutoFreeBuf() {
                if (m_buf) {
                    delete[] m_buf;
                    m_buf = nullptr;
                }
            }
            [[nodiscard]] char *buf() const { return m_buf; }

            char *m_buf = nullptr;
        };
    }
}

#endif //WIND_UTILS_H
