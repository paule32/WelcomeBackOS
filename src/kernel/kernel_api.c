# include "stdint.h"
# include "kernel_api.h"

int32_t sys_write(const char* s, uint32_t len) {
    return __syscall3(SYS_write, (uint32_t)s, len, 0);
}

void sys_exit(int32_t code) {
    __syscall1(SYS_exit, (uint32_t)code);
    for(;;) { } // noreturn
}

KEXPORT(sys_write);
