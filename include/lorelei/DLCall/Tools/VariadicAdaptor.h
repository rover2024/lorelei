// SPDX-License-Identifier: MIT

#ifndef LORE_DLCALL_VARIADICADAPTOR_H
#define LORE_DLCALL_VARIADICADAPTOR_H

#include <cstdarg>

#include <lorelei/DLCall/Global.h>
#include <lorelei/DLCall/Tools/VariadicArgDefs.h>

namespace lore {

    /// VariadicAdaptor - Marshals variadic / dynamically-typed argument lists into typed
    /// \c CVargEntry boxes and replays them as native calls.
    ///
    /// The static C prototype of a function isn't available once a call has crossed the guest/host
    /// boundary, so arguments are carried as a sentinel-terminated array of \c CVargEntry (each a
    /// type tag plus a value). These helpers extract such an array from a \c va_list and re-issue
    /// the call with the platform calling convention.
    class LOREDLCALL_EXPORT VariadicAdaptor {
    public:
        /// Selects how a format string is interpreted when extracting arguments: printf semantics
        /// (values) versus scanf semantics (pointers written through).
        enum FormatStyle {
            PrintF,
            ScanF,
        };

        /// Extract the arguments described by \a fmt out of the \c va_list \a ap into the \a out
        /// \c CVargEntry array (interpreted per \a style). Returns the number of entries written.
        static int extract(FormatStyle style, const char *fmt, va_list ap, CVargEntry *out);

        /// Call \a func with \a argv1 (\a argc1 entries) followed by \a argv2 (\a argc2 entries),
        /// all passed as ordinary positional arguments in order; the result is written to \a ret.
        static void call(void *func, int argc1, CVargEntry *argv1, int argc2, CVargEntry *argv2,
                         CVargEntry *ret);

        /// Like \c call, but \a argv2 is forwarded as a single trailing \c va_list rather than as
        /// individual positional arguments -- i.e. \a func is a variadic function and \a argv1 are
        /// its fixed parameters.
        static void vcall(void *func, int argc1, CVargEntry *argv1, int argc2, CVargEntry *argv2,
                          CVargEntry *ret);

        /// Call \a func using a Box64-style format string that encodes the return type and every
        /// argument type, with \a args supplying the boxed argument values and \a ret receiving the
        /// result.
        ///
        /// Format: \c <ret>_<a1><a2><a3>...  (the return code, an underscore, then one code per
        /// argument), e.g. \c v_iulLpfF. Type codes:
        ///     c: char               C: unsigned char
        ///     s: short              S: unsigned short
        ///     i: int                I: unsigned int
        ///     l: long               u: unsigned long
        ///     L: long long          U: unsigned long long
        ///     f: float              F: double
        ///     p: pointer            v: void (return only)
        static void callFormatBox64(void *func, const char *fmt, void **args, void *ret);
    };

}

#endif // LORE_DLCALL_VARIADICADAPTOR_H
