#include "VariadicAdaptor.h"

#include <cstdarg>

#include <avcall.h>

namespace lore {

    static inline void avcall_start_helper(av_alist *list, void *func, CVargEntry *entry) {
        switch (entry->type) {
            case 0: {
                av_start_void(*list, func);
                break;
            }
            case CVargType_Char: {
                av_start_char(*list, func, &entry->c);
                break;
            }
            case CVargType_UChar: {
                av_start_uchar(*list, func, &entry->uc);
                break;
            }
            case CVargType_Short: {
                av_start_short(*list, func, &entry->s);
                break;
            }
            case CVargType_UShort: {
                av_start_ushort(*list, func, &entry->us);
                break;
            }
            case CVargType_Int: {
                av_start_int(*list, func, &entry->i);
                break;
            }
            case CVargType_UInt: {
                av_start_uint(*list, func, &entry->u);
                break;
            }
            case CVargType_Long: {
                av_start_long(*list, func, &entry->l);
                break;
            }
            case CVargType_ULong: {
                av_start_ulong(*list, func, &entry->ul);
                break;
            }
            case CVargType_LongLong: {
                av_start_longlong(*list, func, &entry->ll);
                break;
            }
            case CVargType_ULongLong: {
                av_start_longlong(*list, func, &entry->ull);
                break;
            }
            case CVargType_Float: {
                av_start_float(*list, func, &entry->f);
                break;
            }
            case CVargType_Double: {
                av_start_double(*list, func, &entry->d);
                break;
            }
            case CVargType_Pointer: {
                av_start_ptr(*list, func, void *, &entry->p);
                break;
            }
            default:
                break;
        }
    }

    static inline void avcall_arg_helper(av_alist *list, CVargEntry *entry) {
        switch (entry->type) {
            case CVargType_Char: {
                av_char(*list, entry->c);
                break;
            }
            case CVargType_UChar: {
                av_uchar(*list, entry->uc);
                break;
            }
            case CVargType_Short: {
                av_short(*list, entry->s);
                break;
            }
            case CVargType_UShort: {
                av_ushort(*list, entry->us);
                break;
            }
            case CVargType_Int: {
                av_int(*list, entry->i);
                break;
            }
            case CVargType_UInt: {
                av_int(*list, entry->u);
                break;
            }
            case CVargType_Long: {
                av_long(*list, entry->l);
                break;
            }
            case CVargType_ULong: {
                av_ulong(*list, entry->ul);
                break;
            }
            case CVargType_LongLong: {
                av_longlong(*list, entry->ll);
                break;
            }
            case CVargType_ULongLong: {
                av_ulonglong(*list, entry->ull);
                break;
            }
            case CVargType_Float: {
                av_float(*list, entry->f);
                break;
            }
            case CVargType_Double: {
                av_double(*list, entry->d);
                break;
            }
            case CVargType_Pointer: {
                av_ptr(*list, void *, entry->p);
                break;
            }
            default:
                break;
        }
    }

    static void vcall_forward(void *func, int argc1, CVargEntry *argv1, CVargEntry *ret, ...) {

        va_list ap;
        va_start(ap, ret);
        {
            av_alist alist;
            if (argc1 < 0) {
                argc1 = CVargEntryLength(argv1);
            }
            avcall_start_helper(&alist, func, ret);
            for (int i = 0; i < argc1; ++i) {
                avcall_arg_helper(&alist, argv1 + i);
            }
#ifdef __x86_64__
            av_ptr(alist, void *, ap);
#else
            av_struct(alist, va_list, ap);
#endif
            av_call(alist);
        }
        va_end(ap);
    }


    void VariadicAdaptor::call(void *func, int argc1, CVargEntry *argv1, int argc2,
                               CVargEntry *argv2, CVargEntry *ret) {
        av_alist alist;
        if (argc1 < 0) {
            argc1 = CVargEntryLength(argv1);
        }
        if (argc2 < 0) {
            argc2 = CVargEntryLength(argv2);
        }

        avcall_start_helper(&alist, func, ret);

        for (int i = 0; i < argc1; ++i) {
            avcall_arg_helper(&alist, argv1 + i);
        }
        for (int i = 0; i < argc2; ++i) {
            avcall_arg_helper(&alist, argv2 + i);
        }

        av_call(alist);
    }

    void VariadicAdaptor::vcall(void *func, int argc1, CVargEntry *argv1, int argc2,
                                CVargEntry *argv2, CVargEntry *ret) {
        av_alist alist;
        if (argc2 < 0) {
            argc2 = CVargEntryLength(argv2);
        }

        av_start_void(alist, vcall_forward);
        av_ptr(alist, void *, func);
        av_int(alist, argc1);
        av_ptr(alist, void *, argv1);
        av_ptr(alist, void *, ret);

        for (int i = 0; i < argc2; ++i) {
            avcall_arg_helper(&alist, argv2 + i);
        }

        av_call(alist);
    }

}
