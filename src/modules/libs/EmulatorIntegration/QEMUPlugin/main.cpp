#include <cstdint>
#include <cstdio>

#include <lorelei/Base/PassThrough/Core/SyscallPassThrough.h>
#include <lorelei/Modules/HostRT/HostSyscallServer.h>

extern "C" {

typedef uint64_t qemu_plugin_id_t;

typedef struct qemu_info_t qemu_info_t;

typedef bool (*qemu_plugin_vcpu_syscall_filter_cb_t)(qemu_plugin_id_t id, unsigned int vcpu_index,
                                                     int64_t num, uint64_t a1, uint64_t a2,
                                                     uint64_t a3, uint64_t a4, uint64_t a5,
                                                     uint64_t a6, uint64_t a7, uint64_t a8,
                                                     uint64_t *sysret);

extern void qemu_plugin_register_vcpu_syscall_filter_cb(qemu_plugin_id_t id,
                                                        qemu_plugin_vcpu_syscall_filter_cb_t cb);

/// @brief QEMU plugin version
LORE_DECL_EXPORT int qemu_plugin_version = 6;

/// @brief QEMU plugin register entry
LORE_DECL_EXPORT int qemu_plugin_install(qemu_plugin_id_t id, const qemu_info_t *info, int argc,
                                         char **argv);
}

namespace plugin {

    static bool vcpu_syscall_filter(qemu_plugin_id_t id, unsigned int vcpu_index, int64_t num,
                                    uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5,
                                    uint64_t a6, uint64_t a7, uint64_t a8, uint64_t *sysret) {
        if (num == lore::SyscallPathThroughNumber) {
            *sysret = lore::mod::HostSyscallServer::dispatchSyscall(num, a1, a2, a3, a4, a5, a6);
            return true;
        }
        return false;
    }

}

int qemu_plugin_install(qemu_plugin_id_t id, const qemu_info_t *info, int argc, char **argv) {
    /// @brief Set the emu address to one of QEMU functions
    lore::mod::HostSyscallServer::emuAddr = (void *) qemu_plugin_register_vcpu_syscall_filter_cb;

    qemu_plugin_register_vcpu_syscall_filter_cb(id, plugin::vcpu_syscall_filter);
    return 0;
}
