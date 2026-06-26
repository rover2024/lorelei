// SPDX-License-Identifier: MIT

#include "GuestClient.h"

#include <pthread.h>
#include <dlfcn.h>

#include <cassert>
#include <cstdint>
#include <tuple>

#include <lorelei/DLCall/Tools/VariadicAdaptor.h>

#ifdef __x86_64__
#  include "Arch/x86_64/Syscall_x86_64.h"
#elif defined(__aarch64__)
#  include "Arch/aarch64/Syscall_aarch64.h"
#elif defined(__riscv) && __riscv_xlen == 64
#  include "Arch/riscv64/Syscall_riscv64.h"
#else
#  error "Unsupported architecture"
#endif

namespace lore::mod {

    namespace {

        // Host runtime's common entry (its LoreCommonHostEntry). Installed via setCommonHostEntry
        // and used as the target of every DR_InvokeProc request.
        static void *g_commonProcEntry = nullptr;

        struct NewThreadInfo {
            pthread_t thread;
            pthread_mutex_t mutex;
            pthread_cond_t cond;

            void *hostEntry;
            void *hostArg;
        };

        static void *newThreadEntry(void *arg) {
            auto info = (struct NewThreadInfo *) arg;

            void *entry = info->hostEntry;
            void *hostArg = info->hostArg;

            /* Signal to the parent that we're ready.  */
            pthread_mutex_lock(&info->mutex);
            pthread_cond_broadcast(&info->cond);
            pthread_mutex_unlock(&info->mutex);
            /* Wait until the parent has finished initializing the tls state.  */

            void *ret;
            GuestClient::invokeThreadEntry(entry, hostArg, &ret);
            return ret;
        }

    }

    static inline uint64_t send(uint64_t a1) {
        return syscall1(DLCallSyscallNumber, a1);
    }

    static inline uint64_t send(uint64_t a1, uint64_t a2) {
        return syscall2(DLCallSyscallNumber, a1, a2);
    }

    static inline uint64_t send(uint64_t a1, uint64_t a2, uint64_t a3) {
        return syscall3(DLCallSyscallNumber, a1, a2, a3);
    }

    static inline uint64_t send(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
        return syscall4(DLCallSyscallNumber, a1, a2, a3, a4);
    }

    static inline uint64_t send(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5) {
        return syscall5(DLCallSyscallNumber, a1, a2, a3, a4, a5);
    }

    static inline uint64_t send(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5,
                                uint64_t a6) {
        return syscall6(DLCallSyscallNumber, a1, a2, a3, a4, a5, a6);
    }

    // Route a request through the host runtime's common entry. The plugin sees a DR_InvokeProc
    // syscall and calls the entry on our behalf:
    //   syscall(DR_InvokeProc, commonProcEntry, id, payload) -> commonProcEntry(id, payload)
    static inline uint64_t invokeHost(DLCallSecondaryID id, void *payload) {
        return send(static_cast<uint64_t>(DR_InvokeProc),
                    reinterpret_cast<uintptr_t>(g_commonProcEntry), static_cast<uint64_t>(id),
                    reinterpret_cast<uintptr_t>(payload));
    }

    static void *convertHostProcAddress_helper(const char *hostLibPath,
                                               const CForwardThunkInfo &thunkInfo,
                                               const char *name) {
        const auto &path = thunkInfo.guestThunk;
        void *handle = dlopen(path, RTLD_NOW | RTLD_NOLOAD);
        if (!handle) {
            handle = dlopen(path, RTLD_NOW);
            if (!handle) {
                loreWarning("[GRT] %1: failed to load thunk library \"%2\"", hostLibPath, path);
                return nullptr;
            }
        }
        void *func = dlsym(handle, name);
        if (!func) {
            return nullptr;
        }
        return func;
    }

    GuestClient *GuestClient::self = nullptr;

    GuestClient::GuestClient() {
        self = this;
    }

    GuestClient::~GuestClient() {
        self = nullptr;
    }

    void GuestClient::setCommonHostEntry(void *entry) {
        g_commonProcEntry = entry;
    }

    void *GuestClient::convertHostProcAddress(const char *name, void *addr) {
        auto hostLibPath = getModulePath(addr, false);
        if (!hostLibPath) {
            loreWarningF("[GRT] failed to get module path for %p", addr);
            return nullptr;
        }

        auto thunkInfo = getThunkInfo(hostLibPath, true);
        if (!thunkInfo.reversed) {
            // The reverse mapping is not found, assume the guest thunk library has the same name
            thunkInfo = getThunkInfo(hostLibPath, false);
            if (!thunkInfo.forward) {
                loreCritical("[GRT] failed to get forward thunk info for %1", hostLibPath);
                return nullptr;
            }
            return convertHostProcAddress_helper(hostLibPath, *thunkInfo.forward, name);
        }

        const auto *reversedThunks = thunkInfo.reversed->thunks;
        for (size_t i = 0; i < thunkInfo.reversed->thunksCount; ++i) {
            const char *thunk = reversedThunks[i];
            auto subThunkInfo = getThunkInfo(thunk, false);
            if (!subThunkInfo.forward) {
                loreWarning("[GRT] %1: failed to get forward thunk info for %2", hostLibPath, thunk);
                continue;
            }
            void *func = convertHostProcAddress_helper(hostLibPath, *subThunkInfo.forward, name);
            if (!func) {
                continue;
            }
            return func;
        }
        loreWarningF("[GRT] %s: failed to convert host function \"%s\" at %p to guest function",
                     hostLibPath, name, addr);
        return nullptr;
    }

