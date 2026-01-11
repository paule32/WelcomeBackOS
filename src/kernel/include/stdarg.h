// ----------------------------------------------------------------------------
// \file  stdarg.h
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
#ifndef __STDARG_H__
#define __STDARG_H__

typedef __builtin_va_list va_list;

# define va_start(ap, X)  __builtin_va_start(ap, X)
# define va_arg(ap, type) __builtin_va_arg(ap, type)
# define va_end(ap)       __builtin_va_end(ap)

#endif  // __STDARG_H__
