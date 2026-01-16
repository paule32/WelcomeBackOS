// ----------------------------------------------------------------------------
// \file  TPect.h
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
//
// \note  Turbo Vision - Version 2.0
//        Copyright (c) 1994 by Borland International
//        All Rights Reserved.
// ----------------------------------------------------------------------------
#ifndef __TVISION_TRECT_H__
#define __TVISION_TRECT_H__

# include "stdint.h"
# include "TurboVision/inc/TPoint.h"

namespace tvision {
    #ifndef TVISION_MIN
    #define TVISION_MIN
    template <class T>
    constexpr T min(T a, T b) { return (b < a) ? b : a; }
    #endif  // TVISION_MIN

    #ifndef TVISION_MAX
    #define TVISION_MAX
    template <class T>
    constexpr T max(T a, T b) { return (a < b) ? b : a; }
    #endif  // TVISION_MAX
    
    class TRect {
    public:
        TRect(int ax, int ay, int bx, int by);
        TRect(TPoint p1, TPoint p2);
        TRect();

        TRect& move(int aDX, int aDY);
        TRect& grow(int aDX, int aDY);
        
        TRect& intersect(const TRect& r);
        TRect& Union    (const TRect& r);
        
        bool   contains (const TPoint& p) const;
        
        bool operator == ( const TRect& r ) const;
        bool operator != ( const TRect& r ) const;
        
        bool isEmpty();

        TPoint a, b;
        
        int x,y, w,h;
    };

    inline TRect::TRect( int ax, int ay, int bx, int by) {
        a.x = ax;
        a.y = ay;
        b.x = bx;
        b.y = by;
    }

    inline TRect::TRect( TPoint p1, TPoint p2 ) {
        a = p1;
        b = p2;
    }

    inline TRect::TRect() {
        a.x = a.y = 0;
        b.x = b.y = 0;
    }

    inline TRect& TRect::move( int aDX, int aDY ) {
        a.x += aDX;
        a.y += aDY;
        b.x += aDX;
        b.y += aDY;
        
        return *this;
    }

    inline TRect& TRect::grow( int aDX, int aDY ) {
        a.x -= aDX;
        a.y -= aDY;
        b.x += aDX;
        b.y += aDY;
        
        return *this;
    }

    inline TRect& TRect::intersect( const TRect& r ) {
        a.x = max( a.x, r.a.x );
        a.y = max( a.y, r.a.y );
        b.x = min( b.x, r.b.x );
        b.y = min( b.y, r.b.y );
        
        return *this;
    }

    inline TRect& TRect::Union( const TRect& r ) {
        a.x = min( a.x, r.a.x );
        a.y = min( a.y, r.a.y );
        b.x = max( b.x, r.b.x );
        b.y = max( b.y, r.b.y );
        
        return *this;
    }

    inline bool TRect::contains(const TPoint& p) const {
        return bool(
        p.x >= a.x && p.x < b.x && p.y >= a.y && p.y < b.y);
    }

    inline bool TRect::operator == (const TRect& r) const {
        return bool( a == r.a && b == r.b );
    }

    inline bool TRect::operator != (const TRect& r) const {
        return bool( !(*this == r) );
    }

    inline bool TRect::isEmpty() {
        return bool( a.x >= b.x || a.y >= b.y );
    }

}   // namespace: std
#endif  // __TVISION_TPECT_H__
