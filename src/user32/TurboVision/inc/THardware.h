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

namespace tvision {
    class THardwareInfo {
    public:
         THardwareInfo();
        ~THardwareInfo();

    // screen functions
        static uint16_t getScreenRows(void);
        static uint16_t getScreenCols(void);

        static uint16_t* getColorAddr(uint16_t);
        static uint16_t* getMonoAddr (uint16_t);
        
        void cursorOn (void);
        void cursorOff(void);
        
        static uint32_t getTickCount  (void);
        static uint32_t getTickCountMs(void);
    private:
        static uint16_t colorSel;
        static uint16_t monoSel ;
        static uint16_t biosSel ;
    };

    inline uint32_t THardwareInfo::getTickCount() {
        return *(uint32_t *) MAKELONG( biosSel, 0x6C );
    }
    inline uint32_t THardwareInfo::getTickCountMs() {
        return getTickCount() * 55;
    }

    inline uint16_t THardwareInfo::getScreenRows(void) {
        return 25;
    }
    inline uint16_t THardwareInfo::getScreenCols(void) {
        return 80;
    }

    inline void THardwareInfo::cursorOn(void) {
        
    }
    inline void THardwareInfo::cursorOff(void) {
        
    }

    inline uint16_t* THardwareInfo::getColorAddr(uint16_t offset) {
        return (uint16_t *) MAKELONG( colorSel, offset );
    }
    inline uint16_t* THardwareInfo::getMonoAddr(uint16_t offset) {
        return (uint16_t *) MAKELONG( monoSel, offset );
    }
}   // namespace: tvision
#endif  // __HARDWARE_H__

