#ifndef LORE_BASE_PASSTHROUGH_VARIADICADAPTOR_H
#define LORE_BASE_PASSTHROUGH_VARIADICADAPTOR_H

#include <cstdarg>

#include <lorelei/Base/PassThrough/c/CVariadicArgs.h>
#include <lorelei/Base/PassThrough/Global.h>

namespace lore {

    class LOREPASSTHROUGH_EXPORT VariadicAdaptor {
    public:
        enum FormatStyle {
            PrintF,
            ScanF,
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

        /// Calls a function with the Box64-style format string.
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
        static void callFormatBox64(void *func, const char *fmt, void **args, void *ret);
    };

}

#endif // LORE_BASE_PASSTHROUGH_VARIADICADAPTOR_H
