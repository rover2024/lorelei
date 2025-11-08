#ifndef LOREHOSTRT_HostServer_H
#define LOREHOSTRT_HostServer_H

#include <lorelei/Core/Connect/ThunkInfo.h>
#include <lorelei/Core/Connect/SyscallServer.h>

#include <lorelei/HostRT/Global.h>

namespace lore {

    class LOREHOSTRT_EXPORT HostServer : public SyscallServer<HostServer> {
    public:
        HostServer();
        ~HostServer();

        static HostServer *instance();

    public:
        inline void setConfig(ThunkInfoConfig config) {
            _config = std::move(config);
        }

        inline const ThunkInfoConfig &config() const {
            return _config;
        }

    public:
        using RunTaskEntry = uint64_t (*)(ClientTask *);

        /// Must be called at the very beginning of the emulator
        static void setRunTaskEntry(RunTaskEntry runTask);

    protected:
        /// Implementation of \c Server
        uint64_t dispatch_impl(uint64_t num, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4,
                               uint64_t a5, uint64_t a6);
        ClientTask *currentTask_impl() const;
        uint64_t runTask_impl();

        ThunkInfoConfig _config;

        friend class Server<HostServer>;
        friend class SyscallServer<HostServer>;
    };

}

#endif // LOREHOSTRT_HostServer_H
