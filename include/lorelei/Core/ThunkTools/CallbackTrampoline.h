#ifndef LORECALL_CALLBACKTRAMPOLINE_H
#define LORECALL_CALLBACKTRAMPOLINE_H

#include <cstdint>
#include <cstddef>

#include <lorelei/Core/Global.h>

namespace lore {

    struct CallbackTrampoline {
        void *saved_callback;
        char thunk_instr[16]; // lea -7(%rip), %r11; jmp jump_instr
    };

    struct CallbackTrampolineTable {
        char jump_instr[32]; // mov -8(%r11), %r11; mov target, %rax; jmp *%rax
        size_t count;        // length of the "trampoline"
        struct CallbackTrampoline trampoline[];

        /// Allocate a new CallbackTrampolineTable with the given count of trampolines.
        static CallbackTrampolineTable *create(size_t count, void *target);

        /// Free the given CallbackTrampolineTable.
        static void destroy(CallbackTrampolineTable *table);
    };

}

#endif // LORECALL_CALLBACKTRAMPOLINE_H
