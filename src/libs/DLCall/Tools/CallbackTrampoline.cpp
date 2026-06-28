// SPDX-License-Identifier: MIT

#include "CallbackTrampoline.h"

#ifndef _WIN32
#  include <sys/mman.h>
#endif

#ifndef _WIN32
#  ifdef __x86_64__
#    include "Arch/x86_64/Trampoline_codegen_x86_64.h"
#  elif defined(__aarch64__)
#    include "Arch/aarch64/Trampoline_codegen_aarch64.h"
#  elif defined(__riscv) && __riscv_xlen == 64
#    include "Arch/riscv64/Trampoline_codegen_riscv64.h"
#  else
#    error "Unsupported architecture"
#  endif
#endif

namespace lore {

#ifdef _WIN32
    CallbackTrampolineTable *CallbackTrampolineTable::create(size_t count, void *target) {
        return {};
    }
    void CallbackTrampolineTable::destroy(CallbackTrampolineTable *table) {
    }
#else
    CallbackTrampolineTable *CallbackTrampolineTable::create(size_t count, void *target) {
        size_t table_size = sizeof(CallbackTrampolineTable) + count * sizeof(CallbackTrampoline);
        // RWX so the synthesized machine code is both writable here and executable when handed out.
        auto trampoline =
            (CallbackTrampolineTable *) mmap(NULL, table_size, PROT_READ | PROT_WRITE | PROT_EXEC,
                                             MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        // Emit the one shared landing stub at the head of the table (jumps to target).
        tramp_gen_jump(trampoline->jump_instr, target);
        trampoline->count = count;
        for (int i = 0; i < count; i++) {
            auto thunk = &trampoline->trampoline[i];
            thunk->saved_callback = NULL;
            // Pass this instance's distance from the table base; the emitted thunk uses it to form a
            // RIP-relative jump back to the shared jump_instr at the table head.
            tramp_gen_thunk(thunk->thunk_instr,
                          (intptr_t) thunk->thunk_instr - (intptr_t) trampoline);
        }
        // Flush the instruction cache over the just-written code so it is visible to execution on
        // architectures without a coherent I-cache (aarch64, riscv64); a no-op on x86_64.
        __builtin___clear_cache((char *) trampoline, (char *) trampoline + table_size);
        return trampoline;
    }

    void CallbackTrampolineTable::destroy(CallbackTrampolineTable *table) {
        size_t table_size =
            sizeof(CallbackTrampolineTable) + table->count * sizeof(CallbackTrampoline);
        munmap(table, table_size);
    }
#endif

}
