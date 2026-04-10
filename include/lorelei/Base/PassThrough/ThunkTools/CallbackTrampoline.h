#ifndef LORE_BASE_PASSTHROUGH_CALLBACKTRAMPOLINE_H
#define LORE_BASE_PASSTHROUGH_CALLBACKTRAMPOLINE_H

#include <cstdint>
#include <cstddef>
#include <cassert>

#include <lorelei/Base/PassThrough/Global.h>

namespace lore {

    struct CallbackTrampoline {
        void *saved_callback;
        char thunk_instr[16]; // lea -7(%rip), %r11; jmp jump_instr
    };

    struct LOREPASSTHROUGH_EXPORT CallbackTrampolineTable {
        char jump_instr[32]; // mov -8(%r11), %r11; mov target, %rax; jmp *%rax
        size_t count;        // length of the "trampoline"
        struct CallbackTrampoline trampoline[];

        /// Allocate a new \c CallbackTrampolineTable with the given count of trampolines.
        static CallbackTrampolineTable *create(size_t count, void *target);

        /// Free the given \c CallbackTrampolineTable.
        static void destroy(CallbackTrampolineTable *table);
    };

}

#endif // LORE_BASE_PASSTHROUGH_CALLBACKTRAMPOLINE_H
