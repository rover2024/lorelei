#ifndef LORE_BASE_RUNTIMEBASE_FUNCTIONTRAMPOLINE_H
#define LORE_BASE_RUNTIMEBASE_FUNCTIONTRAMPOLINE_H

#include <cstdint>
#include <cstddef>
#include <cassert>

#include <LoreBase/RuntimeBase/Global.h>

namespace lore {

    struct FunctionTrampoline {
        void *saved_function;
        char thunk_instr[16]; // lea -7(%rip), %r11; jmp jump_instr
        uintptr_t magic_sign;
    };

    struct LORERTBASE_EXPORT FunctionTrampolineTable {
        char jump_instr[32]; // mov -8(%r11), %r11; mov target, %rax; jmp *%rax
        size_t count;        // length of the "trampoline"
        struct FunctionTrampoline trampoline[];

        /// Allocate a new \c FunctionTrampolineTable with the given count of trampolines.
        static FunctionTrampolineTable *create(size_t count, void *target, uintptr_t magic_sign);

        /// Free the given \c FunctionTrampolineTable.
        static void destroy(FunctionTrampolineTable *table);
    };

    struct FunctionTrampolineTableGuard {
        FunctionTrampolineTable *table;

        inline FunctionTrampolineTableGuard() : table(nullptr) {
        }

        inline ~FunctionTrampolineTableGuard() {
            if (table) {
                FunctionTrampolineTable::destroy(table);
            }
        }

        void create(size_t count, void *target, uintptr_t magic_sign) {
            assert(table == nullptr);
            table = FunctionTrampolineTable::create(count, target, magic_sign);
        }
    };

}

#endif // LORE_BASE_RUNTIMEBASE_FUNCTIONTRAMPOLINE_H
