// ----------------------------------------------------------------------------
// \file  isr.h
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
#pragma once
# include "stdint.h"

typedef struct regs {
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
}   regs_t;

//extern void isr_init(void);
extern void isr_handler(regs_t* r);
