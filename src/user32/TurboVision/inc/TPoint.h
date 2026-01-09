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

namespace tvision {
    class TPoint {
    public:
        TPoint& operator += (const TPoint& adder );
        TPoint& operator -= (const TPoint& subber);
        
        friend TPoint operator - (const TPoint& one, const TPoint& two);
        friend TPoint operator + (const TPoint& one, const TPoint& two);
        
        friend int operator == (const TPoint& one, const TPoint& two);
        friend int operator != (const TPoint& one, const TPoint& two);

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

    inline TPoint operator - ( const TPoint& one, const TPoint& two ) {
        TPoint result;
        
        result.x = one.x - two.x;
        result.y = one.y - two.y;
        
        return result;
    }

    inline TPoint operator + ( const TPoint& one, const TPoint& two ) {
        TPoint result;
        
        result.x = one.x + two.x;
        result.y = one.y + two.y;
        
        return result;
    }

    inline int operator == ( const TPoint& one, const TPoint& two ) {
        return one.x == two.x && one.y == two.y;
    }

    inline int operator!= ( const TPoint& one, const TPoint& two ) {
        return one.x != two.x || one.y != two.y;
    }

}   // namespace: std
#endif  // __TVISION_TPOINT_H__
