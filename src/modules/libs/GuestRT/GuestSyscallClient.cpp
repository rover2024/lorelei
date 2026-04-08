#include <lorelei/Modules/GuestRT/GuestSyscallClient.h>

#include <pthread.h>
#include <dlfcn.h>

#include <cassert>
#include <cstdint>
#include <tuple>

#include <lorelei/Base/Support/Logging.h>
#include <lorelei/Base/PassThrough/Core/SyscallPassThrough.h>
#include <lorelei/Base/PassThrough/Core/IServer.h>
#include <lorelei/Base/PassThrough/ThunkTools/VariadicAdaptor.h>

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

        static constexpr size_t kMaxSpecialEntries = 64;

        static void *g_specialEntries[kMaxSpecialEntries] = {};

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
            GuestSyscallClient::invokeThreadEntry(entry, hostArg, &ret);
            return ret;
        }

    }

    static inline uint64_t send(uint64_t a1) {
        return syscall1(SyscallPathThroughNumber, a1);
    }

    static inline uint64_t send(uint64_t a1, uint64_t a2) {
        return syscall2(SyscallPathThroughNumber, a1, a2);
    }

    static inline uint64_t send(uint64_t a1, uint64_t a2, uint64_t a3) {
        return syscall3(SyscallPathThroughNumber, a1, a2, a3);
    }

    static inline uint64_t send(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
        return syscall4(SyscallPathThroughNumber, a1, a2, a3, a4);
    }

    static inline uint64_t send(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5) {
        return syscall5(SyscallPathThroughNumber, a1, a2, a3, a4, a5);
    }

    static inline uint64_t send(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5,
                                uint64_t a6) {
        return syscall6(SyscallPathThroughNumber, a1, a2, a3, a4, a5, a6);
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

    void *GuestSyscallClient::convertHostProcAddress(const char *name, void *addr) {
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

        const auto &reversedThunks = thunkInfo.reversed->thunks;
        for (size_t i = 0; i < reversedThunks.size; ++i) {
            const auto &thunk = reversedThunks.data[i];
                auto subThunkInfo = getThunkInfo(thunk.data, false);
                if (!subThunkInfo.forward) {
                    loreWarning("[GRT] %1: failed to get forward thunk info for %2", hostLibPath,
                                thunk.data);
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

    CString GuestSyscallClient::getHostAttribute_impl(const char *key) {
        CString ret = {};
        void *a[] = {
            const_cast<char *>(key),
        };
        std::ignore = send(SPID_GetHostAttribute, reinterpret_cast<uintptr_t>(a),
                           reinterpret_cast<uintptr_t>(&ret));
        return ret;
    }

    void GuestSyscallClient::logMessage_impl(int level, const LogContext &context,
                                             const char *msg) {
        void *a[] = {
            reinterpret_cast<void *>(uintptr_t(level)),
            const_cast<LogContext *>(&context),
            const_cast<char *>(msg),
        };
        std::ignore = send(SPID_LogMessage, reinterpret_cast<uintptr_t>(a), 0);
    }

    void *GuestSyscallClient::loadLibrary_impl(const char *path, int flags) {
        void *ret = nullptr;
        void *a[] = {
            const_cast<char *>(path),
            reinterpret_cast<void *>(uintptr_t(flags)),
        };
        std::ignore = send(SPID_LoadLibrary, reinterpret_cast<uintptr_t>(a),
                           reinterpret_cast<uintptr_t>(&ret));
        return ret;
    }

    int GuestSyscallClient::freeLibrary_impl(void *handle) {
        int ret = 0;
        void *a[] = {
            handle,
        };
        std::ignore = send(SPID_FreeLibrary, reinterpret_cast<uintptr_t>(a),
                           reinterpret_cast<uintptr_t>(&ret));
        return ret;
    }

    void *GuestSyscallClient::getProcAddress_impl(void *handle, const char *name) {
        void *ret = nullptr;
        void *a[] = {
            handle,
            const_cast<char *>(name),
        };
        std::ignore = send(SPID_GetProcAddress, reinterpret_cast<uintptr_t>(a),
                           reinterpret_cast<uintptr_t>(&ret));
        return ret;
    }

    char *GuestSyscallClient::getErrorMessage_impl() {
        char *ret = nullptr;
        std::ignore = send(SPID_GetErrorMessage, 0, reinterpret_cast<uintptr_t>(&ret));
        return ret;
    }

    char *GuestSyscallClient::getModulePath_impl(void *opaque, bool isHandle) {
        char *ret = nullptr;
        void *a[] = {
            opaque,
            reinterpret_cast<void *>(uintptr_t(isHandle)),
        };
        std::ignore = send(SPID_GetModulePath, reinterpret_cast<uintptr_t>(a),
                           reinterpret_cast<uintptr_t>(&ret));
        return ret;
    }

    int GuestSyscallClient::invokeProc_impl(const InvocationArguments *ia) {
        ReentryArguments *ra = nullptr;

        int sysret = static_cast<int>(send(SPID_InvokeProc, reinterpret_cast<uintptr_t>(ia),
                                           reinterpret_cast<uintptr_t>(&ra)));
        while (sysret != 0) {
            assert(sysret == 1 && ra != nullptr);
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
                    int ret = pthread_create(
                        &info.thread, reinterpret_cast<pthread_attr_t *>(ra->threadCreate.attr),
                        newThreadEntry, &info);
                    if (ret == 0) {
                        pthread_cond_wait(&info.cond, &info.mutex);
                    }

                    pthread_mutex_unlock(&info.mutex);
                    pthread_cond_destroy(&info.cond);
                    pthread_mutex_destroy(&info.mutex);
                    *ra->threadCreate.ret = ret;

                    break;
                }

                case SC_ThreadExit: {
                    pthread_exit(ra->threadExit.ret);
                    break;
                }

                default:
                    break;
            }

            sysret = static_cast<int>(send(SPID_ResumeInvocation));
        }
        return 0;
    }

    CThunkInfo GuestSyscallClient::getThunkInfo_impl(const char *path, bool isReverse) {
        CThunkInfo ret = {};
        void *a[] = {
            const_cast<char *>(path),
            reinterpret_cast<void *>(uintptr_t(isReverse)),
        };
        std::ignore = send(SPID_GetThunkInfo, reinterpret_cast<uintptr_t>(a),
                           reinterpret_cast<uintptr_t>(&ret));
        return ret;
    }

    void *GuestSyscallClient::heapAlloc_impl(size_t size) {
        void *ret = nullptr;
        std::ignore = send(SPID_HeapAlloc, size, reinterpret_cast<uintptr_t>(&ret));
        return ret;
    }

    void GuestSyscallClient::heapFree_impl(void *ptr) {
        std::ignore = send(SPID_HeapFree, reinterpret_cast<uintptr_t>(ptr));
    }

    void GuestSyscallClient::setSpecialEntry_impl(int id, void *addr) {
        assert(id > 0);
        assert(static_cast<size_t>(id) < kMaxSpecialEntries);
        g_specialEntries[id] = addr;

        void *a[] = {
            reinterpret_cast<void *>(uintptr_t(id)),
            addr,
        };
        std::ignore = send(SPID_SetSpecialEntry, reinterpret_cast<uintptr_t>(a), 0);
    }

    struct initializer {
        initializer() {
            // TODO
            // GuestSyscallClient::setSpecialEntry(
            //     CE_GetProcAddress, reinterpret_cast<void *>(GuestSyscallClient::getProcAddress));
            // GuestSyscallClient::setSpecialEntry(CE_ReentryLoop,
            //                                     reinterpret_cast<void
            //                                     *>(GuestSyscallClient::reentryLoop));
        }
    };

}
