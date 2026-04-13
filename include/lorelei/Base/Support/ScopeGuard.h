#ifndef LORE_BASE_CORELIB_SCOPEGUARD_H
#define LORE_BASE_CORELIB_SCOPEGUARD_H

#include <type_traits>
#include <utility>

namespace lore {

    template <class F>
    class ScopeGuard {
    public:
        explicit ScopeGuard(F &&f) noexcept : m_func(std::move(f)) {
        }

        explicit ScopeGuard(const F &f) noexcept : m_func(f) {
        }

        ScopeGuard(ScopeGuard &&RHS) noexcept
            : m_func(std::move(RHS.m_func)), m_active(std::exchange(RHS.m_active, false)) {
        }

        ~ScopeGuard() noexcept {
            if (m_active)
                m_func();
        }

        void dismiss() noexcept {
            m_active = false;
        }

    protected:
        F m_func;
        bool m_active = true;
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
