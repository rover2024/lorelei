#include "HostSyscallServer.h"

#include "HostSyscallServer.h"

namespace lore::mod {

    HostSyscallServer::HostSyscallServer() = default;

    HostSyscallServer::~HostSyscallServer() = default;

    int HostSyscallServer::invokeReentry_impl(void *proc, int conv, void *opaque) {
        return {};
    }

}
