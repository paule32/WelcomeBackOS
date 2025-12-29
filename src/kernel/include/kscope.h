#pragma once

namespace kx {

// ScopeExit: führt F beim Verlassen des Scopes aus (auch bei early-return)
template <typename F>
class ScopeExit {
    F f_;
    bool active_;

public:
    explicit ScopeExit(F f) : f_(f), active_(true) {}
    ~ScopeExit() { if (active_) f_(); }

    ScopeExit(const ScopeExit&) = delete;
    ScopeExit& operator=(const ScopeExit&) = delete;

    ScopeExit(ScopeExit&& other) : f_((F&&)other.f_), active_(other.active_) {
        other.active_ = false;
    }

    void dismiss() { active_ = false; }
};

template <typename F>
inline ScopeExit<F> scope_exit(F f) { return ScopeExit<F>(f); }

} // namespace kx

// --- defer Makro ---
// Usage:
//   KX_DEFER { kfree(ptr); };
#define KX_DEFER \
    auto _kx_defer_##__LINE__ = ::kx::scope_exit([&]()

#define KX_DEFER_END );
