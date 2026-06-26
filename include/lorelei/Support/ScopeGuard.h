// SPDX-License-Identifier: MIT

#ifndef LORE_SUPPORT_SCOPEGUARD_H
#define LORE_SUPPORT_SCOPEGUARD_H

#include <type_traits>
#include <utility>

namespace lore {

    /// ScopeGuard - Runs a callable when it goes out of scope, for RAII-style cleanup.
    ///
    /// The action fires in the destructor unless the guard was dismiss()ed or moved from. Build one
    /// with makeScopeGuard() so the callable type is deduced.
    template <class F>
    class ScopeGuard {
    public:
        explicit ScopeGuard(F &&f) noexcept : m_func(std::move(f)) {
        }

        explicit ScopeGuard(const F &f) noexcept : m_func(f) {
        }

        /// Takes over the action from \c RHS, leaving it inactive so it runs only once.
        ScopeGuard(ScopeGuard &&RHS) noexcept
            : m_func(std::move(RHS.m_func)), m_active(std::exchange(RHS.m_active, false)) {
        }

        ~ScopeGuard() noexcept {
            if (m_active)
                m_func();
        }

        /// Cancels the action so it will not run on destruction.
        void dismiss() noexcept {
            m_active = false;
        }

    protected:
        F m_func;             ///< the cleanup action
        bool m_active = true; ///< false once dismissed or moved from
    };

    // Deduce a function pointer when constructed from a plain function reference.
    template <class F>
    ScopeGuard(F (&)()) -> ScopeGuard<F (*)()>;

    /// Creates a ScopeGuard from any callable, deducing and decaying its type.
    //! [make_scope_guard]
    template <typename F>
    [[nodiscard]] ScopeGuard<typename std::decay<F>::type> makeScopeGuard(F &&f) {
        return ScopeGuard<typename std::decay<F>::type>(std::forward<F>(f));
    }

}

#endif // LORE_SUPPORT_SCOPEGUARD_H
