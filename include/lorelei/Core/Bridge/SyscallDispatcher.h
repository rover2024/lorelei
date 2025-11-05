#ifndef LORECALL_SYSCALLDISPATCHER_H
#define LORECALL_SYSCALLDISPATCHER_H

#include <cstdint>

#include <lorelei/Core/Bridge/BridgeTask.h>

namespace lore {

    /// SyscallDispatcher - Abstract remote dispatcher for handling system calls, used to process
    /// requests from the SyscallBridge instance.
    template <class T = void>
    class SyscallDispatcher {
    public:
        /// Handle a system call request.
        /// \param num The syscall number.
        /// \param a1, a2, a3, a4, a5, a6 The arguments of the syscall.
        /// \return The return value of the syscall.
        uint64_t dispatch(uint64_t num, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4,
                          uint64_t a5, uint64_t a6) {
            return get()->dispatch_impl(num, a1, a2, a3, a4, a5, a6);
        }

        /// Get the current task and then set the task argument.
        /// \param task The task to set.
        BridgeTask *currentTask() const {
            return get()->currentTask_impl();
        }

        /// Run the current task.
        /// \return The return value of the task.
        uint64_t runTask() {
            return get()->runTask_impl();
        }

    public:
        uint64_t runTaskFunction(void *proc, void *args, void *ret, void *metadata) {
            BridgeTask &payload = *currentTask();
            payload.type = BridgeTask::TASK_FUNCTION;

            decltype(BridgeTask::function) &task = payload.function;
            task.proc = proc;
            task.args = args;
            task.ret = ret;
            task.metadata = metadata;
            return runTask();
        }

        uint64_t runTaskCallback(void *thunk, void *callback, void *args, void *ret,
                                 void *metadata) {
            BridgeTask &payload = *currentTask();
            payload.type = BridgeTask::TASK_CALLBACK;

            decltype(BridgeTask::callback) &task = payload.callback;
            task.thunk = thunk;
            task.callback = callback;
            task.args = args;
            task.ret = ret;
            task.metadata = metadata;
            return runTask();
        }

        uint64_t runTaskPthreadCreate(void *thread, void *attr, void *start_routine, void *arg,
                                      int *ret) {
            BridgeTask &payload = *currentTask();
            payload.type = BridgeTask::TASK_PTHREAD_CREATE;

            decltype(BridgeTask::pthread_create) &task = payload.pthread_create;
            task.thread = thread;
            task.attr = attr;
            task.start_routine = start_routine;
            task.arg = arg;
            task.ret = ret;
            return runTask();
        }

        uint64_t runTaskPthreadExit(void *ret) {
            BridgeTask &payload = *currentTask();
            payload.type = BridgeTask::TASK_PTHREAD_EXIT;

            decltype(BridgeTask::pthread_exit) &task = payload.pthread_exit;
            task.ret = ret;
            return runTask();
        }

        uint64_t runTaskHostLibraryOpen(const char *id) {
            BridgeTask &payload = *currentTask();
            payload.type = BridgeTask::TASK_HOST_LIBRARY_OPEN;

            decltype(BridgeTask::host_library_open) &task = payload.host_library_open;
            task.id = id;
            return runTask();
        }

    private:
        T *get() {
            return static_cast<T *>(this);
        }

        const T *get() const {
            return static_cast<const T *>(this);
        }
    };

}

#endif // LORECALL_SYSCALLDISPATCHER_H
