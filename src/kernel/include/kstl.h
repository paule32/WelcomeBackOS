// ----------------------------------------------------------------------------
// \file  kstl.h
// \note  (c) 2025 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
#pragma once

// freestanding minimal types
# include "stdint.h"

namespace std {

// -------------------------------
// Minimal allocator hooks
// -------------------------------
// Du kannst hier direkt kmalloc/kfree dranhängen.
extern "C" void* mini_malloc(size_t n);
extern "C" void  mini_free(void* p);

// -------------------------------
// Minimal char traits (nur was wir brauchen)
// -------------------------------
struct char_traits {
    static size_t length(const char* s) {
        if (!s) return 0;
        const char* p = s;
        while (*p) ++p;
        return (size_t)(p - s);
    }
    static int compare(const char* a, const char* b, size_t n) {
        for (size_t i = 0; i < n; ++i) {
            unsigned char ac = (unsigned char)a[i];
            unsigned char bc = (unsigned char)b[i];
            if (ac < bc) return -1;
            if (ac > bc) return  1;
            if (ac == 0) return 0;
        }
        return 0;
    }
    static void copy(char* dst, const char* src, size_t n) {
        for (size_t i = 0; i < n; ++i) dst[i] = src[i];
    }
    static void move(char* dst, const char* src, size_t n) {
        if (dst == src || n == 0) return;
        if (dst < src) {
            for (size_t i = 0; i < n; ++i) dst[i] = src[i];
        } else {
            for (size_t i = n; i-- > 0;) dst[i] = src[i];
        }
    }
};

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

        char* nb = (char*)mini_malloc(newcap + 1);
        if (!nb) {
            // freestanding: du kannst hier panic() oder abort() machen
            // wir lassen's hart crashen, weil nullptr weiter fatal wäre.
            for (;;) {}
        }
        if (_buf && _size) char_traits::copy(nb, _buf, _size);
        nb[_size] = '\0';

        if (_buf) mini_free(_buf);
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
        if (o._size) char_traits::copy(_buf, o._buf, o._size);
        _size = o._size;
        _buf[_size] = '\0';
    }

    string& operator=(const string& o) {
        if (this == &o) return *this;
        ensure_cap(o._size);
        if (o._size) char_traits::copy(_buf, o._buf, o._size);
        _size = o._size;
        _buf[_size] = '\0';
        return *this;
    }

    ~string() {
        if (_buf) mini_free(_buf);
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
        if (n) char_traits::copy(_buf + _size, s, n);
        _size += n;
        _buf[_size] = '\0';
        return *this;
    }

    string& operator+=(const string& s) { return (*this += s.c_str()); }

    friend string operator+(const string& a, const string& b) {
        string r;
        r.ensure_cap(a._size + b._size);
        if (a._size) char_traits::copy(r._buf, a._buf, a._size);
        if (b._size) char_traits::copy(r._buf + a._size, b._buf, b._size);
        r._size = a._size + b._size;
        r._buf[r._size] = '\0';
        return r;
    }

    char operator[](size_t i) const { return _buf[i]; }
    char& operator[](size_t i) { return _buf[i]; }
};

// -------------------------------
// Minimal iostream
// -------------------------------
struct endl_t {};
static constexpr endl_t endl{};

class ostream {
public:
    // Du setzt diesen Hook einmal beim Init.
    using write_fn = void(*)(const char*, size_t);
    write_fn write = nullptr;

    ostream& put(char c) {
        if (write) write(&c, 1);
        return *this;
    }

    ostream& write_str(const char* s) {
        if (!s) s = "(null)";
        size_t n = char_traits::length(s);
        if (write && n) write(s, n);
        return *this;
    }

    // --- operator<< Überladungen
    ostream& operator<<(const char* s) { return write_str(s); }
    ostream& operator<<(char c)        { return put(c); }
    ostream& operator<<(const string& s){ return write_str(s.c_str()); }

    ostream& operator<<(endl_t) {
        put('\n');
        return *this;
    }

    // integer printing (minimal)
    ostream& operator<<(sint16_t v)  { return print_signed((sint16_t)v); }
    ostream& operator<<(sint32_t v)  { return print_signed((sint32_t)v); }
    
    ostream& operator<<(uint16_t v)  { return print_unsigned((uint16_t)v); }
    ostream& operator<<(uint32_t v)  { return print_unsigned((uint32_t)v); }

    ostream& hex(uint32_t v) { // helper: "0x..."
        write_str("0x");
        return print_hex(v);
    }

private:
    ostream& print_signed(sint32_t v) {
        if (v < 0) { put('-'); return print_unsigned((uint32_t)(-v)); }
        return print_unsigned((uint32_t)v);
    }
    ostream& print_unsigned(uint32_t v) {
        char buf[32];
        size_t i = 0;
        if (v == 0) { put('0'); return *this; }
        while (v && i < sizeof(buf)) {
            buf[i++] = char('0' + (v % 10));
            v /= 10;
        }
        while (i) put(buf[--i]);
        return *this;
    }
    ostream& print_hex(uint32_t v) {
        char buf[32];
        const char* digits = "0123456789ABCDEF";
        size_t i = 0;
        if (v == 0) { put('0'); return *this; }
        while (v && i < sizeof(buf)) {
            buf[i++] = digits[(size_t)(v & 0xF)];
            v >>= 4;
        }
        while (i) put(buf[--i]);
        return *this;
    }
};

class istream {
public:
    using read_fn = size_t(*)(char*, size_t);
    read_fn read = nullptr;

    // liest ein "Wort" (whitespace-separated) in std::string
    istream& operator>>(string& out) {
        out.clear();
        if (!read) return *this;

        // skip spaces
        char c = 0;
        while (true) {
            size_t n = read(&c, 1);
            if (n == 0) return *this;     // no data
            if (c != ' ' && c != '\n' && c != '\r' && c != '\t')
                break;
        }

        // read token
        while (true) {
            if (c == ' ' || c == '\n' || c == '\r' || c == '\t')
                break;
            out.push_back(c);
            size_t n = read(&c, 1);
            if (n == 0) break;
        }
        return *this;
    }

    // getline minimal
    bool getline(string& out) {
        out.clear();
        if (!read) return false;
        char c = 0;
        while (true) {
            size_t n = read(&c, 1);
            if (n == 0) return (out.size() != 0);
            if (c == '\n') break;
            if (c == '\r') continue;
            out.push_back(c);
        }
        return true;
    }
};

// globale Streams
extern ostream cout;
extern istream cin;

} // namespace std
