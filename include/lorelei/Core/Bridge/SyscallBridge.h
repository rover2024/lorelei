#ifndef LORECALL_SYSCALLBRIDGE_H
#define LORECALL_SYSCALLBRIDGE_H

#include <lorelei/Core/Bridge/Bridge.h>

namespace lore {

    /// SyscallBridge - The bridge based on system calls.
    template <class T = void>
    class SyscallBridge : public Bridge<T> {
    public:
        /// Unique syscall ID for the remote invoking process.
        static const uint64_t ID = 0x66CCFF;

        /// The sub-ID of the syscall to be invoked.
        enum CallSubID {
            SUBID_CHECK_HEALTH = 1,
            SUBID_LOG_MESSAGE,

            SUBID_LOAD_LIBRARY = 0x101,
            SUBID_FREE_LIBRARY,
            SUBID_GET_PROC_ADDRESS,
            SUBID_GET_ERROR_MESSAGE,
            SUBID_GET_MODULE_PATH,
            SUBID_INVOKE_PROC,
            SUBID_GET_THUNK_INFO,

            /// Reserved for internal use.
            SUBID_RESUME_PROC,
        };

        /// The return value of the remote invoking process.
        enum Return {
            RETURN_ERROR = -1,
            RETURN_OK = 0,
            RETURN_NEXT_TASK,
        };
    };

}

#endif // LORECALL_SYSCALLBRIDGE_H
