#ifndef LORE_BASE_PASSTHROUGH_ISERVER_H
#define LORE_BASE_PASSTHROUGH_ISERVER_H

#include <type_traits>

namespace lore {

    /// \brief Reentry conventions for \c Server<T>::reenter.
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
        SC_StandardCallback = 1,

        /// \sa \c VariadicAdaptor::callFormatBox64
        SC_Format = 2,

        /// Reserved for thread creation.
        /// \code
        ///     void (void *thread, void *attr, void *start_routine, void *arg, int *ret)
        /// \endcode
        SC_ThreadCreate = 3,

        /// Reserved for thread exit.
        /// \code
        ///     void (void *ret)
        /// \endcode
        SC_ThreadExit = 4,
    };

    /// ReentryArguments - Arguments for a reentry.
    struct ReentryArguments {
        int conv;
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

    template <class T>
    class IServer {
    public:
        /// Invoke a reentry with a specific calling convention.
        /// \param conv Calling convention to use.
        /// \param args Arguments for invoking the function.
        static void reenter(ReentryArguments *ra) {
            T::reenter_impl(ra);
        }

    public:
        static inline void reenterStandard(void *proc, void **args, void *ret, void *metadata) {
            ReentryArguments a;
            a.conv = SC_Standard;
            a.standard.proc = proc;
            a.standard.args = (void *) args;
            a.standard.ret = ret;
            a.standard.metadata = metadata;
            reenter(&a);
        }

        static inline void reenterStandardCallback(void *proc, void *callback, void **args,
                                                   void *ret, void *metadata) {
            ReentryArguments a;
            a.conv = SC_StandardCallback;
            a.standardCallback.proc = proc;
            a.standardCallback.callback = callback;
            a.standardCallback.args = (void *) args;
            a.standardCallback.ret = ret;
            a.standardCallback.metadata = metadata;
            reenter(&a);
        }

        static inline void reenterFormat(void *proc, const char *format, void **args, void *ret) {
            ReentryArguments a;
            a.conv = SC_Format;
            a.format.proc = proc;
            a.format.format = format;
            a.format.args = args;
            a.format.ret = ret;
            reenter(&a);
        }

        static inline int reenterThreadCreate(void *thread, void *attr, void *start_routine,
                                              void *arg) {
            int ret;
            ReentryArguments a;
            a.conv = SC_ThreadCreate;
            a.threadCreate.thread = thread;
            a.threadCreate.attr = attr;
            a.threadCreate.start_routine = start_routine;
            a.threadCreate.arg = arg;
            a.threadCreate.ret = &ret;
            reenter(&a);
            return ret;
        }

        static inline void reenterThreadExit(void *ret) {
            ReentryArguments a;
            a.conv = SC_ThreadExit;
            a.threadExit.ret = ret;
            reenter(&a);
        }
    };
}

#endif // LORE_BASE_PASSTHROUGH_ISERVER_H
