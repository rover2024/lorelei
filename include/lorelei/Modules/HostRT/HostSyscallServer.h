#ifndef LORE_MODULES_HOSTRT_HOSTSYSCALLSERVER_H
#define LORE_MODULES_HOSTRT_HOSTSYSCALLSERVER_H

#include <cstdint>
#include <memory>

#include <lorelei/Base/PassThrough/Core/ThunkInfo.h>
#include <lorelei/Modules/HostRT/Invocation.h>
#include <lorelei/Modules/HostRT/Global.h>

namespace lore::mod {

    class LOREHOSTRT_EXPORT HostSyscallServer : public IServer<HostSyscallServer> {
    public:
        HostSyscallServer();
        ~HostSyscallServer();

        static inline HostSyscallServer *instance() {
            return self;
        }

        static uint64_t dispatchSyscall(uint64_t num, uint64_t a1, uint64_t a2, uint64_t a3,
                                        uint64_t a4, uint64_t a5, uint64_t a6);

        void setThunkConfig(std::unique_ptr<ThunkInfoConfig> config);

        inline const ThunkInfoConfig *thunkConfig() const {
            return m_thunkConfig.get();
        }

        /// Assigned in the emulator entry
        static void *emuAddr;

    protected:
        static inline void reenter_impl(ReentryArguments *ra) {
            return Invocation::reenter(ra);
        }

        static HostSyscallServer *self;
        std::unique_ptr<ThunkInfoConfig> m_thunkConfig;

        friend class IServer<HostSyscallServer>;
    };

}

#endif // LORE_MODULES_HOSTRT_HOSTSYSCALLSERVER_H
