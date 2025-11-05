#ifndef LORECALL_BRIDGE_H
#define LORECALL_BRIDGE_H

#include <array>

#include <lorelei/Core/Bridge/ThunkInfo.h>

namespace lore {

    /// Bridge - Abstract bridge to build arch-specific captibility layer, used to send request to
    /// the the remote dispatcher.
    template <class T = void>
    class Bridge {
    public:
        /// \brief Calling conventions for function invocations.
        enum Convention {
            /// The standard calling convention, where all arguments are passed through pointer on
            /// the stack.
            /// \code
            ///     void (void **args, void *ret, void *metadata)
            /// \endcode
            CONV_STANDARD = 0,

            /// The callback pointer is the first argument, followed by the standard arguments.
            /// \code
            ///     void (void *callback, void **args, void *ret, void *metadata)
            /// \endcode
            CONV_STANDARD_CALLBACK,

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
            ///     void (const char *format, void **args, void *ret)
            /// \endcode
            CONV_FORMAT = 0x20,

            /// Reserved for thread entry.
            /// \code
            ///     void *(void *arg)
            /// \endcode
            CONV_THREAD_ENTRY = 0x80,
        };

    public:
        /// Check if the bridge is healthy and ready to use.
        /// \return 0 if the bridge is healthy, non-zero otherwise.
        int checkHealth() {
            return get()->checkHealth_impl();
        }

        /// Log a message to the remote dispatcher.
        /// \param level Log level.
        /// \param context Context pointer.
        /// \param msg Message to log.
        void logMessage(int level, const void *context, const char *msg) {
            return get()->logMessage_impl(level, context, msg);
        }

        /// Load a library and return a handle to it.
        /// \param path Path to the library to load.
        /// \param flags Flags to pass to the loader.
        /// \return Handle to the loaded library, or NULL if the load failed.
        void *loadLibrary(const char *path, int flags) {
            return get()->loadLibrary_impl(path, flags);
        }

        /// Unload a library and release its resources.
        /// \param handle Handle to the library to unload.
        /// \return 0 if the unload was successful, non-zero otherwise.
        int freeLibrary(void *handle) {
            return get()->freeLibrary_impl(handle);
        }

        /// Get the address of a function in a library.
        /// \param handle Handle to the library to query.
        /// \param name Name of the function to look up.
        /// \return Address of the function, or NULL if the function was not found.
        void *getProcAddress(void *handle, const char *name) {
            return get()->getProcAddress_impl(handle, name);
        }

        /// Get the last error message.
        /// \return Last error message, or NULL if there was no error.
        char *getErrorMessage() {
            return get()->getErrorMessage_impl();
        }

        /// Get the module path for a library handle or a function pointer.
        /// \param opaque Handle or function pointer to query.
        /// \param isHandle True if the opaque parameter is a handle, false if it is a function
        /// pointer.
        /// \return Module path of the library or function, or NULL if the information is not
        /// available.
        char *getModulePath(void *opaque, bool isHandle) {
            return get()->getModulePath_impl(opaque, isHandle);
        }

        /// Invoke a function with a specific calling convention.
        /// \param proc Function pointer to invoke.
        /// \param conv Calling convention to use.
        /// \param opaque Opaque data to pass to the function according to the convention.
        /// \return 0 if the function was invoked successfully, non-zero otherwise.
        int invokeProc(void *proc, int conv, void *opaque) {
            return get()->invokeProc_impl(proc, conv, opaque);
        }

        /// Get information about a thunk library.
        /// \param path Path or filename of the thunk library to query.
        /// \param isReverse True if you want to get the reverse thunk information.
        ThunkInfo getThunkInfo(const char *path, bool isReverse) {
            return get()->getThunkInfo_impl(path, isReverse);
        }

    public:
        inline void invokeStandard(void *proc, void **args, void *ret, void *metadata) {
            void *opaque[] = {(void *) args, ret, metadata};
            invokeProc(proc, CONV_STANDARD, opaque);
        }

        inline void invokeStandardCallback(void *callback, void *proc, void **args, void *ret,
                                           void *metadata) {
            void *opaque[] = {callback, (void *) args, ret, metadata};
            invokeProc(proc, CONV_STANDARD_CALLBACK, opaque);
        }

        inline void invokeFormat(const char *format, void **args, void *ret) {
            void *opaque[] = {(void *) format, (void *) args, ret};
            invokeProc(nullptr, CONV_FORMAT, opaque);
        }

        inline void invokeThreadEntry(void *arg, void *ret) {
            void *opaque[] = {arg, ret};
            invokeProc(nullptr, CONV_THREAD_ENTRY, opaque);
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

#endif // LORECALL_BRIDGE_H
