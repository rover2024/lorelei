#ifndef LORE_MODULES_HOSTRT_HOSTSYSCALLSERVER_H
#define LORE_MODULES_HOSTRT_HOSTSYSCALLSERVER_H

#include <cstdint>

#include <lorelei/Modules/HostRT/Invocation.h>
#include <lorelei/Modules/HostRT/Global.h>

namespace lore::mod {

    class LOREHOSTRT_EXPORT HostSyscallServer : public IServer<HostSyscallServer> {
    public:
        HostSyscallServer() = delete;
        ~HostSyscallServer() = delete;

        static uint64_t dispatchSyscall(uint64_t num, uint64_t a1, uint64_t a2, uint64_t a3,
                                        uint64_t a4, uint64_t a5, uint64_t a6);

    protected:
        static inline void reenter_impl(ReentryArguments *ra) {
            return Invocation::reenter(ra);
        }

    protected:
        friend class IServer<HostSyscallServer>;
    };

}

#endif // LORE_MODULES_HOSTRT_HOSTSYSCALLSERVER_H
