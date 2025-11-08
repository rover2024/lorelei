#include "GuestClient.h"

#include <pthread.h>
#include <dlfcn.h>

#ifdef __x86_64__
#  include "Arch/x86_64/Syscall_x86_64.h"
#elif defined(__aarch64__)
#  include "Arch/aarch64/Syscall_aarch64.h"
#elif defined(__riscv) && __riscv_xlen == 64
#  include "Arch/riscv64/Syscall_riscv64.h"
#else
#  error "Unsupported architecture"
#endif

#include <lorelei/Core/Connect/ClientTask.h>

namespace lore {

    static inline uint64_t send(uint64_t a1) {
        return syscall1(GuestClient::ID, a1);
    }

    static inline uint64_t send(uint64_t a1, uint64_t a2) {
        return syscall2(GuestClient::ID, a1, a2);
    }

    static inline uint64_t send(uint64_t a1, uint64_t a2, uint64_t a3) {
        return syscall3(GuestClient::ID, a1, a2, a3);
    }

    static inline uint64_t send(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
        return syscall4(GuestClient::ID, a1, a2, a3, a4);
    }

    static inline uint64_t send(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5) {
        return syscall5(GuestClient::ID, a1, a2, a3, a4, a5);
    }

    static inline uint64_t send(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5,
                                uint64_t a6) {
        return syscall6(GuestClient::ID, a1, a2, a3, a4, a5, a6);
    }

    struct thread_info {
        GuestClient *client;

        pthread_t thread;
        pthread_mutex_t mutex;
        pthread_cond_t cond;

        void *host_entry;
        void *host_arg;
    };

    static void *theThreadEntry(void *arg) {
        auto info = (struct thread_info *) arg;

        GuestClient *client = info->client;
        void *entry = info->host_entry;
        void *hostArg = info->host_arg;

        /* Signal to the parent that we're ready.  */
        pthread_mutex_lock(&info->mutex);
        pthread_cond_broadcast(&info->cond);
        pthread_mutex_unlock(&info->mutex);
        /* Wait until the parent has finished initializing the tls state.  */

        void *ret;
        void *opaque[] = {
            hostArg,
            &ret,
        };
        client->invokeProc(entry, GuestClient::CONV_THREAD_ENTRY, opaque);
        return ret;
    }

    static GuestClient *m_instance = nullptr;

    GuestClient::GuestClient() {
        if (m_instance) {
            fprintf(stderr, "GuestClient can only be instantiated once!!!");
            std::abort();
        }
        m_instance = this;
    }

    GuestClient::~GuestClient() {
        m_instance = nullptr;
    }

    GuestClient *GuestClient::instance() {
        return m_instance;
    }

    int GuestClient::checkConnection_impl() {
        return send(REQUEST_CHECK_CONNECTION);
    }

    void GuestClient::logMessage_impl(int level, const void *context, const char *msg) {
        send(REQUEST_LOG_MESSAGE, level, (uintptr_t) context, (uintptr_t) msg);
    }

    void *GuestClient::loadLibrary_impl(const char *path, int flags) {
        void *ret = nullptr;
        void *a[] = {
            const_cast<char *>(path),
            reinterpret_cast<void *>(uintptr_t(flags)),
        };
        std::ignore = send(REQUEST_LOAD_LIBRARY, (uintptr_t) a, (uintptr_t) ret);
        return ret;
    }

    int GuestClient::freeLibrary_impl(void *handle) {
        int ret = 0;
        void *a[] = {
            handle,
        };
        std::ignore = send(REQUEST_FREE_LIBRARY, (uintptr_t) a, (uintptr_t) ret);
        return ret;
    }

    void *GuestClient::getProcAddress_impl(void *handle, const char *name) {
        void *ret = nullptr;
        void *a[] = {
            handle,
            const_cast<char *>(name),
        };
        std::ignore = send(REQUEST_GET_PROC_ADDRESS, (uintptr_t) a, (uintptr_t) ret);
        return ret;
    }

    char *GuestClient::getErrorMessage_impl() {
        char *ret = nullptr;
        std::ignore = send(REQUEST_GET_ERROR_MESSAGE, 0, (uintptr_t) ret);
        return ret;
    }

