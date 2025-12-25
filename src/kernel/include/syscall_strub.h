#ifndef __SYSCALL_STUB_H__
#define __SYSCALL_STUB_H__

# include "stdint.h"

static inline int32_t __syscall3(uint32_t n, uint32_t a1, uint32_t a2, uint32_t a3) {
    int32_t ret;
    __asm__ volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(n), "b"(a1), "c"(a2), "d"(a3)
        : "memory"
    );
    return ret;
}

static inline int32_t __syscall1(uint32_t n, uint32_t a1) {
    int32_t ret;
    __asm__ volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(n), "b"(a1)
        : "memory"
    );
    return ret;
}

#endif  // __SYSCALL_STUB_H__
