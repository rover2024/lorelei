#ifndef LORE_BASE_RUNTIMEBASE_ICLIENT_H
#define LORE_BASE_RUNTIMEBASE_ICLIENT_H

#include <array>

#include <LoreBase/CoreLib/Support/Logging.h>
#include <LoreBase/RuntimeBase/c/LThunkInfo.h>

namespace lore {

    /// \brief Calling conventions for \c Client<T>::invokeProc.
    enum ClientCallingConvention {
        /// The standard calling convention, where all arguments are passed through pointer on
        /// the stack.
        /// \code
        ///     void (void **args, void *ret, void *metadata)
        /// \endcode
        CC_Standard = 0,

        /// The callback pointer is the first argument, followed by the standard arguments.
        /// \code
        ///     void (void *callback, void **args, void *ret, void *metadata)
        /// \endcode
        CC_StandardCallback = 0x10,

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
        CC_Format = 0x20,

        /// Reserved for thread entry.
        /// \code
        ///     void *(void *arg)
        /// \endcode
        CC_ThreadEntry = 0x80,
    };

    /// IClient - Interface for guest-side client to send invocation request to the the host-side server.
    template <class T>
    class IClient {
    public:
        /// Get an attribute of the host.
        /// \param key Key of the attribute to query.
        /// \return Optional string value of the attribute, or \c nullptr if the attribute does
        /// not exist.
        static LString getHostAttribute(const char *key) {
            return T::getHostAttribute_impl(key);
        }

        /// Log a message to the remote server.
        /// \param level Log level.
        /// \param context Context pointer.
        /// \param msg Message to log.
        static void logMessage(int level, const LogContext &context, const char *msg) {
            return T::logMessage_impl(level, context, msg);
        }

        /// Load a library and return a handle to it.
        /// \param path Path to the library to load.
        /// \param flags Flags to pass to the loader.
        /// \return Handle to the loaded library, or NULL if the load failed.
        static void *loadLibrary(const char *path, int flags) {
            return T::loadLibrary_impl(path, flags);
        }

        /// Unload a library and release its resources.
        /// \param handle Handle to the library to unload.
        /// \return 0 if the unload was successful, non-zero otherwise.
        static int freeLibrary(void *handle) {
            return T::freeLibrary_impl(handle);
        }

        /// Get the address of a function in a library.
        /// \param handle Handle to the library to query.
        /// \param name Name of the function to look up.
        /// \return Address of the function, or NULL if the function was not found.
        static void *getProcAddress(void *handle, const char *name) {
            return T::getProcAddress_impl(handle, name);
        }

        /// Get the last error message.
        /// \return Last error message, or NULL if there was no error.
        static char *getErrorMessage() {
            return T::getErrorMessage_impl();
        }

        /// Get the module path for a library handle or a function pointer.
        /// \param opaque Handle or function pointer to query.
        /// \param isHandle True if the opaque parameter is a handle, false if it is a function
        /// pointer.
        /// \return Module path of the library or function, or NULL if the information is not
        /// available.
        static char *getModulePath(void *opaque, bool isHandle) {
            return T::getModulePath_impl(opaque, isHandle);
        }

        /// Invoke a function with a specific calling convention.
        /// \param proc Function pointer to invoke.
        /// \param conv Calling convention to use.
        /// \param opaque Opaque data to pass to the function according to the convention.
        /// \return 0 if the function was invoked successfully, non-zero otherwise.
        static int invokeProc(void *proc, int conv, void *opaque) {
            return T::invokeProc_impl(proc, conv, opaque);
        }

        /// Get information about a thunk library.
        /// \param path Path or filename of the thunk library to query.
        /// \param isReverse True if you want to get the reverse thunk information.
        static LThunkInfo getThunkInfo(const char *path, bool isReverse) {
            return T::getThunkInfo_impl(path, isReverse);
        }

        /// Allocate memory on the heap.
        /// \param size Size of the memory to allocate.
        /// \return Pointer to the allocated memory, or NULL if the allocation failed.
        static void *heapAlloc(size_t size) {
            return T::heapAlloc_impl(size);
        }

        /// Free memory allocated on the heap.
        /// \param ptr Pointer to the memory to free.
        static void heapFree(void *ptr) {
            return T::heapFree_impl(ptr);
        }

    public:
        static inline void invokeStandard(void *proc, void **args, void *ret, void *metadata) {
            void *opaque[] = {(void *) args, ret, metadata};
            invokeProc(proc, CC_Standard, opaque);
        }

        static inline void invokeStandardCallback(void *proc, void *callback, void **args,
                                                  void *ret, void *metadata) {
            void *opaque[] = {callback, (void *) args, ret, metadata};
            invokeProc(proc, CC_StandardCallback, opaque);
        }

        static inline void invokeFormat(void *proc, const char *format, void **args, void *ret) {
            void *opaque[] = {(void *) format, (void *) args, ret};
            invokeProc(proc, CC_Format, opaque);
        }

        static inline void invokeThreadEntry(void *proc, void *arg, void *ret) {
            void *opaque[] = {arg, ret};
            invokeProc(proc, CC_ThreadEntry, opaque);
        }
    };

}

#endif // LORE_BASE_RUNTIMEBASE_ICLIENT_H
