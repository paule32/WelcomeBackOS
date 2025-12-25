#ifndef __KERNEL_APU_H__
#define __KERNEL_APU_H__

# include "stdint.h"

int32_t sys_write(const char* s, uint32_t len);
__attribute__((noreturn)) void sys_exit(int32_t code);

#endif  // __KERNEL_APU_H__