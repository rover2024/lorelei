#include "loreuser.h"

#include <pthread.h>

#include <stddef.h>
#include <stdio.h>
#include <unistd.h>

#include "syscall_helper.h"

struct thread_info {
    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_cond_t cond;

    void *host_entry;
    void *host_args;
};

static void *ThreadEntry(void *args) {
    struct thread_info *info = args;

    void *entry = info->host_entry;
    void *arg = info->host_args;

    /* Signal to the parent that we're ready.  */
    pthread_mutex_lock(&info->mutex);
    pthread_cond_broadcast(&info->cond);
    pthread_mutex_unlock(&info->mutex);
    /* Wait until the parent has finished initializing the tls state.  */

    void *ret;
    Lore_InvokeHostProc(entry, arg, &ret, LOREUSER_PC_ThreadEntry, NULL);
    return ret;
}

static void HostProcLoop(void *a) {
    void *next_call[8];

    // Call and wait for the potential callback request
    uint64_t syscall_ret = syscall3(LOREUSER_SYSCALL_NUM, (void *) LOREUSER_CT_InvokeHostProc, a, next_call);
    while (syscall_ret != LOREUSER_PR_Finished) {
        switch (syscall_ret) {
            case LOREUSER_PR_Callback: {
                typedef void (*CallbackThunk)(void * /*callback*/, void * /*args*/, void * /*ret*/);
                CallbackThunk thunk = next_call[0];
                void *callback = next_call[1];
                void *args = next_call[2];
                void *ret = next_call[3];
                thunk(callback, args, ret);
                break;
            }

            case LOREUSER_PR_PThreadCreate: {
                pthread_attr_t *attr = next_call[1];
                void *host_start_routine = next_call[2];
                void *host_args = next_call[3];
                int *ret_ref = next_call[4];

                struct thread_info info;
                info.host_entry = host_start_routine;
                info.host_args = host_args;
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
                void *ret = next_call[0];
                pthread_exit(ret);
                break;
            }

            default:
                break;
        }

        // Notify host to continue and wait for the next callback request
        syscall_ret = syscall1(LOREUSER_SYSCALL_NUM, (void *) LOREUSER_CT_ResumeHostProc);
    }
}

void Lore_InvokeHostProc(void *func, void **args, void *ret, int convention, void **metadata) {
    void *a[] = {
        func, args, ret, (void *) (intptr_t) convention, metadata,
    };
    HostProcLoop(a);
}