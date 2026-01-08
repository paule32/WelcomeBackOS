// ----------------------------------------------------------------------------
// \file  Hardware.h
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
//
// \note  Turbo Vision - Version 2.0
//        Copyright (c) 1994 by Borland International
//        All Rights Reserved.
// ----------------------------------------------------------------------------
#ifndef __HARDWARE_H__
#define __HARDWARE_H__

#if defined( Uses_THardwareInfo ) && !defined( __THardwareInfo )
#define __THardwareInfo
#endif

# include <compat/windows/windows.h>

#if !defined( MAKELONG )
#define MAKELONG(h,l) \
    ((long)(((unsigned)(l)) | (((long)((unsigned)(h))) << 16)))
#endif

class THardwareInfo {
public:
    THardwareInfo();
    ~THardwareInfo();
    
    static uint32_t getTickCount() noexcept;
    static uint32_t getTickCountMs();
};
#endif  // __HARDWARE_H__

