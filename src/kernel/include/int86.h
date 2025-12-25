#ifndef __INT86_H__
#define __INT86_H__

# pragma once
# include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

typedef struct REGS16 {
    uint16_t ax, bx, cx, dx;
    uint16_t si, di, bp;
    uint16_t ds, es;     // optional
    uint16_t flags;      // Carry etc.
} REGS16;

int int86(uint8_t intno, const REGS16 *in, REGS16 *out);

#ifdef __cplusplus
};
#endif  // __cplusplus
#endif  // __INT86_H__
