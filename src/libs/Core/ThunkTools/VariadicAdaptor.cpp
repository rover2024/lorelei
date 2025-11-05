#include "VariadicAdaptor.h"

#include <mpaland_printf/printf.h>
#include <mpaland_printf/printf_scanf.h>

#include "Config.h"

#if LORE_CONFIG_VARIADIC_ADAPTOR_USE_FFCALL
#  include <avcall.h>
#else
#  include <ffi.h>

#  include <stdlib.h>
#endif

namespace lore {

    static int extractPrintFArgs(const char *format, va_list ap, struct CVargEntry *out) {
        char buffer[1];
        (void) _vsnprintf(_out_null, buffer, (size_t) -1, format, ap, out);

        struct CVargEntry *p = out;
        while (p->type) {
            p++;
        }
        return p - out;
    }

    static int extractScanFArgs(const char *format, va_list ap, struct CVargEntry *out) {
        char buffer[1];
        (void) _vsnprintf_scanf(_out_null, buffer, (size_t) -1, format, ap, out);

        struct CVargEntry *p = out;
        while (p->type) {
            p++;
        }
        return p - out;
    }

    int VariadicAdaptor::extract(FormatStyle style, const char *fmt, va_list ap, CVargEntry *out) {
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

    static inline int varg_entry_length(struct CVargEntry *argv) {
        int argc = 0;
        for (struct CVargEntry *p = argv; p && p->type; ++p) {
            argc++;
        }
        return argc;
    }

#if LORE_CONFIG_VARIADIC_ADAPTOR_USE_FFCALL
    static inline void avcall_start_helper(av_alist *list, void *func, struct CVargEntry *entry) {
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

    static inline void avcall_arg_helper(av_alist *list, struct CVargEntry *entry) {
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

    void VariadicAdaptor::call(void *func, int argc1, CVargEntry *argv1, int argc2,
                               CVargEntry *argv2, CVargEntry *ret) {
        av_alist alist;
        if (argc1 < 0) {
            argc1 = varg_entry_length(argv1);
        }
        if (argc2 < 0) {
            argc2 = varg_entry_length(argv2);
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

    static void vcall_forward(void *func, int argc1, struct CVargEntry *argv1,
                              struct CVargEntry *ret, ...) {

        va_list ap;
        va_start(ap, ret);
        {
            av_alist alist;
            if (argc1 < 0) {
                argc1 = varg_entry_length(argv1);
            }
            avcall_start_helper(&alist, func, ret);
            for (int i = 0; i < argc1; ++i) {
                avcall_arg_helper(&alist, argv1 + i);
            }
#  ifdef __x86_64__
            av_ptr(alist, void *, ap);
#  else
            av_struct(alist, va_list, ap);
#  endif
            av_call(alist);
        }
        va_end(ap);
    }

    void VariadicAdaptor::vcall(void *func, int argc1, CVargEntry *argv1, int argc2,
                                CVargEntry *argv2, CVargEntry *ret) {
        av_alist alist;
        if (argc2 < 0) {
            argc2 = varg_entry_length(argv2);
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
#else

    // Helper function to map LORE_VT_* types to ffi_type
    static ffi_type *get_ffi_type(int lore_type) {
        switch (lore_type) {
            case 0: // void
                return &ffi_type_void;
            case CVargType_Char:
                return &ffi_type_sint8;
            case CVargType_UChar:
                return &ffi_type_uint8;
            case CVargType_Short:
                return &ffi_type_sint16;
            case CVargType_UShort:
                return &ffi_type_uint16;
            case CVargType_Int:
                return &ffi_type_sint32;
            case CVargType_UInt:
                return &ffi_type_uint32;
            case CVargType_Long:
                return sizeof(long) == 4 ? &ffi_type_sint32 : &ffi_type_sint64;
            case CVargType_ULong:
                return sizeof(unsigned long) == 4 ? &ffi_type_uint32 : &ffi_type_uint64;
            case CVargType_LongLong:
                return &ffi_type_sint64;
            case CVargType_ULongLong:
                return &ffi_type_uint64;
            case CVargType_Float:
                return &ffi_type_float;
            case CVargType_Double:
                return &ffi_type_double;
            case CVargType_Pointer:
                return &ffi_type_pointer;
            default:
                return &ffi_type_void;
        }
    }

    // Helper function to get value pointer from CVargEntry
    static void *get_value_ptr(struct CVargEntry *entry) {
        switch (entry->type) {
            case CVargType_Char:
                return &entry->c;
            case CVargType_UChar:
                return &entry->uc;
            case CVargType_Short:
                return &entry->s;
            case CVargType_UShort:
                return &entry->us;
            case CVargType_Int:
                return &entry->i;
            case CVargType_UInt:
                return &entry->u;
            case CVargType_Long:
                return &entry->l;
            case CVargType_ULong:
                return &entry->ul;
            case CVargType_LongLong:
                return &entry->ll;
            case CVargType_ULongLong:
                return &entry->ull;
            case CVargType_Float:
                return &entry->f;
            case CVargType_Double:
                return &entry->d;
            case CVargType_Pointer:
                return &entry->p;
            default:
                return NULL;
        }
    }

    void VariadicAdaptor::call(void *func, int argc1, CVargEntry *argv1, int argc2,
                               CVargEntry *argv2, CVargEntry *ret) {
        if (argc1 < 0)
            argc1 = varg_entry_length(argv1);
        if (argc2 < 0)
            argc2 = varg_entry_length(argv2);

        int total_args = argc1 + argc2;

        auto arg_types = (ffi_type **) alloca(total_args * sizeof(ffi_type *));
        auto arg_values = (void **) alloca(total_args * sizeof(void *));

        // Prepare arguments
        int idx = 0;
        for (int i = 0; i < argc1; i++) {
            arg_types[idx] = get_ffi_type(argv1[i].type);
            arg_values[idx] = get_value_ptr(&argv1[i]);
            idx++;
        }
        for (int i = 0; i < argc2; i++) {
            arg_types[idx] = get_ffi_type(argv2[i].type);
            arg_values[idx] = get_value_ptr(&argv2[i]);
            idx++;
        }

        // Prepare return type
        auto ret_type = get_ffi_type(ret->type);
        auto ret_ptr = get_value_ptr(ret);

        // Prepare cif
        ffi_cif cif;
        if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, total_args, ret_type, arg_types) != FFI_OK) {
            return;
        }

        // Call the function
        ffi_call(&cif, FFI_FN(func), ret_ptr, arg_values);
    }

    static void vcall_forward(void *func, int argc1, struct CVargEntry *argv1,
                              struct CVargEntry *ret, ...) {
        va_list ap;
        va_start(ap, ret);

        if (argc1 < 0)
            argc1 = varg_entry_length(argv1);

        int total_args = argc1 + 1; // +1 for va_list
        auto arg_types = (ffi_type **) alloca(total_args * sizeof(ffi_type *));
        auto arg_values = (void **) alloca(total_args * sizeof(void *));

        // Prepare normal arguments
        for (int i = 0; i < argc1; i++) {
            arg_types[i] = get_ffi_type(argv1[i].type);
            arg_values[i] = get_value_ptr(&argv1[i]);
        }

        // Prepare va_list argument
        arg_types[argc1] = &ffi_type_pointer; // va_list is passed as pointer
        arg_values[argc1] = &ap;

        // Prepare return type
        auto ret_type = get_ffi_type(ret->type);
        auto ret_ptr = get_value_ptr(ret);

        // Prepare cif
        ffi_cif cif;
        if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, total_args, ret_type, arg_types) != FFI_OK) {
            va_end(ap);
            return;
        }

        // Call the function
        ffi_call(&cif, FFI_FN(func), ret_ptr, arg_values);
        va_end(ap);
    }

    void VariadicAdaptor::vcall(void *func, int argc1, CVargEntry *argv1, int argc2,
                                CVargEntry *argv2, CVargEntry *ret) {
        if (argc2 < 0)
            argc2 = varg_entry_length(argv2);

        // Prepare arguments for vcall_forward
        int total_args = 4 + argc2; // func, argc1, argv1, ret + variable args
        auto arg_types = (ffi_type **) alloca(total_args * sizeof(ffi_type *));
        auto arg_values = (void **) alloca(total_args * sizeof(void *));

        // Fixed arguments
        arg_types[0] = &ffi_type_pointer; // func
        arg_values[0] = &func;

        arg_types[1] = &ffi_type_sint32; // argc1
        arg_values[1] = &argc1;

        arg_types[2] = &ffi_type_pointer; // argv1
        arg_values[2] = &argv1;

        arg_types[3] = &ffi_type_pointer; // ret
        arg_values[3] = &ret;

        // Variable arguments
        for (int i = 0; i < argc2; i++) {
            arg_types[4 + i] = get_ffi_type(argv2[i].type);
            arg_values[4 + i] = get_value_ptr(&argv2[i]);
        }

        // Prepare cif
        ffi_cif cif;
        if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, total_args, &ffi_type_void, arg_types) != FFI_OK) {
            return;
        }

        // Call vcall_forward
        ffi_call(&cif, FFI_FN(vcall_forward), NULL, arg_values);
    }

#endif

}