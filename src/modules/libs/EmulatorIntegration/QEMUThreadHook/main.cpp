#include <dlfcn.h>
#include <pthread.h>

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string_view>

#include <lorelei/Base/Support/StringExtras.h>
#include <lorelei/Base/PassThrough/Core/SyscallPassThrough.h>
#include <lorelei/Modules/HostRT/HostSyscallServer.h>

// This module is loaded by setting LD_PRELOAD when QEMU starts.

namespace {

    using PFN_pthread_create = decltype(&pthread_create);
    using PFN_pthread_exit = decltype(&pthread_exit);

    struct ThreadContext {
        bool in_hook = false;
        bool pending_host_thread_create = false;
        pthread_t *pending_host_thread = nullptr;
        const pthread_attr_t *pending_host_attr = nullptr;
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

    static inline bool is_pass_through() {
        return lore::mod::HostSyscallServer::curSyscallNum == lore::SyscallPathThroughNumber;
    }

    static inline bool is_qemu_thread_create() {
        // QEMU uses pthread_create when handling clone syscalls
        return lore::mod::HostSyscallServer::curSyscallNum == 56;
    }

    static inline bool is_qemu_thread_exit() {
        // QEMU uses pthread_exit when handling exit syscalls (not main thread)
        return lore::mod::HostSyscallServer::curSyscallNum == 60;
    }

#ifdef LORE_ENABLE_ASAN
    static inline bool is_asan_wrapped_start_routine(void *(*start_routine)(void *) ) {
        if (!start_routine) {
            return false;
        }
        Dl_info info = {};
        if (dladdr(reinterpret_cast<void *>(start_routine), &info) == 0) {
            return false;
        }
        if (!info.dli_fname) {
            return false;
        }
        return std::strstr(info.dli_fname, "libasan.so") != nullptr;
    }
#endif

}

extern "C" LORE_DECL_EXPORT int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                                               void *(*start_routine)(void *), void *arg) {
    auto real_pthread_create = resolve_real_pthread_create();

    if (is_qemu_thread_create()) {
        /*
         * QEMU uses pthread_create when handling clone syscalls, and attr is always not NULL.
         */
        if (!attr) {
            abort();
        }

        /*
         * - default guest-origin create: keep QEMU's detached attr.
         * - host-origin reentered create: obey host detach intent.
         */
        if (thread_ctx.pending_host_thread_create) {
            if (thread_ctx.pending_host_attr) {
                // If host attr is not NULL, obey host detach intent.
                int state;
                pthread_attr_getdetachstate(thread_ctx.pending_host_attr, &state);
                if (state == PTHREAD_CREATE_JOINABLE) {
                    std::ignore = pthread_attr_setdetachstate(const_cast<pthread_attr_t *>(attr),
                                                              PTHREAD_CREATE_JOINABLE);
                }
            } else {
                // If attr is NULL, the default intent is joinable.
                std::ignore = pthread_attr_setdetachstate(const_cast<pthread_attr_t *>(attr),
                                                          PTHREAD_CREATE_JOINABLE);
            }

            int ret = real_pthread_create(thread, attr, start_routine, arg);
            *thread_ctx.pending_host_thread = *thread;
            return ret;
        }
        return real_pthread_create(thread, attr, start_routine, arg);
    }

    if (thread_ctx.in_hook) {
        return real_pthread_create(thread, attr, start_routine, arg);
    }

#ifdef LORE_ENABLE_ASAN
    if (is_asan_wrapped_start_routine(start_routine)) {
        // ASAN's pthread interceptor passes an internal wrapper entrypoint.
        // Reentering this path would forward that wrapper into HostRT
        // CC_ThreadEntry and trigger ASAN thread-state assertions.
        return real_pthread_create(thread, attr, start_routine, arg);
    }
#endif

    if (!is_pass_through()) {
        return real_pthread_create(thread, attr, start_routine, arg);
    }

    /*
     * Route non-QEMU thread creation back through HostRT reentry, so
     * thread creation can happen on the guest/QEMU-managed path.
     */
    thread_ctx.in_hook = true;
    thread_ctx.pending_host_thread_create = true;
    thread_ctx.pending_host_thread = thread;
    thread_ctx.pending_host_attr = attr;

    int ret = lore::mod::HostSyscallServer::reenterThreadCreate(
        thread, const_cast<pthread_attr_t *>(attr), reinterpret_cast<void *>(start_routine), arg);

    thread_ctx.pending_host_thread_create = false;
    thread_ctx.pending_host_thread = nullptr;
    thread_ctx.pending_host_attr = nullptr;
    thread_ctx.in_hook = false;
    return ret;
}

extern "C" LORE_DECL_EXPORT void pthread_exit(void *ret) {
    auto real_pthread_exit = resolve_real_pthread_exit();

    if (is_qemu_thread_exit()) {
        real_pthread_exit(ret);
        __builtin_unreachable();
    }

    if (thread_ctx.in_hook) {
        real_pthread_exit(ret);
        __builtin_unreachable();
    }

    if (!is_pass_through()) {
        real_pthread_exit(ret);
        __builtin_unreachable();
    }

    thread_ctx.in_hook = true;
    lore::mod::HostSyscallServer::reenterThreadExit(ret);
    thread_ctx.in_hook = false;

    real_pthread_exit(ret);
    __builtin_unreachable();
}
