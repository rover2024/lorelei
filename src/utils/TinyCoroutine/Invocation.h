// SPDX-License-Identifier: MIT

#ifndef LORE_UTILS_TINYCOROUTINE_INVOCATION_H
#define LORE_UTILS_TINYCOROUTINE_INVOCATION_H

#include <cstdint>

namespace lore::utils {

    /// Invocation - Drives one coroutine-style native call that can suspend to reenter the caller.
    ///
    /// This is the machinery behind nested cross-boundary calls: a native invocation runs to
    /// completion unless it needs to call back ("reenter") the other side, in which case it
    /// suspends and hands the reentry arguments back to the driver, which services them and resumes
    /// it. The driver loop is:
    ///
    /// \code
    ///   ReentryArguments *ra;
    ///   int64_t r = Invocation::invoke(ia, &ra);
    ///   while (r == 1) {        // 1 == suspended at a reentry, 0 == finished
    ///       service(ra);        // handle the reentry on the other side
    ///       r = Invocation::resume();
    ///   }
    /// \endcode
    ///
    /// All members are static; the class is never instantiated.
    ///
    /// \note invokeByConv() is NOT defined here. The main target must provide it (it performs the
    /// actual ABI-specific native call); otherwise linking fails.
    class Invocation {
    public:
        /// Opaque invocation arguments; the concrete layout is defined by the main target.
        using InvocationArguments = void;
        /// Opaque reentry arguments, surfaced to the driver between suspensions.
        using ReentryArguments = void;

        // Static-only utility; not instantiable.
        Invocation() = delete;
        ~Invocation() = delete;

        /// Starts a native invocation. On a reentry it suspends and writes the next reentry
        /// arguments through \a ra_ptr.
        /// \returns 0 when the invocation finished, or 1 when it suspended at a reentry.
        static int64_t invoke(const InvocationArguments *ia, ReentryArguments **ra_ptr);

        /// Resumes a suspended invocation after its reentry has been serviced; called by the guest.
        /// \returns 0 when finished, or 1 when it suspended at the next reentry.
        static int64_t resume();

        /// Suspends the running invocation to reenter the other side; called by the host from within
        /// the native call. \a ra is surfaced to the driver as the next reentry's arguments.
        static void reenter(ReentryArguments *ra);

        /// Performs the actual ABI-specific native call. Must be defined by the main target (see the
        /// class note); not provided here.
        static int invokeByConv(const InvocationArguments *ia);
    };

}

#endif // LORE_UTILS_TINYCOROUTINE_INVOCATION_H
