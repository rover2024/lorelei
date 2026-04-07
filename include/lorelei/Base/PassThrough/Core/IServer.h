#ifndef LORE_BASE_PASSTHROUGH_ISERVER_H
#define LORE_BASE_PASSTHROUGH_ISERVER_H

#include <type_traits>

namespace lore {

    /// \brief Reentry conventions for \c Server<T>::invokeCallback.
    enum ServerReentryConvention {
        /// The standard calling convention, where all arguments are passed through pointer on
        /// the stack.
        /// \code
        ///     void (void *proc, void **args, void *ret, void *metadata)
        /// \endcode
        SC_Standard = 0,

        /// The callback pointer is the first argument, followed by the standard arguments.
        /// \code
        ///     void (void *proc, void *callback, void **args, void *ret, void *metadata)
        /// \endcode
        SC_StandardCallback = 0x10,

        /// The printf-like calling convention, where the first argument is a format string,
        /// followed by the arguments.
        ///
        /// Format: <ret>_<a1><a2><a3>...
        /// \example v_iulLpfF
        ///     c: char
        ///     C: unsigned char
        ///     s: short
        ///     S: unsigned short
        ///     i: int
        ///     I: unsigned int
        ///     l: long
        ///     u: unsigned long
        ///     L: long long
        ///     U: unsigned long long
        ///     f: float
        ///     F: double
        ///     p: pointer
        ///     v: void (return only)
        ///
        /// \code
        ///     void (void *proc, const char *format, void **args, void *ret)
        /// \endcode
        SC_Format = 0x20,

        /// Reserved for thread creation.
        /// \code
        ///     void (void *thread, void *attr, void *start_routine, void *arg, int *ret)
        /// \endcode
        SC_ThreadCreate = 0x80,

        /// Reserved for thread exit.
        /// \code
        ///     void (void *ret)
        /// \endcode
        SC_ThreadExit = 0x81,
    };

    template <class T>
    class IServer {
    public:
        /// Invoke a reentry with a specific calling convention.
        /// \param proc Function pointer to invoke.
        /// \param conv Calling convention to use.
        /// \param opaque Opaque data to pass to the function according to the convention.
        /// \return 0 if the function was invoked successfully, non-zero otherwise.
        static int invokeReentry(void *proc, int conv, void *opaque) {
            return T::invokeReentry_impl(proc, conv, opaque);
        }

    public:
        static inline void invokeStandard(void *proc, void **args, void *ret, void *metadata) {
            void *opaque[] = {(void *) args, ret, metadata};
            invokeReentry(proc, SC_Standard, opaque);
        }

        static inline void invokeStandardCallback(void *proc, void *callback, void **args,
                                                  void *ret, void *metadata) {
            void *opaque[] = {callback, (void *) args, ret, metadata};
            invokeReentry(proc, SC_StandardCallback, opaque);
        }

        static inline void invokeFormat(void *proc, const char *format, void **args, void *ret) {
            void *opaque[] = {(void *) format, (void *) args, ret};
            invokeReentry(proc, SC_Format, opaque);
        }

        static inline int invokeThreadCreate(void *thread, void *attr, void *start_routine,
                                             void *arg) {
            int ret;
            void *opaque[] = {thread, attr, start_routine, arg, &ret};
            invokeReentry(thread, SC_ThreadCreate, opaque);
            return ret;
        }

        static inline void invokeThreadExit(void *ret) {
            invokeReentry(ret, SC_ThreadExit, nullptr);
        }
    };

}

#endif // LORE_BASE_PASSTHROUGH_ISERVER_H
