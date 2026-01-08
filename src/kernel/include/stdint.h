// ----------------------------------------------------------------------------
// \file  stdint.h
// \note  (c) 2025 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
#ifndef __STDINT_H__
#define __STDINT_H__

# define TRUE  1
# define FALSE 0

typedef signed int     int32_t;
typedef signed short   int16_t;
typedef signed char     int8_t;
// ----------------------------
typedef int32_t        sint32_t;
typedef int16_t        sint16_t;
typedef int8_t         sint8_t;

typedef signed char    SCHAR;
typedef signed int     SINT;
typedef signed short   SSHORT;
typedef signed long    SLONG;
// ----------------------------
typedef unsigned char  UCHAR;
typedef unsigned int   UINT;
typedef unsigned short USHORT;
typedef unsigned long  ULONG;

typedef unsigned int   uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char   uint8_t;

typedef uint32_t*     uintptr_t;
typedef uint32_t      size_t;

typedef uint32_t       DWORD;
typedef int            BOOL;
typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef int            INT;

# undef false
# undef true

# define false 0
# define true  1

# define UINT_MAX 4294967295U

# define NULL (void*)0

#endif  // __STDINT_H__
