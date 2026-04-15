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

        /// Set in the emulator entry
        static void *emuAddr;

        /// Set the syscall number at current syscall entry and -1 at exit
        static thread_local uint64_t curSyscallNum;

    protected:
        static inline void reenter_impl(ReentryArguments *ra) {
            // FIXME: Maybe host and guest have different pthread_attr_t layout, should we thunk the
            // ThreadCreate attr and do some conversions?
            return Invocation::reenter(ra);
        }

        static HostSyscallServer *self;
        std::unique_ptr<ThunkInfoConfig> m_thunkConfig;

        friend class IServer<HostSyscallServer>;
    };

}

#endif // LORE_MODULES_HOSTRT_HOSTSYSCALLSERVER_H
