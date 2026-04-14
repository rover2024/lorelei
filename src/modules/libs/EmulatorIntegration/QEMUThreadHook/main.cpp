#include <dlfcn.h>
#include <pthread.h>

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string_view>

#include <lorelei/Base/Support/StringExtras.h>
#include <lorelei/Modules/HostRT/HostSyscallServer.h>

// This module is loaded by setting LD_PRELOAD when QEMU starts.

namespace {

    using PFN_pthread_create = decltype(&pthread_create);
    using PFN_pthread_exit = decltype(&pthread_exit);

    enum CallerKind : uint8_t {
        CK_Unknown = 0,
        CK_QEMU = 1,
        CK_NonQEMU = 2,
    };

    struct ThreadContext {
        bool in_hook = false;
        CallerKind caller_kind = CK_Unknown;
        bool pending_host_thread_create = false;
        bool pending_host_attr_valid = false;
        bool pending_host_attr_detached = false;
    };

    static thread_local ThreadContext thread_ctx = {};

    static PFN_pthread_create resolve_real_pthread_create() {
        static PFN_pthread_create fn = nullptr;
        if (!fn) {
            fn = reinterpret_cast<PFN_pthread_create>(dlsym(RTLD_NEXT, "pthread_create"));
            if (!fn) {
                std::abort();
            }
        }
        return fn;
    }

    static PFN_pthread_exit resolve_real_pthread_exit() {
        static PFN_pthread_exit fn = nullptr;
        if (!fn) {
            fn = reinterpret_cast<PFN_pthread_exit>(dlsym(RTLD_NEXT, "pthread_exit"));
            if (!fn) {
                std::abort();
            }
        }
        return fn;
    }

    static bool is_qemu_caller(void *ret_addr) {
        Dl_info info = {};
        if (!ret_addr || dladdr(ret_addr, &info) == 0 || !info.dli_fname) {
            return false;
        }

        const char *path = info.dli_fname;
        if (!path) {
            return false;
        }
        const char *base = std::strrchr(path, '/');
        base = base ? base + 1 : path;
        if (!base || !*base) {
            return false;
        }
        return lore::starts_with(base, "qemu-") || lore::starts_with(base, "libqemu");
    }

    static bool hook_cache_disabled() {
        return false;

        // not used now
        static int disabled = -1;
        if (disabled < 0) {
            const char *v = std::getenv("LORELEI_QEMU_THREAD_HOOK_NO_CACHE");
            disabled = (v && v[0] != '\0' && v[0] != '0') ? 1 : 0;
        }
        return disabled != 0;
    }

}

extern "C" LORE_DECL_EXPORT int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                                               void *(*start_routine)(void *), void *arg) {
    auto real_pthread_create = resolve_real_pthread_create();

    if (thread_ctx.in_hook) {
        return real_pthread_create(thread, attr, start_routine, arg);
    }

    bool qemu_caller = false;
    if (!hook_cache_disabled() && thread_ctx.caller_kind != CK_Unknown) {
        qemu_caller = (thread_ctx.caller_kind == CK_QEMU);
    } else {
        void *ret_addr = __builtin_return_address(0);
        qemu_caller = is_qemu_caller(ret_addr);
        if (!hook_cache_disabled()) {
            thread_ctx.caller_kind = qemu_caller ? CK_QEMU : CK_NonQEMU;
        }
    }

    if (qemu_caller) {
        /*
         * - default guest-origin create: keep QEMU's detached attr.
         * - host-origin reentered create: obey host detach intent.
         */
        if (thread_ctx.pending_host_thread_create) {
            const bool host_wants_detached =
                thread_ctx.pending_host_attr_valid && thread_ctx.pending_host_attr_detached;
            thread_ctx.pending_host_thread_create = false;
            thread_ctx.pending_host_attr_valid = false;
            thread_ctx.pending_host_attr_detached = false;

            if (!host_wants_detached) {
                if (attr) {
                    pthread_attr_t *mutable_attr = const_cast<pthread_attr_t *>(attr);
                    int old_state = PTHREAD_CREATE_JOINABLE;
                    if (pthread_attr_getdetachstate(mutable_attr, &old_state) == 0 &&
                        old_state == PTHREAD_CREATE_DETACHED &&
                        pthread_attr_setdetachstate(mutable_attr, PTHREAD_CREATE_JOINABLE) == 0) {
                        int ret = real_pthread_create(thread, mutable_attr, start_routine, arg);
                        (void) pthread_attr_setdetachstate(mutable_attr, old_state);
                        return ret;
                    }
                } else {
                    pthread_attr_t tmp_attr;
                    if (pthread_attr_init(&tmp_attr) == 0) {
                        if (pthread_attr_setdetachstate(&tmp_attr, PTHREAD_CREATE_JOINABLE) == 0) {
                            int ret = real_pthread_create(thread, &tmp_attr, start_routine, arg);
                            pthread_attr_destroy(&tmp_attr);
                            return ret;
                        }
                        pthread_attr_destroy(&tmp_attr);
                    }
                }
            }
        }
        return real_pthread_create(thread, attr, start_routine, arg);
    }

    /*
     * Route non-QEMU thread creation back through HostRT reentry, so
     * thread creation can happen on the guest/QEMU-managed path.
     */
    thread_ctx.in_hook = true;
    thread_ctx.pending_host_thread_create = true;
    thread_ctx.pending_host_attr_valid = false;
    thread_ctx.pending_host_attr_detached = false;
    if (attr) {
        int detach_state = PTHREAD_CREATE_JOINABLE;
        if (pthread_attr_getdetachstate(attr, &detach_state) == 0) {
            thread_ctx.pending_host_attr_valid = true;
            thread_ctx.pending_host_attr_detached = (detach_state == PTHREAD_CREATE_DETACHED);
        }
    }
    int ret = lore::mod::HostSyscallServer::reenterThreadCreate(
        thread, const_cast<pthread_attr_t *>(attr), reinterpret_cast<void *>(start_routine), arg);
    thread_ctx.pending_host_thread_create = false;
    thread_ctx.pending_host_attr_valid = false;
    thread_ctx.pending_host_attr_detached = false;
    thread_ctx.in_hook = false;
    return ret;
}

extern "C" LORE_DECL_EXPORT void pthread_exit(void *ret) {
    auto real_pthread_exit = resolve_real_pthread_exit();

    if (thread_ctx.in_hook) {
        real_pthread_exit(ret);
        __builtin_unreachable();
    }

    bool qemu_caller = false;
    if (!hook_cache_disabled() && thread_ctx.caller_kind != CK_Unknown) {
        qemu_caller = (thread_ctx.caller_kind == CK_QEMU);
    } else {
        void *ret_addr = __builtin_return_address(0);
        qemu_caller = is_qemu_caller(ret_addr);
        if (!hook_cache_disabled()) {
            thread_ctx.caller_kind = qemu_caller ? CK_QEMU : CK_NonQEMU;
        }
    }

    if (qemu_caller) {
        real_pthread_exit(ret);
        __builtin_unreachable();
    }

    thread_ctx.in_hook = true;
    lore::mod::HostSyscallServer::reenterThreadExit(ret);
    thread_ctx.in_hook = false;

    real_pthread_exit(ret);
    __builtin_unreachable();
}
