#ifndef LORE_BASE_PASSTHROUGH_ICLIENT_H
#define LORE_BASE_PASSTHROUGH_ICLIENT_H

#include <array>

#include <lorelei/Base/Support/Logging.h>
#include <lorelei/Base/PassThrough/Core/ThunkInfo.h>

namespace lore {

    /// ClientCallingConvention - Calling conventions for \c Client<T>::invokeProc.
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
        CC_StandardCallback = 1,

        /// \sa \c VariadicAdaptor::callFormatBox64
        CC_Format = 2,

        /// Reserved for thread entry.
        /// \code
        ///     void *(void *arg)
        /// \endcode
        CC_ThreadEntry = 3,
    };

    /// ClientSpecialEntry - Special entry points of guest-side environment.
    enum ClientSpecialEntry {
        /// Get the address of a function in a library.
        CE_GetProcAddress = 1,

        /// Reentry loop of guest-side runtime.
        CE_ReentryLoop,
    };

    /// InvocationArguments - Arguments for invoking a function.
    struct InvocationArguments {
        int conv;
        union {
            struct {
                void *proc;
                void **args;
                void *ret;
                void *metadata;
            } standard;
            struct {
                void *proc;
                void *callback;
                void **args;
                void *ret;
                void *metadata;
            } standardCallback;
            struct {
                void *proc;
                const char *format;
                void **args;
                void *ret;
            } format;
            struct {
                void *proc;
                void *arg;
                void **ret;
            } threadEntry;
        };
    };

    /// IClient - Interface for guest-side client to send invocation request to the the host-side
    /// server.
    template <class T>
    class IClient {
    public:
        /// Get an attribute of the host.
        /// \param key Key of the attribute to query.
        /// \return Optional string value of the attribute, or \c nullptr if the attribute does
        /// not exist.
        static CString getHostAttribute(const char *key) {
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
        /// \param ia The invocation convention and payload.
        static void invokeProc(const InvocationArguments *ia) {
            T::invokeProc_impl(ia);
        }

        /// Get information about a thunk library.
        /// \param path Path or filename of the thunk library to query.
        /// \param isReverse True if you want to get the reverse thunk information.
        static CThunkInfo getThunkInfo(const char *path, bool isReverse) {
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

        /// Set the special entry point of the guest-side runtime.
        /// \param entry Special entry point to set.
        /// \param addr Address of the special entry point.
        static void setSpecialEntry(int id, void *addr) {
            T::setSpecialEntry_impl(id, addr);
        }

    public:
        static inline void invokeStandard(void *proc, void **args, void *ret, void *metadata) {
            InvocationArguments ia;
            ia.conv = CC_Standard;
            ia.standard.proc = proc;
            ia.standard.args = args;
            ia.standard.ret = ret;
            ia.standard.metadata = metadata;
            invokeProc(&ia);
        }

        static inline void invokeStandardCallback(void *proc, void *callback, void **args,
                                                  void *ret, void *metadata) {
            InvocationArguments ia;
            ia.conv = CC_StandardCallback;
            ia.standardCallback.proc = proc;
            ia.standardCallback.callback = callback;
            ia.standardCallback.args = args;
            ia.standardCallback.ret = ret;
            ia.standardCallback.metadata = metadata;
            invokeProc(&ia);
        }

        static inline void invokeFormat(void *proc, const char *format, void **args, void *ret) {
            InvocationArguments ia;
            ia.conv = CC_Format;
            ia.format.proc = proc;
            ia.format.format = format;
            ia.format.args = args;
            ia.format.ret = ret;
            invokeProc(&ia);
        }

        static inline void invokeThreadEntry(void *proc, void *arg, void **ret) {
            InvocationArguments ia;
            ia.conv = CC_ThreadEntry;
            ia.threadEntry.proc = proc;
            ia.threadEntry.arg = arg;
            ia.threadEntry.ret = ret;
            invokeProc(&ia);
        }
    };

}

#endif // LORE_BASE_PASSTHROUGH_ICLIENT_H
