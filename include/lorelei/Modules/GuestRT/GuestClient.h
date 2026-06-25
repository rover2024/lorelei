#ifndef LORE_MODULES_GUESTRT_GUESTCLIENT_H
#define LORE_MODULES_GUESTRT_GUESTCLIENT_H

#include <lorelei/Support/Logging.h>
#include <lorelei/DLCall/Protocol.h>
#include <lorelei/DLCall/ThunkDatabase.h>
#include <lorelei/Modules/GuestRT/Global.h>

namespace lore::mod {

    /// GuestClient - Guest-side endpoint of the DLCall bridge.
    ///
    /// Runs inside the emulated guest and reaches the host by issuing the DLCall magic syscall
    /// (\c DLCallSyscallNumber), which the QEMU \c dlcall plugin intercepts. The library-management
    /// primitives (\c getHostAttribute, \c loadLibrary, \c getProcAddress, \c freeLibrary,
    /// \c getLibraryError) are served by the plugin directly. Everything else (\c logMessage,
    /// \c getModulePath, \c invokeFunction, \c getThunkInfo) is forwarded as a \c DR_InvokeProc
    /// request to the host runtime's common entry, so that entry must be installed with
    /// \c setCommonHostEntry before any of those calls are made.
    class LOREGUESTRT_EXPORT GuestClient {
    public:
        GuestClient();
        ~GuestClient();

        /// The singleton instance. Constructed at host runtime startup.
        static inline GuestClient *instance() {
            return self;
        }

        /// Install the host runtime's common entry. Called at guest runtime startup. Every
        /// \c DR_InvokeProc request is routed through it, so it must be set before
        /// \c invokeFunction (and the helpers built on it).
        static void setCommonHostEntry(void *entry);

    public:
        /// Map a host function address back to a callable guest (thunk) address. Used to implement
        /// \c *GetProcAddress* -style APIs that hand the guest a raw host pointer.
        static void *convertHostProcAddress(const char *name, void *addr);

    public:
        /// Query a host attribute by key (e.g. \c "emu"). Served by the dlcall plugin.
        static const char *getHostAttribute(const char *key);

        /// Load a host library; returns an opaque host handle, or \c nullptr on failure. Wraps the
        /// host's \c dlopen with the given \a flags.
        static void *loadLibrary(const char *path, int flags);

        /// Resolve a symbol from a host library. A \c nullptr \a handle searches the host's default
        /// scope. Returns \c nullptr if not found. Wraps the host's \c dlsym.
        static void *getProcAddress(void *handle, const char *name);

        /// Release a host library handle. Wraps the host's \c dlclose.
        static int freeLibrary(void *handle);

        /// The host's last dynamic-linker error string, or \c nullptr. Wraps the host's \c dlerror.
        static char *getLibraryError();

    public:
        /// Forward a log record to the host so guest logs land in the host's logging sink.
        static void logMessage(int level, const LogContext &context, const char *msg);

        /// Resolve the filesystem path of a host module. \a opaque is a library handle when
        /// \a isHandle is true, otherwise a function address inside the module.
        static char *getModulePath(void *opaque, bool isHandle);

        /// Invoke a host function described by \a args, driving the reentry loop (host-to-guest
        /// callbacks, thread create/exit) until the call completes. Prefer the typed \c invoke*
        /// helpers below.
        static int invokeFunction(const InvocationArguments *args);

        /// Look up the thunk-database entry for a library \a path. \a isReverse selects the
        /// reversed (host-to-guest) mapping instead of the forward one.
        static CThunkInfo getThunkInfo(const char *path, bool isReverse);

    public:
        static inline void invokeStandard(void *proc, void **args, void *ret, void *metadata) {
            InvocationArguments ia;
            ia.conv = CC_Standard;
            ia.standard.proc = proc;
            ia.standard.args = args;
            ia.standard.ret = ret;
            ia.standard.metadata = metadata;
            invokeFunction(&ia);
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
            invokeFunction(&ia);
        }

        static inline void invokeFormat(void *proc, const char *format, void **args, void *ret) {
            InvocationArguments ia;
            ia.conv = CC_Format;
            ia.format.proc = proc;
            ia.format.format = format;
            ia.format.args = args;
            ia.format.ret = ret;
            invokeFunction(&ia);
        }

        static inline void invokeThreadEntry(void *proc, void *arg, void **ret) {
            InvocationArguments ia;
            ia.conv = CC_ThreadEntry;
            ia.threadEntry.proc = proc;
            ia.threadEntry.arg = arg;
            ia.threadEntry.ret = ret;
            invokeFunction(&ia);
        }

    protected:
        static GuestClient *self;
    };

}

#endif // LORE_MODULES_GUESTRT_GUESTCLIENT_H
