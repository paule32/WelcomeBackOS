// ----------------------------------------------------------------------------
// \file  syscall.h
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
#ifndef SYSCALL_H
#define SYSCALL_H

# include "stdint.h"
# include "isr.h"

//extern void syscall_init(void);           // Initialisiert die Syscall-Schicht
extern void syscall_dispatch(regs_t* r);  // Dispatcher für int 0x80

typedef int32_t (*syscall_fn_t)(regs_t* r);

static int32_t sys_print     (regs_t* tf);
static int32_t sys_alloc     (regs_t* tf);
static int32_t sys_free      (regs_t* tf);

#define KAPI_MAGIC 0x4B415049u

extern syscall_fn_t g_syscalls[];

typedef struct KApi_v1 {
    uint32_t magic;
    uint16_t version;
    uint16_t size;
    uint32_t flags;

    uint32_t sys_print;
    uint32_t sys_alloc;
    uint32_t sys_free;
} KApi_v1;

enum {
    SYS_PRINT      = 1,
    SYS_ALLOC      = 2,
    SYS_FREE       = 3,
    SYS_MAX
};

// ein Beispiel-Syscall: SYSCALL_PUTCHAR (ID 1)
#define SYSCALL_PUTCHAR  1

#endif
