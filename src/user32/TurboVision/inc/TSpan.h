// ----------------------------------------------------------------------------
// \file  TSpan.h
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
//
// \note  Turbo Vision - Version 2.0
//        Copyright (c) 1994 by Borland International
//        All Rights Reserved.
// ----------------------------------------------------------------------------
#ifndef __TVISION_TSPAN_H__
#define __TVISION_TSPAN_H__

# include "stdint.h"

template <class T>
class TSpan {
    T *ptr;
    size_t len;
public:
    constexpr TSpan() :
        ptr(0),
        len(0) {
    }

    constexpr TSpan(T *first, size_t n) : ptr(first),
        len(n) {
    }

    constexpr TSpan(decltype(nullptr)) : TSpan() {
    }

    template<size_t N>
    constexpr TSpan(T (&array)[N]) :
        TSpan(array, N) {
    }

    constexpr operator TSpan<const T>() const {
        return TSpan<const T>(ptr, len);
    }

    constexpr T _FAR * data() const {
        return ptr;
    }

    constexpr size_t size() const {
        return len;
    }

    constexpr size_t size_bytes() const {
        return size()*sizeof(T);
    }

    constexpr bool empty() const {
        return bool( size() == 0 );
    }

    constexpr T _FAR & operator[](size_t pos) const {
        return ptr[pos];
    }

    constexpr T _FAR & front() const {
        return ptr[0];
    }

    constexpr T _FAR & back() const {
        return ptr[len - 1];
    }

    constexpr TSpan subspan(size_t pos) const {
        return TSpan<T>(ptr + pos, len - pos);
    }

    constexpr TSpan subspan(size_t pos, size_t n) const {
        return TSpan<T>(ptr + pos, n <= len - pos ? n : len - pos);
    }

    constexpr T _FAR * begin() const {
        return &ptr[0];
    }

    constexpr const T _FAR * cbegin() const {
        return &ptr[0];
    }

    constexpr T _FAR * end() const {
        return &ptr[len];
    }

    constexpr const T _FAR * cend() const {
        return &ptr[len];
    }
};

#endif  // __TVISION_TSPAN_H__
