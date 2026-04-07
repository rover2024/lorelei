#ifndef LORE_BASE_PASSTHROUGH_VARIADICADAPTOR_H
#define LORE_BASE_PASSTHROUGH_VARIADICADAPTOR_H

#include <cstdarg>

#include <lorelei/Base/PassThrough/c/CVariadicArgs.h>
#include <lorelei/Base/PassThrough/Global.h>

namespace lore {

    class LOREPASSTHROUGH_EXPORT VariadicAdaptor {
    public:
        enum FormatStyle {
            FS_printf,
            FS_scanf,
        };

        /// Extracts the arguments from a variadic argument list and stores them in a
        /// \c CVargEntry array.
        static int extract(FormatStyle style, const char *fmt, va_list ap, CVargEntry *out);

        /// Calls a function, the arguments are passed in order.
        static void call(void *func, int argc1, CVargEntry *argv1, int argc2, CVargEntry *argv2,
                         CVargEntry *ret);

        /// Calls a function, the \a argv1 is passed as arguments in order and \a argv2 is packed in
        /// a \c va_list as the last argument.
        static void vcall(void *func, int argc1, CVargEntry *argv1, int argc2, CVargEntry *argv2,
                          CVargEntry *ret);
    };

}

#endif // LORE_BASE_PASSTHROUGH_VARIADICADAPTOR_H
