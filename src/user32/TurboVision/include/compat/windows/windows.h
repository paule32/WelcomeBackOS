// ----------------------------------------------------------------------------
// \file  windows.h
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
//
// \note  Turbo Vision - Version 2.0
//        Copyright (c) 1994 by Borland International
//        All Rights Reserved.
// ----------------------------------------------------------------------------
#ifndef __WINDOWS_H__
#define __WINDOWS_H__

# include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void    *PVOID    ;
typedef PVOID    HANDLE   ;
typedef char     CHAR     ;
typedef short    SHORT    ;
typedef int32_t  LONG     ;

# define MAX_PATH 260

#ifndef FALSE
# define FALSE 0
#endif

#ifndef TRUE
# define TRUE 1
#endif

typedef struct _COORD {
    SHORT X;
    SHORT Y;
} COORD;

typedef struct _SMALL_RECT {
    SHORT Left;
    SHORT Top;
    SHORT Right;
    SHORT Bottom;
} SMALL_RECT;


#ifdef __cplusplus
};
#endif

#endif  // __WINDOWS_H__