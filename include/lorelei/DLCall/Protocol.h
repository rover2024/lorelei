// SPDX-License-Identifier: MIT

#ifndef LORE_DLCALL_PROTOCOL_H
#define LORE_DLCALL_PROTOCOL_H

/// \file
/// \brief Wire protocol shared by both ends of the DLCall bridge.
///
/// The guest (\c lore::mod::GuestClient) encodes requests this way and issues them through the
/// magic syscall; the host (\c LoreCommonHostEntry) decodes and serves them. Everything here is
/// plain-old-data so it can cross the guest/host boundary unchanged.

namespace lore {

    /// Magic syscall number the guest issues to reach the host, intercepted by the QEMU \c dlcall
    /// plugin. Must match the number the plugin is configured with (its default is also 4096).
    static constexpr int DLCallSyscallNumber = 4096;

    /// DLCallRequestID - Selects the operation for a DLCall syscall (its first argument). The
    /// library-management IDs are served by the \c dlcall plugin itself; \c DR_InvokeProc calls a
    /// host function pointer, which the runtime aims at the host common entry to reach the
    /// operations in \c DLCallSecondaryID.
    enum DLCallRequestID {
        DR_GetHostAttribute, ///< Query a host attribute by key.
        DR_LoadLibrary,      ///< dlopen a host library.
        DR_GetProcAddress,   ///< dlsym a symbol from a host library.
        DR_FreeLibrary,      ///< dlclose a host library.
        DR_GetLibraryError,  ///< Fetch the last dynamic-linker error (dlerror).
        DR_InvokeProc,       ///< Call a host function pointer; secondary op selected per below.
    };

    /// DLCallSecondaryID - Selects the host-runtime operation carried inside a \c DR_InvokeProc
    /// request whose target is the host common entry (\c LoreCommonHostEntry); passed as that
    /// call's first operand.
    enum DLCallSecondaryID {
        DS_InvokeFunction, ///< Invoke a host function (starts the reentry loop).
        DS_ResumeFunction, ///< Resume a host invocation after the guest serviced a reentry.
        DS_LogMessage,     ///< Forward a guest log record to the host's logging sink.
        DS_GetModulePath,  ///< Resolve the module path of a host handle or address.
        DS_GetThunkInfo,   ///< Look up a thunk-database entry for a library.
    };

    /// ClientCallingConvention - How a host function is ultimately invoked by
    /// \c GuestClient::invokeFunction. This value selects the active \c InvocationArguments member.
    enum ClientCallingConvention {
        /// Standard convention: arguments and the return slot are passed by pointer.
        /// \code
        ///     void (void **args, void *ret, void *metadata)
        /// \endcode
        CC_Standard = 0,

        /// Like \c CC_Standard, but with the callback pointer prepended.
        /// \code
        ///     void (void *callback, void **args, void *ret, void *metadata)
        /// \endcode
        CC_StandardCallback = 1,

        /// Format convention: variadic arguments are marshalled per a printf-style format string.
        /// \sa \c VariadicAdaptor::callFormatBox64
        CC_Format = 2,

        /// Thread-entry convention: \c proc runs as a thread body.
        /// \code
        ///     void *(void *arg)
        /// \endcode
        CC_ThreadEntry = 3,
    };

    /// ServerReentryConvention - How a guest function is invoked during a reentry, driven by
    /// \c HostServer::reenter. This value selects the active \c ReentryArguments member.
    enum ServerReentryConvention {
        /// Standard convention: arguments and the return slot are passed by pointer.
        /// \code
        ///     void (void *proc, void **args, void *ret, void *metadata)
        /// \endcode
        SC_Standard = 0,

        /// Like \c SC_Standard, but with the callback pointer prepended.
        /// \code
        ///     void (void *proc, void *callback, void **args, void *ret, void *metadata)
        /// \endcode
        SC_StandardCallback = 1,

        /// Format convention: variadic arguments are marshalled per a printf-style format string.
        /// \sa \c VariadicAdaptor::callFormatBox64
        SC_Format = 2,

        /// Thread-creation convention: reenter the guest to spawn a thread running \c
        /// start_routine.
        /// \code
        ///     void (void *thread, void *attr, void *start_routine, void *arg, int *ret)
        /// \endcode
        SC_ThreadCreate = 3,

        /// Thread-exit convention: reenter the guest to terminate the current thread.
        /// \code
        ///     void (void *ret)
        /// \endcode
        SC_ThreadExit = 4,
    };

    /// InvocationArguments - Argument block for invoking a host function.
    struct InvocationArguments {
        /// Selects which union member holds the call's operands.
        int conv;

        /// Operands for the selected convention.
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

    /// ReentryArguments - Argument block for a reentry (the host calling back into the guest).
    struct ReentryArguments {
        /// Selects which union member holds the reentry's operands.
        int conv;

        /// Operands for the selected convention.
        struct {
            void *proc;
            void *args;
            void *ret;
            void *metadata;
        } standard;
        struct {
            void *proc;
            void *callback;
            void *args;
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
            void *thread;
            void *attr;
            void *start_routine;
            void *arg;
            int *ret;
        } threadCreate;
        struct {
            void *ret;
        } threadExit;
    };

}

#endif // LORE_DLCALL_PROTOCOL_H
