#ifndef LORE_BASE_CORELIB_SCOPEGUARD_H
#define LORE_BASE_CORELIB_SCOPEGUARD_H

#include <type_traits>
#include <utility>

namespace lore {

    template <class F>
    class ScopeGuard {
    public:
        explicit ScopeGuard(F &&f) noexcept : _func(std::move(f)) {
        }

        explicit ScopeGuard(const F &f) noexcept : _func(f) {
        }

        ScopeGuard(ScopeGuard &&RHS) noexcept
            : _func(std::move(RHS._func)), _active(std::exchange(RHS._active, false)) {
        }

        ~ScopeGuard() noexcept {
            if (_active)
                _func();
        }

        void dismiss() noexcept {
            _active = false;
        }

    protected:
        F _func;
        bool _active = true;
    };

    template <class F>
    ScopeGuard(F (&)()) -> ScopeGuard<F (*)()>;

    //! [make_scope_guard]
    template <typename F>
    [[nodiscard]] ScopeGuard<typename std::decay<F>::type> makeScopeGuard(F &&f) {
        return ScopeGuard<typename std::decay<F>::type>(std::forward<F>(f));
    }

}

#endif // LORE_BASE_CORELIB_SCOPEGUARD_H
