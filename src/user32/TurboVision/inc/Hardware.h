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
    THardwareInfo() noexcept;
    ~THardwareInfo();

// screen functions
    static uint16_t getScreenRows(void) noexcept;
    static uint16_t getScreenCols(void) noexcept;

    static uint16_t* getColorAddr(void) noexcept;
    static uint16_t* getMonoAddr (void) noexcept;
    
    void cursorOn (void) noexcept;
    void cursorOff(void) noexcept;
    
    static uint32_t getTickCount(void) noexcept;
    static uint32_t getTickCountMs(void) noexcept;
private:
    static uint16_t colorSel;
    static uint16_t monoSel ;
    static uint16_t biosSel ;
};

inline uint32_t THardwareInfo::getTickCount() noexcept {
    return *(uint32_t *) MAKELONG( biosSel, 0x6C );
}
inline uint32_t THardwareInfo::getTickCountMs() noexcept {
    return getTickCount() * 55;
}

inline uint16_t THardwareInfo::getScreenRows(void) noexcept
{
    return 25;
}
inline uint16_t THardwareInfo::getScreenCols(void) noexcept
{
    return 80;
}

inline void THardwareInfo::cursorOn(void) noexcept {
    
}
inline void THardwareInfo::cursorOff(void) noexcept {
    
}

inline uint16_t* THardwareInfo::getColorAddr(void) {
    return (uint16_t*)colorSel;
}
inline uint16_t* THardwareInfo::getMonoAddr(void) {
    return (uint16_t*)monoSel;
}

#endif  // __HARDWARE_H__

