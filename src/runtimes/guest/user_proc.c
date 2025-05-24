#include "loreuser.h"

#include <dlfcn.h>
#include <pthread.h>

#include <stddef.h>
#include <stdio.h>
#include <unistd.h>

#include "syscall_helper.h"

LORELEI_DECL_EXPORT __thread void *Lore_GRTThreadCallback;

struct thread_info {
    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_cond_t cond;

    void *host_entry;
    void *host_arg;
};

static void *ThreadEntry(void *arg) {
    struct thread_info *info = arg;

    void *entry = info->host_entry;
    void *hostArg = info->host_arg;

    /* Signal to the parent that we're ready.  */
    pthread_mutex_lock(&info->mutex);
    pthread_cond_broadcast(&info->cond);
    pthread_mutex_unlock(&info->mutex);
    /* Wait until the parent has finished initializing the tls state.  */

    void *ret;
    Lore_InvokeHostProc(LOREUSER_PC_ThreadEntry, entry, hostArg, &ret, NULL);
    return ret;
}

static void HostProcLoop(void *a) {
    union LOREUSER_PROC_NEXTCALL next_call;

    // Call and wait for the potential callback request
    uint64_t syscall_ret = syscall3(LOREUSER_SYSCALL_NUM, (void *) LOREUSER_CT_InvokeHostProc, a, &next_call);
    while (syscall_ret != LOREUSER_PR_Finished) {
        switch (syscall_ret) {
            case LOREUSER_PR_Callback: {
                typedef void (*CallbackThunk)(void * /*callback*/, void * /*args*/, void * /*ret*/,
                                              void * /*metadata*/);
                CallbackThunk thunk = next_call.callback.thunk;
                void *callback = next_call.callback.callback;
                void *args = next_call.callback.args;
                void *ret = next_call.callback.ret;
                void *metadata = next_call.callback.metadata;
                thunk(callback, args, ret, metadata);
                break;
            }

            case LOREUSER_PR_PThreadCreate: {
                pthread_attr_t *attr = next_call.pthread_create.attr;
                void *host_start_routine = next_call.pthread_create.start_routine;
                void *host_arg = next_call.pthread_create.arg;
                int *ret_ref = next_call.pthread_create.ret;

                struct thread_info info;
                info.host_entry = host_start_routine;
                info.host_arg = host_arg;
                pthread_mutex_init(&info.mutex, NULL);
                pthread_mutex_lock(&info.mutex);
                pthread_cond_init(&info.cond, NULL);

                // Create thread
                int ret = pthread_create(&info.thread, attr, ThreadEntry, &info);
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

            case LOREUSER_PR_PThreadExit: {
                void *ret = next_call.pthread_exit.ret;
                pthread_exit(ret);
                break;
            }

            case LOREUSER_PR_HostLibraryOpen: {
                const char *identifier = next_call.host_library_open.id;
                struct LORE_THUNK_LIBRARY_DATA *lib_data = Lore_GetLibraryData(identifier, true);
                if (!lib_data) {
                    break;
                }
                (void) dlopen(lib_data->gtl, RTLD_NOW);
                break;
            }

            default:
                break;
        }

        // Notify host to continue and wait for the next callback request
        syscall_ret = syscall1(LOREUSER_SYSCALL_NUM, (void *) LOREUSER_CT_ResumeHostProc);
    }
}

void Lore_InvokeHostProc(int convention, void *func, void **args, void *ret, void **metadata) {
    void *a[] = {
        (void *) (intptr_t) convention, func, args, ret, metadata,
    };
    HostProcLoop(a);
}