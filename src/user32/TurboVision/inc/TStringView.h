// ----------------------------------------------------------------------------
// \file  TStringView.h
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
//
// \note  Turbo Vision - Version 2.0
//        Copyright (c) 1994 by Borland International
//        All Rights Reserved.
// ----------------------------------------------------------------------------
#ifndef __TURBOVISION_TSTRINGVIEW_H__
#define __TURBOVISION_TSTRINGVIEW_H__

# include "stdint.h"
# include "stl/inc/char_traits.h"
# include "stl/inc/string.h"
# include "stl/inc/string_view.h"
# include "TurboVision/inc/TSpan.h"

extern "C" int kmemcmp(const void *a, const void *b, size_t n);

namespace tvision
{
    class TStringView {
    private:
        const char* str;
        size_t len;
    public:
        constexpr TStringView();
        constexpr TStringView(const char* str);
        constexpr TStringView(const char* str, size_t len);
        constexpr TStringView(TSpan<char> span);
        constexpr TStringView(TSpan<const char> span);
        constexpr TStringView(std::string_view text);
        
        constexpr operator std::string_view() const;
        constexpr operator TSpan<const char>() const;
        
        constexpr const char* data() const;
        constexpr size_t      size() const;
        constexpr bool       empty() const;
        
        constexpr const char& operator[](size_t pos) const;
        constexpr const char& front() const;
        constexpr const char& back() const;

        constexpr TStringView substr(size_t pos) const;
        constexpr TStringView substr(size_t pos, size_t n) const;

        constexpr const char* begin () const;
        constexpr const char* cbegin() const;
        constexpr const char* end   () const;
        constexpr const char* cend  () const;
    };

    inline constexpr TStringView::TStringView() :
        str(0),
        len(0) { }

    inline constexpr TStringView::TStringView(const char* str) :
        str(str),
        len(str ? ::std::char_traits::length(str) : 0) { }

    inline constexpr TStringView::TStringView(const char* str, size_t len) :
        str(str),
        len(len) { }

    inline constexpr TStringView::TStringView(TSpan<char> span) :
        str(span.data()),
        len(span.size()) { }

    inline constexpr TStringView::TStringView(TSpan<const char> span) :
        str(span.data()),
        len(span.size()) { }

    inline constexpr TStringView::TStringView(std::string_view text) :
        str(text.data()),
        len(text.size()) { }

    inline constexpr TStringView::operator std::string_view() const {
        return {str, len};
    }

    inline constexpr TStringView::operator TSpan<const char>() const {
        return TSpan<const char>(str, len);
    }

    inline constexpr const char* TStringView::data() const {
        return str;
    }

    inline constexpr size_t TStringView::size() const {
        return len;
    }

    inline constexpr bool TStringView::empty() const {
        return bool( size() == 0 );
    }

    inline constexpr const char& TStringView::operator[](size_t pos) const {
        return str[pos];
    }

    inline constexpr const char& TStringView::front() const {
        return str[0];
    }

    inline constexpr const char& TStringView::back() const {
        return str[len - 1];
    }

    inline constexpr TStringView TStringView::substr(size_t pos) const {
        if (pos >= len)
        return TStringView(str + len, 0); else
        return TStringView(str + pos, len - pos);
    }

    inline constexpr TStringView TStringView::substr(size_t pos, size_t n) const {
        if (pos >= len)
        return TStringView(str + len, 0); else
        return TStringView(str + pos, n <= len - pos ? n : len - pos);
    }

    inline constexpr const char* TStringView::begin() const {
        return &str[0];
    }

    inline constexpr const char* TStringView::cbegin() const {
        return &str[0];
    }

    inline constexpr const char* TStringView::end() const {
        return &str[len];
    }

    inline constexpr const char* TStringView::cend() const {
        return &str[len];
    }

    inline constexpr bool operator == (TStringView a, TStringView b) {
        if (a.size() == b.size())
        return bool( b.size() == 0 || kmemcmp(a.data(), b.data(), b.size()) == 0 );
        return false;
    }

    inline constexpr bool operator!=(TStringView a, TStringView b) {
        return bool( !(a == b) );
    }
}   // namespace: std
#endif  // __TURBOVISION_TSTRINGVIEW_H__
