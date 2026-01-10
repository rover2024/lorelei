#ifndef LORE_BASE_RUNTIMEBASE_VARIADICADAPTOR_H
#define LORE_BASE_RUNTIMEBASE_VARIADICADAPTOR_H

#include <LoreBase/RuntimeBase/c/LVariadicArgs.h>
#include <LoreBase/RuntimeBase/Global.h>

namespace lore {

    class LORERTBASE_EXPORT VariadicAdaptor {
    public:
        enum FormatStyle {
            FS_printf,
            FS_scanf,
        };

        /// Extracts the arguments from a variadic argument list and stores them in a
        /// \c LVargEntry array.
        static int extract(FormatStyle style, const char *fmt, va_list ap, LVargEntry *out);

        /// Calls a function, the arguments are passed in order.
        static void call(void *func, int argc1, LVargEntry *argv1, int argc2, LVargEntry *argv2,
                         LVargEntry *ret);

        /// Calls a function, the \a argv1 is passed as arguments in order and \a argv2 is packed in
        /// a \c va_list as the last argument.
        static void vcall(void *func, int argc1, LVargEntry *argv1, int argc2, LVargEntry *argv2,
                          LVargEntry *ret);
    };

}

#endif // LORE_BASE_RUNTIMEBASE_VARIADICADAPTOR_H
