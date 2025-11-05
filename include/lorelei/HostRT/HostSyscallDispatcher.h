#ifndef LOREHOSTRT_HOSTSYSCALLDISPATCHER_H
#define LOREHOSTRT_HOSTSYSCALLDISPATCHER_H

#include <lorelei/Core/Bridge/ThunkInfo.h>
#include <lorelei/Core/Bridge/SyscallDispatcher.h>

#include <lorelei/HostRT/Global.h>

namespace lore {

    class LOREHOSTRT_EXPORT HostSyscallDispatcher
        : public SyscallDispatcher<HostSyscallDispatcher> {
    public:
        HostSyscallDispatcher();
        ~HostSyscallDispatcher();

        static HostSyscallDispatcher *instance();

    public:
        inline void setConfig(ThunkInfoConfig config) {
            _config = std::move(config);
        }

        inline const ThunkInfoConfig &config() const {
            return _config;
        }

    public:
        using RunTaskEntry = uint64_t (*)(BridgeTask *);

        /// Must be called at the very beginning of the emulator
        static void setRunTaskEntry(RunTaskEntry runTask);

    protected:
        /// Implementation of \c SyscallDispatcher
        uint64_t dispatch_impl(uint64_t num, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4,
                               uint64_t a5, uint64_t a6);
        BridgeTask *currentTask_impl() const;
        uint64_t runTask_impl();

        ThunkInfoConfig _config;

        friend class SyscallDispatcher<HostSyscallDispatcher>;
    };

}

#endif // LOREHOSTRT_HOSTSYSCALLDISPATCHER_H
