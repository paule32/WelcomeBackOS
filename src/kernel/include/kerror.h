#pragma once
#include <stdint.h>
#include <stddef.h>

namespace kx {

enum class Err : uint32_t {
    Ok = 0,
    Unknown,
    NoMem,
    InvalidArg,
    Io,
    NotFound,
    Busy,
    Timeout,
    Unsupported,
    Internal,
};

struct SrcLoc {
    const char* file;
    const char* func;
    uint32_t    line;

    static constexpr SrcLoc none() { return {nullptr, nullptr, 0u}; }
};

struct Error {
    Err         code;
    const char* msg;   // optional
    SrcLoc      where;

    static constexpr Error ok() { return {Err::Ok, nullptr, SrcLoc::none()}; }
    constexpr bool is_ok() const { return code == Err::Ok; }
};

// C++20: source_location wäre nett, aber wir bleiben freestanding/ohne STL.
constexpr inline SrcLoc here(const char* file, uint32_t line, const char* func) {
    return {file, func, line};
}

constexpr inline Error make_error(Err code, const char* msg, SrcLoc loc) {
    return {code, msg, loc};
}

template <typename T>
class Result {
    union { T value_; };
    Error err_;
    bool ok_;

public:
    // ok
    Result(const T& v) : value_(v), err_(Error::ok()), ok_(true) {}
    Result(T&& v)      : value_((T&&)v), err_(Error::ok()), ok_(true) {}

    // err
    Result(Error e) : err_(e), ok_(false) {}

    ~Result() { if (ok_) value_.~T(); }

    Result(const Result& o) : err_(o.err_), ok_(o.ok_) {
        if (ok_) new (&value_) T(o.value_);
    }
    Result& operator=(const Result& o) {
        if (this == &o) return *this;
        if (ok_) value_.~T();
        err_ = o.err_;
        ok_  = o.ok_;
        if (ok_) new (&value_) T(o.value_);
        return *this;
    }

    bool ok() const { return ok_; }
    explicit operator bool() const { return ok_; }

    T&       value()       { return value_; }
    const T& value() const { return value_; }

    Error error() const { return err_; }

    // Convenience: value_or
    T value_or(const T& fallback) const { return ok_ ? value_ : fallback; }
};

template <>
class Result<void> {
    Error err_;
    bool ok_;
public:
    Result() : err_(Error::ok()), ok_(true) {}
    Result(Error e) : err_(e), ok_(false) {}

    bool ok() const { return ok_; }
    explicit operator bool() const { return ok_; }
    Error error() const { return err_; }
};

} // namespace kx
