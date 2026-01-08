// ----------------------------------------------------------------------------
// \file  string.h
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
#ifndef __STL_STRING_H__
#define __STL_STRING_H__

# include "stdint.h"
extern "C" void* kmalloc(uint32_t);
extern "C" void  kfree(void*);

# include "stl/inc/char_traits.h"
# include "stl/inc/iostream.h"

namespace std {
    // -------------------------------
    // std::string (minimal)
    // -------------------------------
    class string {
        char*  _buf;
        size_t _size;
        size_t _cap; // capacity (ohne \0)

        void ensure_cap(size_t want) {
            if (want <= _cap) return;
            size_t newcap = (_cap == 0) ? 16 : _cap;
            while (newcap < want) newcap = newcap * 2;

            char* nb = (char*)kmalloc(newcap + 1);
            if (!nb) {
                // freestanding: du kannst hier panic() oder abort() machen
                // wir lassen's hart crashen, weil nullptr weiter fatal wäre.
                for (;;) {}
            }
            if (_buf && _size) ::std::char_traits::copy(nb, _buf, _size);
            nb[_size] = '\0';

            if (_buf) kfree(_buf);
            _buf = nb;
            _cap = newcap;
        }

    public:
        string() : _buf(nullptr), _size(0), _cap(0) {
            ensure_cap(0);
            if (_buf) _buf[0] = '\0';
        }

        string(const char* s) : _buf(nullptr), _size(0), _cap(0) {
            size_t n = char_traits::length(s);
            ensure_cap(n);
            if (n) char_traits::copy(_buf, s, n);
            _size = n;
            _buf[_size] = '\0';
        }

        string(const string& o) : _buf(nullptr), _size(0), _cap(0) {
            ensure_cap(o._size);
            if (o._size) std::char_traits::copy(_buf, o._buf, o._size);
            _size = o._size;
            _buf[_size] = '\0';
        }

        string& operator=(const string& o) {
            if (this == &o) return *this;
            ensure_cap(o._size);
            if (o._size) std::char_traits::copy(_buf, o._buf, o._size);
            _size = o._size;
            _buf[_size] = '\0';
            return *this;
        }

        ~string() {
            if (_buf) kfree(_buf);
            _buf = nullptr;
            _size = _cap = 0;
        }

        const char* c_str() const { return _buf ? _buf : ""; }
        char* data() { return _buf; }
        const char* data() const { return _buf; }

        size_t size() const { return _size; }
        bool empty() const { return _size == 0; }

        void clear() {
            _size = 0;
            if (_buf) _buf[0] = '\0';
        }

        void reserve(size_t n) { ensure_cap(n); }
        size_t capacity() const { return _cap; }

        void push_back(char c) {
            ensure_cap(_size + 1);
            _buf[_size++] = c;
            _buf[_size] = '\0';
        }

        string& operator+=(const char* s) {
            size_t n = char_traits::length(s);
            ensure_cap(_size + n);
            if (n) std::char_traits::copy(_buf + _size, s, n);
            _size += n;
            _buf[_size] = '\0';
            return *this;
        }

        string& operator+=(const string& s) { return (*this += s.c_str()); }

        friend string operator+(const string& a, const string& b) {
            string r;
            r.ensure_cap(a._size + b._size);
            if (a._size) std::char_traits::copy(r._buf, a._buf, a._size);
            if (b._size) std::char_traits::copy(r._buf + a._size, b._buf, b._size);
            r._size = a._size + b._size;
            r._buf[r._size] = '\0';
            return r;
        }

        char operator[](size_t i) const { return _buf[i]; }
        char& operator[](size_t i) { return _buf[i]; }
    };
}   // namespace std

#endif  // __STL_STRING_H__