    bool GuestClient::isHostAddressNaive(void *addr) {
        Dl_info info;
        bool isGuestAddr = dladdr(addr, &info);
        return !isGuestAddr;
    }

    const char *GuestClient::getHostAttribute(const char *key) {
        const char *ret = nullptr;
        std::ignore = send(static_cast<uint64_t>(DR_GetHostAttribute),
                           reinterpret_cast<uintptr_t>(key), reinterpret_cast<uintptr_t>(&ret));
        return ret;
    }

    void *GuestClient::loadLibrary(const char *path, int flags) {
        void *ret = nullptr;
        std::ignore = send(static_cast<uint64_t>(DR_LoadLibrary),
                           reinterpret_cast<uintptr_t>(path), static_cast<uint64_t>(flags),
                           reinterpret_cast<uintptr_t>(&ret));
        return ret;
    }

    void *GuestClient::getProcAddress(void *handle, const char *name) {
        void *ret = nullptr;
        std::ignore = send(static_cast<uint64_t>(DR_GetProcAddress),
                           reinterpret_cast<uintptr_t>(handle), reinterpret_cast<uintptr_t>(name),
                           reinterpret_cast<uintptr_t>(&ret));
        return ret;
    }

    int GuestClient::freeLibrary(void *handle) {
        int ret = 0;
        std::ignore = send(static_cast<uint64_t>(DR_FreeLibrary),
                           reinterpret_cast<uintptr_t>(handle), reinterpret_cast<uintptr_t>(&ret));
        return ret;
    }

    char *GuestClient::getLibraryError() {
        char *ret = nullptr;
        std::ignore = send(static_cast<uint64_t>(DR_GetLibraryError),
                           reinterpret_cast<uintptr_t>(&ret));
        return ret;
    }

    void GuestClient::logMessage(int level, const LogContext &context, const char *msg) {
        void *a[] = {
            reinterpret_cast<void *>(uintptr_t(level)),
            const_cast<LogContext *>(&context),
            const_cast<char *>(msg),
        };
        std::ignore = invokeHost(DS_LogMessage, a);
    }

    char *GuestClient::getModulePath(void *opaque, bool isHandle) {
        char *ret = nullptr;
        void *a[] = {
            opaque,
            reinterpret_cast<void *>(uintptr_t(isHandle)),
            &ret,
        };
        std::ignore = invokeHost(DS_GetModulePath, a);
        return ret;
    }

    int GuestClient::invokeFunction(const InvocationArguments *ia) {
        ReentryArguments *ra = nullptr;
        int ret = 0;
        void *opaque[] = {
            const_cast<InvocationArguments *>(ia),
            &ra,
            &ret,
        };

        std::ignore = invokeHost(DS_InvokeFunction, opaque);
        // The host returns 1 whenever it needs the guest to run a reentry (a host-to-guest callback,
        // thread create/exit, ...) before the original call can finish. We execute the reentry
        // described by `ra`, then resume the host; this repeats for nested reentries until it
        // returns 0, meaning the original invocation is complete.
        while (ret != 0) {
            assert(ret == 1 && ra != nullptr);
            switch (ra->conv) {
                case SC_Standard: {
                    using Func = void (*)(void * /*args*/, void * /*ret*/, void * /*metadata*/);

                    auto func = reinterpret_cast<Func>(ra->standard.proc);
                    func(ra->standard.args, ra->standard.ret, ra->standard.metadata);
                    break;
                }

                case SC_StandardCallback: {
                    using Func = void (*)(void * /*callback*/, void * /*args*/, void * /*ret*/,
                                          void * /*metadata*/);

                    auto func = reinterpret_cast<Func>(ra->standardCallback.proc);
                    func(ra->standardCallback.callback, ra->standardCallback.args,
                         ra->standardCallback.ret, ra->standardCallback.metadata);
                    break;
                }

                case SC_Format: {
                    VariadicAdaptor::callFormatBox64(ra->format.proc, ra->format.format,
                                                     ra->format.args, ra->format.ret);
                    break;
                }

                case SC_ThreadCreate: {
                    struct NewThreadInfo info;
                    info.hostEntry = ra->threadCreate.start_routine;
                    info.hostArg = ra->threadCreate.arg;

                    pthread_mutex_init(&info.mutex, NULL);
                    pthread_mutex_lock(&info.mutex);
                    pthread_cond_init(&info.cond, NULL);

                    // Create thread
                    int rc = pthread_create(
                        &info.thread, reinterpret_cast<pthread_attr_t *>(ra->threadCreate.attr),
                        newThreadEntry, &info);
                    if (rc == 0) {
                        pthread_cond_wait(&info.cond, &info.mutex);
                    }

                    pthread_mutex_unlock(&info.mutex);
                    pthread_cond_destroy(&info.cond);
                    pthread_mutex_destroy(&info.mutex);
                    *ra->threadCreate.ret = rc;

                    break;
                }

                case SC_ThreadExit: {
                    pthread_exit(ra->threadExit.ret);
                    break;
                }

                default:
                    break;
            }

            std::ignore = invokeHost(DS_ResumeFunction, &ret);
        }
        return 0;
    }

    CThunkInfo GuestClient::getThunkInfo(const char *path, bool isReverse) {
        CThunkInfo ret = {};
        void *a[] = {
            const_cast<char *>(path),
            reinterpret_cast<void *>(uintptr_t(isReverse)),
            &ret,
        };
        std::ignore = invokeHost(DS_GetThunkInfo, a);
        return ret;
    }

}
