#ifndef LORECALL_SYSCALLDISPATCHER_H
#define LORECALL_SYSCALLDISPATCHER_H

#include <cstdint>

#include <lorelei/Core/Connect/ClientTask.h>

namespace lore {

    /// Server - Abstract remote server for handling client requests.
    template <class T = void>
    class Server {
    public:
        /// Get the current task and then set the task argument.
        /// \param task The task to set.
        ClientTask *currentTask() const {
            return get()->currentTask_impl();
        }

        /// Run the current task.
        /// \return The return value of the task.
        uint64_t runTask() {
            return get()->runTask_impl();
        }

    public:
        uint64_t runTaskFunction(void *proc, void *args, void *ret, void *metadata) {
            ClientTask &payload = *currentTask();
            payload.id = ClientTask::TASK_FUNCTION;

            decltype(ClientTask::function) &task = payload.function;
            task.proc = proc;
            task.args = args;
            task.ret = ret;
            task.metadata = metadata;
            return runTask();
        }

        uint64_t runTaskCallback(void *thunk, void *callback, void *args, void *ret,
                                 void *metadata) {
            ClientTask &payload = *currentTask();
            payload.id = ClientTask::TASK_CALLBACK;

            decltype(ClientTask::callback) &task = payload.callback;
            task.thunk = thunk;
            task.callback = callback;
            task.args = args;
            task.ret = ret;
            task.metadata = metadata;
            return runTask();
        }

        uint64_t runTaskPthreadCreate(void *thread, void *attr, void *start_routine, void *arg,
                                      int *ret) {
            ClientTask &payload = *currentTask();
            payload.id = ClientTask::TASK_PTHREAD_CREATE;

            decltype(ClientTask::pthread_create) &task = payload.pthread_create;
            task.thread = thread;
            task.attr = attr;
            task.start_routine = start_routine;
            task.arg = arg;
            task.ret = ret;
            return runTask();
        }

        uint64_t runTaskPthreadExit(void *ret) {
            ClientTask &payload = *currentTask();
            payload.id = ClientTask::TASK_PTHREAD_EXIT;

            decltype(ClientTask::pthread_exit) &task = payload.pthread_exit;
            task.ret = ret;
            return runTask();
        }

        uint64_t runTaskHostLibraryOpen(const char *id) {
            ClientTask &payload = *currentTask();
            payload.id = ClientTask::TASK_HOST_LIBRARY_OPEN;

            decltype(ClientTask::host_library_open) &task = payload.host_library_open;
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
