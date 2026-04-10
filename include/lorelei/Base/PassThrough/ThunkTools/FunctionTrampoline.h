#ifndef LORE_BASE_PASSTHROUGH_FUNCTIONTRAMPOLINE_H
#define LORE_BASE_PASSTHROUGH_FUNCTIONTRAMPOLINE_H

#include <cstdint>
#include <cstddef>
#include <cassert>

#include <lorelei/Base/PassThrough/Global.h>

namespace lore {

    struct FunctionTrampoline {
        void *saved_function;
        char thunk_instr[16]; // lea -7(%rip), %r11; jmp jump_instr
        uintptr_t magic_sign;
    };

    struct LOREPASSTHROUGH_EXPORT FunctionTrampolineTable {
        char jump_instr[32]; // mov -8(%r11), %r11; mov target, %rax; jmp *%rax
        size_t count;        // length of the "trampoline"
        struct FunctionTrampoline trampoline[];

        /// Allocate a new \c FunctionTrampolineTable with the given count of trampolines.
        static FunctionTrampolineTable *create(size_t count, void *target, uintptr_t magic_sign);

        /// Free the given \c FunctionTrampolineTable.
        static void destroy(FunctionTrampolineTable *table);
    };

}

#endif // LORE_BASE_PASSTHROUGH_FUNCTIONTRAMPOLINE_H