    char *GuestClient::getModulePath_impl(void *opaque, bool isHandle) {
        char *ret = nullptr;
        void *a[] = {
            opaque,
            reinterpret_cast<void *>(uintptr_t(isHandle)),
        };
        std::ignore = send(REQUEST_GET_MODULE_PATH, (uintptr_t) a, (uintptr_t) ret);
        return ret;
    }

    int GuestClient::invokeProc_impl(void *proc, int conv, void *opaque) {
        ClientTask next_task;

        // Call and wait for the potential callback SyscallClient
        uint64_t syscall_ret = send(REQUEST_INVOKE_PROC, (uintptr_t) proc, conv, (uintptr_t) opaque,
                                    (uintptr_t) &next_task);
        while (syscall_ret != RETURN_NEXT_TASK) {
            switch (next_task.id) {
                case ClientTask::TASK_FUNCTION: {
                    typedef void (*FunctionThunk)(void * /*args*/, void * /*ret*/,
                                                  void * /*metadata*/);
                    auto thunk = (FunctionThunk) next_task.function.proc;
                    void *args = next_task.function.args;
                    void *ret = next_task.function.ret;
                    void *metadata = next_task.function.metadata;
                    thunk(args, ret, metadata);
                    break;
                }

                case ClientTask::TASK_CALLBACK: {
                    typedef void (*CallbackThunk)(void * /*callback*/, void * /*args*/,
                                                  void * /*ret*/, void * /*metadata*/);
                    auto thunk = (CallbackThunk) next_task.callback.thunk;
                    void *callback = next_task.callback.callback;
                    void *args = next_task.callback.args;
                    void *ret = next_task.callback.ret;
                    void *metadata = next_task.callback.metadata;
                    thunk(callback, args, ret, metadata);
                    break;
                }

                case ClientTask::TASK_PTHREAD_CREATE: {
                    auto attr = (pthread_attr_t *) next_task.pthread_create.attr;
                    void *host_start_routine = next_task.pthread_create.start_routine;
                    void *host_arg = next_task.pthread_create.arg;
                    int *ret_ref = next_task.pthread_create.ret;

                    struct thread_info info;
                    info.client = this;
                    info.host_entry = host_start_routine;
                    info.host_arg = host_arg;
                    pthread_mutex_init(&info.mutex, NULL);
                    pthread_mutex_lock(&info.mutex);
                    pthread_cond_init(&info.cond, NULL);

                    // Create thread
                    int ret = pthread_create(&info.thread, attr, theThreadEntry, &info);
                    if (ret == 0) {
                        pthread_cond_wait(&info.cond, &info.mutex);
                    }

                    pthread_mutex_unlock(&info.mutex);
                    pthread_cond_destroy(&info.cond);
                    pthread_mutex_destroy(&info.mutex);
                    *ret_ref = ret;

                    // Host pthread id will be set in host runtime
                    break;
                }

                case ClientTask::TASK_PTHREAD_EXIT: {
                    void *ret = next_task.pthread_exit.ret;
                    pthread_exit(ret);
                    break;
                }

                case ClientTask::TASK_HOST_LIBRARY_OPEN: {
                    const char *identifier = next_task.host_library_open.id;
                    ThunkInfo info = GuestClient::getThunkInfo(identifier, true);
                    if (!info.forward) {
                        break;
                    }
                    (void) dlopen(info.forward->guestThunk.c_str(), RTLD_NOW);
                    break;
                }

                default:
                    break;
            }

            // Notify host to continue and wait for the next callback SyscallClient
            syscall_ret = send(REQUEST_RESUME_PROC);
        }

        if (syscall_ret == RETURN_ERROR) {
            return -1;
        }
        return 0;
    }

    ThunkInfo GuestClient::getThunkInfo_impl(const char *path, bool isReverse) {
        CThunkInfo *ret;
        void *a[] = {
            const_cast<char *>(path),
            reinterpret_cast<void *>(uintptr_t(isReverse)),
        };
        std::ignore = send(REQUEST_GET_THUNK_INFO, (uintptr_t) a, (uintptr_t) ret);
        return ThunkInfo::fromCThunkInfo(ret);
    }

}