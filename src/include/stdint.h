#pragma once

# define TRUE  1
# define FALSE 0

typedef signed int     int32_t;
typedef signed short   int16_t;
typedef signed char     int8_t;

typedef unsigned char  UCHAR;
typedef unsigned int   UINT;
typedef unsigned short USHORT;
typedef unsigned long  ULONG;

typedef unsigned int   uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char   uint8_t;

typedef uint32_t* uintptr_t;
typedef uint32_t  size_t;

# undef false
# undef true

# define false 0
# define true  1

# define NULL (void*)0
