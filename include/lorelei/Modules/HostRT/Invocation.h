#ifndef LORE_MODULES_HOSTRT_INVOCATION_H
#define LORE_MODULES_HOSTRT_INVOCATION_H

#include <cstdint>

#include <lorelei/Base/PassThrough/Core/IClient.h>
#include <lorelei/Base/PassThrough/Core/IServer.h>
#include <lorelei/Modules/HostRT/Global.h>

namespace lore::mod {

    class LOREHOSTRT_EXPORT Invocation {
    public:
        Invocation() = delete;
        ~Invocation() = delete;

        /// Start a native pass-through invocation.
        /// @param conv The invocation convention.
        /// @param ia The invocation arguments.
        /// @param ra_ptr The pointer to store the reentry arguments.
        /// @return 0 if the invocation is complete, 1 if there is a reentry.
        static int64_t invoke(const InvocationArguments *ia, ReentryArguments **ra_ptr);
        static int64_t resume();
        static void reenter(ReentryArguments *ra);
    };

}

#endif // LORE_MODULES_HOSTRT_INVOCATION_H
