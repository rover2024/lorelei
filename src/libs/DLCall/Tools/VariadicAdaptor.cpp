#include "VariadicAdaptor.h"

#include <cstdarg>
#include <cassert>

#include <printf/printf.h>
#include <printf/printf_scanf.h>

#include <llvm/ADT/SmallVector.h>

namespace lore {

    static void callFormatBox64_arg_entry(char fmt, CVargEntry *entry, void *arg) {
        switch (fmt) {
            case 'c':
                entry->type = CVargType_Char;
                entry->c = *reinterpret_cast<char *>(arg);
                break;
            case 'C':
                entry->type = CVargType_UChar;
                entry->uc = *reinterpret_cast<unsigned char *>(arg);
                break;
            case 's':
                entry->type = CVargType_Short;
                entry->s = *reinterpret_cast<short *>(arg);
                break;
            case 'S':
                entry->type = CVargType_UShort;
                entry->us = *reinterpret_cast<unsigned short *>(arg);
                break;
            case 'i':
                entry->type = CVargType_Int;
                entry->i = *reinterpret_cast<int *>(arg);
                break;
            case 'I':
                entry->type = CVargType_UInt;
                entry->u = *reinterpret_cast<unsigned int *>(arg);
                break;
            case 'l':
                entry->type = CVargType_Long;
                entry->l = *reinterpret_cast<long *>(arg);
                break;
            case 'u':
                entry->type = CVargType_ULong;
                entry->ul = *reinterpret_cast<unsigned long *>(arg);
                break;
            case 'L':
                entry->type = CVargType_LongLong;
                entry->ll = *reinterpret_cast<long long *>(arg);
                break;
            case 'U':
                entry->type = CVargType_ULongLong;
                entry->ull = *reinterpret_cast<unsigned long long *>(arg);
                break;
            case 'f':
                entry->type = CVargType_Float;
                entry->f = *reinterpret_cast<float *>(arg);
                break;
            case 'F':
                entry->type = CVargType_Double;
                entry->d = *reinterpret_cast<double *>(arg);
                break;
            case 'p':
                entry->type = CVargType_Pointer;
                entry->p = *reinterpret_cast<void **>(arg);
                break;
            default:
                entry->type = CVargType_Void;
                break;
        }
    }

    static void callFormatBox64_ret_entry(char fmt, CVargEntry *entry) {
        switch (fmt) {
            case 'c':
                entry->type = CVargType_Char;
                break;
            case 'C':
                entry->type = CVargType_UChar;
                break;
            case 's':
                entry->type = CVargType_Short;
                break;
            case 'S':
                entry->type = CVargType_UShort;
                break;
            case 'i':
                entry->type = CVargType_Int;
                break;
            case 'I':
                entry->type = CVargType_UInt;
                break;
            case 'l':
                entry->type = CVargType_Long;
                break;
            case 'u':
                entry->type = CVargType_ULong;
                break;
            case 'L':
                entry->type = CVargType_LongLong;
                break;
            case 'U':
                entry->type = CVargType_ULongLong;
                break;
            case 'f':
                entry->type = CVargType_Float;
                break;
            case 'F':
                entry->type = CVargType_Double;
                break;
            case 'p':
                entry->type = CVargType_Pointer;
                break;
            default:
                entry->type = CVargType_Void;
                break;
        }
    }

    static int extractPrintFArgs(const char *format, va_list ap, CVargEntry *out) {
        char buffer[1];
        (void) mp_vsnprintf(_out_null, buffer, (size_t) -1, format, ap, out);

        CVargEntry *p = out;
        while (p->type) {
            p++;
        }
        return p - out;
    }

    static int extractScanFArgs(const char *format, va_list ap, CVargEntry *out) {
        char buffer[1];
        (void) mp_vsnprintf_scanf(_out_null, buffer, (size_t) -1, format, ap, out);

        CVargEntry *p = out;
        while (p->type) {
            p++;
        }
        return p - out;
    }

    int VariadicAdaptor::extract(FormatStyle style, const char *fmt, va_list ap, CVargEntry *out) {
        switch (style) {
            case PrintF:
                return extractPrintFArgs(fmt, ap, out);
            case ScanF:
                return extractScanFArgs(fmt, ap, out);
            default:
                break;
        }
        return -1;
    }

    void VariadicAdaptor::callFormatBox64(void *func, const char *fmt, void **args, void *ret) {
        assert(func);
        assert(fmt);
        assert(std::strlen(fmt) >= 2);
        assert(fmt[1] == '_');

        const auto len = std::strlen(fmt);
        CVargEntry vret{};
        callFormatBox64_ret_entry(fmt[0], &vret);

        llvm::SmallVector<CVargEntry, 16> vargs;
        vargs.resize(len - 2);
        for (size_t i = 0; i < len - 2; ++i) {
            const auto fmt_char = fmt[i + 2];
            callFormatBox64_arg_entry(fmt_char, &vargs[i], args[i]);
        }

        call(func, static_cast<int>(vargs.size()), vargs.data(), 0, nullptr, &vret);

        if (!ret) {
            return;
        }

        switch (vret.type) {
            case CVargType_Char:
                *reinterpret_cast<char *>(ret) = vret.c;
                break;
            case CVargType_UChar:
                *reinterpret_cast<unsigned char *>(ret) = vret.uc;
                break;
            case CVargType_Short:
                *reinterpret_cast<short *>(ret) = vret.s;
                break;
            case CVargType_UShort:
                *reinterpret_cast<unsigned short *>(ret) = vret.us;
                break;
            case CVargType_Int:
                *reinterpret_cast<int *>(ret) = vret.i;
                break;
            case CVargType_UInt:
                *reinterpret_cast<unsigned int *>(ret) = vret.u;
                break;
            case CVargType_Long:
                *reinterpret_cast<long *>(ret) = vret.l;
                break;
            case CVargType_ULong:
                *reinterpret_cast<unsigned long *>(ret) = vret.ul;
                break;
            case CVargType_LongLong:
                *reinterpret_cast<long long *>(ret) = vret.ll;
                break;
            case CVargType_ULongLong:
                *reinterpret_cast<unsigned long long *>(ret) = vret.ull;
                break;
            case CVargType_Float:
                *reinterpret_cast<float *>(ret) = vret.f;
                break;
            case CVargType_Double:
                *reinterpret_cast<double *>(ret) = vret.d;
                break;
            case CVargType_Pointer:
                *reinterpret_cast<void **>(ret) = vret.p;
                break;
            default:
                break;
        }
    }

}

#if 1
#  include "VariadicAdaptor_FFCall.cpp.inc"
#else
#  include "VariadicAdaptor_FFI.cpp.inc"
#endif
