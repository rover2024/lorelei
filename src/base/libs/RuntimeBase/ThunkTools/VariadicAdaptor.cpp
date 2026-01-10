#include "VariadicAdaptor.h"

#include <cstdarg>

#include <printf/printf.h>
#include <printf/printf_scanf.h>

namespace lore {

    static int extractPrintFArgs(const char *format, va_list ap, struct LVargEntry *out) {
        char buffer[1];
        (void) mp_vsnprintf(_out_null, buffer, (size_t) -1, format, ap, out);

        struct LVargEntry *p = out;
        while (p->type) {
            p++;
        }
        return p - out;
    }

    static int extractScanFArgs(const char *format, va_list ap, struct LVargEntry *out) {
        char buffer[1];
        (void) mp_vsnprintf_scanf(_out_null, buffer, (size_t) -1, format, ap, out);

        struct LVargEntry *p = out;
        while (p->type) {
            p++;
        }
        return p - out;
    }

    int VariadicAdaptor::extract(FormatStyle style, const char *fmt, va_list ap, LVargEntry *out) {
        switch (style) {
            case FS_printf:
                return extractPrintFArgs(fmt, ap, out);
            case FS_scanf:
                return extractScanFArgs(fmt, ap, out);
            default:
                break;
        }
        return -1;
    }

}

#if 1
#  include "VariadicAdaptor_FFCall.cpp.inc"
#else
#  include "VariadicAdaptor_FFI.cpp.inc"
#endif
