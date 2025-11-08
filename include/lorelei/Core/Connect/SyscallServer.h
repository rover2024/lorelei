#ifndef SYSCALLSERVER_H
#define SYSCALLSERVER_H

#include <lorelei/Core/Connect/Server.h>

namespace lore {

    /// SyscallServer - The server based on system calls.
    template <class T = void>
    class SyscallServer : public Server<T> {
    public:
        /// Handle a low-level client request.
        /// \param num The syscall number.
        /// \param a1, a2, a3, a4, a5, a6 The request payload.
        /// \return The return value of the syscall.
        uint64_t dispatch(uint64_t num, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4,
                          uint64_t a5, uint64_t a6) {
            return get()->dispatch_impl(num, a1, a2, a3, a4, a5, a6);
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

#endif // SYSCALLSERVER_H