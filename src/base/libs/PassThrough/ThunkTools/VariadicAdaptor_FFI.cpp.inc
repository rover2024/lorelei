#include "VariadicAdaptor.h"

#include <cstdlib>

#include <ffi.h>

#if _WIN32
#  define ALLOCA _alloca
#else
#  define ALLOCA alloca
#endif

namespace lore {

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
            argc1 = CVargEntryLength(argv1);
        if (argc2 < 0)
            argc2 = CVargEntryLength(argv2);

        int total_args = argc1 + argc2;

        auto arg_types = (ffi_type **) ALLOCA(total_args * sizeof(ffi_type *));
        auto arg_values = (void **) ALLOCA(total_args * sizeof(void *));

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
            argc1 = CVargEntryLength(argv1);

        int total_args = argc1 + 1; // +1 for va_list
        auto arg_types = (ffi_type **) ALLOCA(total_args * sizeof(ffi_type *));
        auto arg_values = (void **) ALLOCA(total_args * sizeof(void *));

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
            argc2 = CVargEntryLength(argv2);

        // Prepare arguments for vcall_forward
        int total_args = 4 + argc2; // func, argc1, argv1, ret + variable args
        auto arg_types = (ffi_type **) ALLOCA(total_args * sizeof(ffi_type *));
        auto arg_values = (void **) ALLOCA(total_args * sizeof(void *));

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

}
