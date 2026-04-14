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

    struct ThreadContext {
        bool in_hook = false;
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

    static inline bool is_qemu_thread_create() {
        // QEMU uses pthread_create when handling clone syscalls
        return lore::mod::HostSyscallServer::curSyscallNum == 56;
    }

    static inline bool is_qemu_thread_exit() {
        // QEMU uses pthread_exit when handling exit syscalls (not main thread)
        return lore::mod::HostSyscallServer::curSyscallNum == 60;
    }

}

extern "C" LORE_DECL_EXPORT int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                                               void *(*start_routine)(void *), void *arg) {
    auto real_pthread_create = resolve_real_pthread_create();

    if (thread_ctx.in_hook) {
        return real_pthread_create(thread, attr, start_routine, arg);
    }

    if (is_qemu_thread_create()) {
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

    if (!lore::mod::HostSyscallServer::emuAddr) {
        // not initialized yet
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

    if (is_qemu_thread_exit()) {
        real_pthread_exit(ret);
        __builtin_unreachable();
    }

    if (!lore::mod::HostSyscallServer::emuAddr) {
        // not initialized yet
        real_pthread_exit(ret);
    }

    thread_ctx.in_hook = true;
    lore::mod::HostSyscallServer::reenterThreadExit(ret);
    thread_ctx.in_hook = false;

    real_pthread_exit(ret);
    __builtin_unreachable();
}
