#pragma once
#include "kerror.hpp"

extern "C" void gfx_printf(const char*, ...);

namespace kx {

inline const char* err_name(Err e) {
    switch (e) {
        case Err::Ok:          return "Ok";
        case Err::Unknown:     return "Unknown";
        case Err::NoMem:       return "NoMem";
        case Err::InvalidArg:  return "InvalidArg";
        case Err::Io:          return "Io";
        case Err::NotFound:    return "NotFound";
        case Err::Busy:        return "Busy";
        case Err::Timeout:     return "Timeout";
        case Err::Unsupported: return "Unsupported";
        case Err::Internal:    return "Internal";
        default:               return "?";
    }
}

inline void log_error(const Error& e, const char* prefix = "ERR") {
    const char* file = e.where.file ? e.where.file : "<?>"; 
    const char* func = e.where.func ? e.where.func : "<?>"; 
    const char* msg  = e.msg ? e.msg : "<no msg>";

    gfx_printf("%s %s (%u) at %s:%u (%s): %s\n",
               prefix,
               err_name(e.code),
               (unsigned)e.code,
               file,
               (unsigned)e.where.line,
               func,
               msg);
}

__attribute__((noreturn)) inline void panic(const char* why) {
    gfx_printf("PANIC: %s\n", why ? why : "<no reason>");
    __builtin_trap();
}

__attribute__((noreturn)) inline void panic_error(const Error& e, const char* why = nullptr) {
    if (why) gfx_printf("PANIC: %s\n", why);
    log_error(e, "PANIC_ERR");
    __builtin_trap();
}

} // namespace kx
