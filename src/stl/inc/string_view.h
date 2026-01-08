// ----------------------------------------------------------------------------
// \file  string_view.h
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
#ifndef __STRING_VIEW_H__
#define __STRING_VIEW_H__

# include "stdint.h"
# include "cstdint.h"

namespace std {
    class string_view {
    public:
        using size_type = std::size_t;
        static const size_type npos = size_type(-1);

        constexpr string_view(): ptr_(nullptr), len_(0) {}
        constexpr string_view(const char* s, size_type n): ptr_(s), len_(n) {}
        // C-String ctor (berechnet Länge)
        explicit string_view(const char* s): ptr_(s), len_(cstrlen_(s)) {}

        // Zugriff
        constexpr const char* data()  const { return ptr_; }
        constexpr size_type   size()  const { return len_; }
        constexpr bool        empty() const { return len_ == 0; }

        constexpr char operator[](size_type i) const { return ptr_[i]; }

        // Teilansicht
        constexpr string_view substr(size_type pos, size_type count = npos) const {
            if (pos > len_) return {}; // oder assert/throw, je nach Geschmack
            const size_type rcount = (count == npos || pos + count > len_) ? (len_ - pos) : count;
            return string_view(ptr_ + pos, rcount);
        }

        // Prefix/Suffix entfernen (praktisch im Kernel)
        constexpr void remove_prefix(size_type n) {
            if (n > len_) n = len_;
            ptr_ += n; len_ -= n;
        }
        constexpr void remove_suffix(size_type n) {
            if (n > len_) n = len_;
            len_ -= n;
        }

        // Vergleichen (lexikografisch)
        constexpr int compare(string_view other) const {
            const size_type m = (len_ < other.len_) ? len_ : other.len_;
            for (size_type i = 0; i < m; ++i) {
                const unsigned a = (unsigned char)ptr_[i];
                const unsigned b = (unsigned char)other.ptr_[i];
                if (a != b) return (a < b) ? -1 : 1;
            }
            if (len_ == other.len_) return 0;
            return (len_ < other.len_) ? -1 : 1;
        }

        friend constexpr bool operator==(string_view a, string_view b) {
            return a.len_ == b.len_ && a.compare(b) == 0;
        }
        friend constexpr bool operator!=(string_view a, string_view b) { return !(a == b); }

    private:
        const char* ptr_;
        size_type   len_;

        static constexpr size_type cstrlen_(const char* s) {
            if (!s) return 0;
            size_type n = 0;
            while (s[n] != '\0') ++n;
            return n;
        }
    };
}       // namespace: std
#endif  // __STRING_VIEW_H__
