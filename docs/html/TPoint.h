// ----------------------------------------------------------------------------
// \file  TPoint.h
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
//
// \note  Turbo Vision - Version 2.0
//        Copyright (c) 1994 by Borland International
//        All Rights Reserved.
// ----------------------------------------------------------------------------
#ifndef __TVISION_TPOINT_H__
#define __TVISION_TPOINT_H__

# include "stdint.h"

/**
 * @brief Reserviert Speicher aus dem Kernel-Heap.
 * @param size Anzahl Bytes.
 * @return Pointer auf Block oder NULL.
 * @note  Nur im Kernel-Kontext verwenden.
 * @see kfree
 */
namespace tvision {
    class TPoint {
    public:
        TPoint& operator += (const TPoint& adder );
        TPoint& operator -= (const TPoint& subber);
        
        TPoint  operator  - (const TPoint& rhs) const;
        TPoint  operator  + (const TPoint& rhs) const;
        TPoint  operator  * (const TPoint& rhs) const;
        TPoint  operator  / (const TPoint& rhs) const;
        TPoint  operator  ^ (const TPoint& rhs) const;
        
        TPoint& operator  = (const TPoint& src);
        
        bool    operator  < (const TPoint& rhs) const;
        bool    operator  > (const TPoint& rhs) const;
               
        bool    operator == (const TPoint& rhs) const;
        bool    operator != (const TPoint& rhs) const;

        int x, y;
    };
    
    inline TPoint& TPoint::operator += ( const TPoint& adder ) {
        x += adder.x;
        y += adder.y;
        
        return *this;
    }

    inline TPoint& TPoint::operator -= ( const TPoint& subber ) {
        x -= subber.x;
        y -= subber.y;
        
        return *this;
    }
    
    TPoint& TPoint::operator = (const TPoint& src) {
        if (*this != src) {
            x = src.x;
            y = src.y;
        }
        
        return *this;
    }

    inline TPoint TPoint::operator - (const TPoint& rhs) const {
        return {
            x - rhs.x,
            y - rhs.y
        };
    }

    inline TPoint TPoint::operator + (const TPoint& rhs) const {
        return {
            x + rhs.x,
            y + rhs.y
        };
    }
    
    inline TPoint TPoint::operator * (const TPoint& rhs) const {
        return {
            x * rhs.x,
            y * rhs.y
        };
    }

    inline TPoint TPoint::operator / (const TPoint& rhs) const {
        return {
            x / rhs.x,
            y / rhs.y
        };
    }
    
    inline TPoint TPoint::operator ^ ( const TPoint& rhs) const {
        return TPoint {
            x + (rhs.x * rhs.x),
            y + (rhs.y * rhs.y)
        };
    }
    
    inline bool TPoint::operator == (const TPoint& rhs) const {
        return (
            x == rhs.x &&
            y == rhs.y
        );
    }

    inline bool TPoint::operator != (const TPoint& rhs) const {
        return (
            x != rhs.x ||
            y != rhs.y
        );
    }

}   // namespace: std
#endif  // __TVISION_TPOINT_H__
