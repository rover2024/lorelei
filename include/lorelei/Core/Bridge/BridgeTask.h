#ifndef LORECALL_BRIDGETASK_H
#define LORECALL_BRIDGETASK_H

#include <lorelei/Core/Global.h>

namespace lore {

    /// BridgeTask - The task payload carrying task data to be executed by the bridge during an
    /// unfinished invoking process. The invoking process will temporarily return and the bridge
    /// needs to complete the current task and resume the invoking.
    struct BridgeTask {
        enum Type {
            TASK_FUNCTION = 1,
            TASK_CALLBACK,
            TASK_PTHREAD_CREATE,
            TASK_PTHREAD_EXIT,
            TASK_HOST_LIBRARY_OPEN,
        };

        int type;

        union {
            struct {
                void *proc;
                void *args;
                void *ret;
                void *metadata;
            } function;

            struct {
                void *thunk;
                void *callback;
                void *args;
                void *ret;
                void *metadata;
            } callback;

            struct {
                void *thread;
                void *attr;
                void *start_routine;
                void *arg;
                int *ret;
            } pthread_create;

            struct {
                void *ret;
            } pthread_exit;

            struct {
                const char *id;
            } host_library_open;
        };
    };

}

#endif // LORECALL_BRIDGETASK_H
