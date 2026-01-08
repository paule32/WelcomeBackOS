// ----------------------------------------------------------------------------
// \file  string_view.h
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
#ifndef __STRING_VIEW_H__
#define __STRING_VIEW_H__

#include <cstddef>
#include <cstdint>

class string_view {
public:
    using size_type = std::size_t;
    static const size_type npos = size_type(-1);

    constexpr string_view() noexcept : ptr_(nullptr), len_(0) {}
    constexpr string_view(const char* s, size_type n) noexcept : ptr_(s), len_(n) {}
    // C-String ctor (berechnet Länge)
    explicit string_view(const char* s) noexcept : ptr_(s), len_(cstrlen_(s)) {}

    // Zugriff
    constexpr const char* data()  const noexcept { return ptr_; }
    constexpr size_type   size()  const noexcept { return len_; }
    constexpr bool        empty() const noexcept { return len_ == 0; }

    constexpr char operator[](size_type i) const noexcept { return ptr_[i]; }

    // Teilansicht
    constexpr string_view substr(size_type pos, size_type count = npos) const noexcept {
        if (pos > len_) return {}; // oder assert/throw, je nach Geschmack
        const size_type rcount = (count == npos || pos + count > len_) ? (len_ - pos) : count;
        return string_view(ptr_ + pos, rcount);
    }

    // Prefix/Suffix entfernen (praktisch im Kernel)
    constexpr void remove_prefix(size_type n) noexcept {
        if (n > len_) n = len_;
        ptr_ += n; len_ -= n;
    }
    constexpr void remove_suffix(size_type n) noexcept {
        if (n > len_) n = len_;
        len_ -= n;
    }

    // Vergleichen (lexikografisch)
    constexpr int compare(string_view other) const noexcept {
        const size_type m = (len_ < other.len_) ? len_ : other.len_;
        for (size_type i = 0; i < m; ++i) {
            const unsigned a = (unsigned char)ptr_[i];
            const unsigned b = (unsigned char)other.ptr_[i];
            if (a != b) return (a < b) ? -1 : 1;
        }
        if (len_ == other.len_) return 0;
        return (len_ < other.len_) ? -1 : 1;
    }

    friend constexpr bool operator==(string_view a, string_view b) noexcept {
        return a.len_ == b.len_ && a.compare(b) == 0;
    }
    friend constexpr bool operator!=(string_view a, string_view b) noexcept { return !(a == b); }

private:
    const char* ptr_;
    size_type   len_;

    static constexpr size_type cstrlen_(const char* s) noexcept {
        if (!s) return 0;
        size_type n = 0;
        while (s[n] != '\0') ++n;
        return n;
    }
};

#endif  // __STRING_VIEW_H__
