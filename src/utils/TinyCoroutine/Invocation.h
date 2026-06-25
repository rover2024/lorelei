#ifndef LORE_MODULES_HOSTRT_INVOCATION_H
#define LORE_MODULES_HOSTRT_INVOCATION_H

#include <cstdint>

namespace lore::utils {

    class Invocation {
    public:
        using InvocationArguments = void;
        using ReentryArguments = void;

        Invocation() = delete;
        ~Invocation() = delete;

        /// Start a native pass-through invocation.
        /// @param ia The invocation arguments.
        /// @param ra_ptr The pointer for caller to get the next reentry arguments.
        /// @return 0 if the invocation is complete, 1 if there is a reentry.
        static int64_t invoke(const InvocationArguments *ia, ReentryArguments **ra_ptr);

        /// Resume the invocation, called by the guest after a reentry.
        static int64_t resume();

        /// Reenter the invocation, called by the host during an invocation.
        /// @param ra The reentry arguments.
        static void reenter(ReentryArguments *ra);

        /// Must be defined in the main target.
        static int invokeByConv(const InvocationArguments *ia);
    };

}

#endif // LORE_MODULES_HOSTRT_INVOCATION_H
