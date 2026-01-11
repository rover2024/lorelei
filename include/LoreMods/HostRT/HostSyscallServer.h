#ifndef LORE_MODULES_HOSTRT_HOSTSYSCALLSERVER_H
#define LORE_MODULES_HOSTRT_HOSTSYSCALLSERVER_H

#include <LoreBase/RuntimeBase/PassThrough/IServer.h>
#include <LoreMods/HostRT/Global.h>

namespace lore::mod {

    class LOREHOSTRT_EXPORT HostSyscallServer : public IServer<HostSyscallServer> {
    public:
        HostSyscallServer();
        ~HostSyscallServer();

    protected:
        static int invokeReentry_impl(void *proc, int conv, void *opaque);

    protected:
        friend class IServer<HostSyscallServer>;
    };

}

#endif // LORE_MODULES_HOSTRT_HOSTSYSCALLSERVER_H
