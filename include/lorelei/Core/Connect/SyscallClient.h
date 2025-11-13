#ifndef LORECALL_SYSCALLBRIDGE_H
#define LORECALL_SYSCALLBRIDGE_H

#include <lorelei/Core/Connect/Client.h>

namespace lore {

    /// SyscallClient - The client based on system calls.
    template <class T = void>
    class SyscallClient : public Client<T> {
    public:
        /// Unique syscall ID for the remote invocation.
        static const uint64_t ID = 0x66CCFF;

        /// The request ID in the syscall payload.
        enum RequestID {
            REQUEST_CHECK_CONNECTION = 1,
            REQUEST_LOG_MESSAGE,

            REQUEST_LOAD_LIBRARY = 0x101,
            REQUEST_FREE_LIBRARY,
            REQUEST_GET_PROC_ADDRESS,
            REQUEST_GET_ERROR_MESSAGE,
            REQUEST_GET_MODULE_PATH,
            REQUEST_INVOKE_PROC,
            REQUEST_GET_THUNK_INFO,

            /// Reserved for internal use.
            REQUEST_RESUME_PROC,
        };

        /// The return value of the remote invocation.
        enum Return {
            RETURN_ERROR = -1, // not used now
            RETURN_OK = 0,
            RETURN_NEXT_TASK,
        };
    };

}

#endif // LORECALL_SYSCALLBRIDGE_H
