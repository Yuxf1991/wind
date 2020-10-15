//
// Created by tracy on 2020/9/20.
//

#ifndef WIND_PTHREADCHECK_H
#define WIND_PTHREADCHECK_H

#ifdef CHECK_PTHREAD_RETURN_VALUE

#ifdef NDEBUG
__BEGIN_DECLS
extern void __assert_perror_fail (int errnum,
                                  const char *file,
                                  unsigned int line,
                                  const char *function)
    noexcept __attribute__ ((__noreturn__));
__END_DECLS
#endif

#define PTHREAD_CHECK(ret)                                              \
    do {                                                                \
        __typeof__ (ret) errnum = (ret);                                \
        if (__builtin_expect(errnum != 0, false)) {                     \
            __assert_perror_fail (errnum, __FILE__, __LINE__, __func__);\
        }                                                               \
    } while(0)
#else // CHECK_PTHREAD_RETURN_VALUE

#define PTHREAD_CHECK(ret)              \
    do {                                \
        __typeof__ (ret) errnum = (ret);\
        assert(errnum == 0);            \
        (void) errnum;                  \
    } while(0)

#endif // CHECK_PTHREAD_RETURN_VALUE

#endif//WIND_PTHREADCHECK_H
