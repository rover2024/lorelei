#include "CallbackTrampoline.h"

#ifdef __linux__
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
        auto trampoline =
            (CallbackTrampolineTable *) mmap(NULL, table_size, PROT_READ | PROT_WRITE | PROT_EXEC,
                                             MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        tramp_gen_jump(trampoline->jump_instr, target);
        trampoline->count = count;
        for (int i = 0; i < count; i++) {
            auto thunk = &trampoline->trampoline[i];
            thunk->saved_callback = NULL;
            tramp_gen_thunk(thunk->thunk_instr,
                          (intptr_t) thunk->thunk_instr - (intptr_t) trampoline);
        }
        return trampoline;
    }

    void CallbackTrampolineTable::destroy(CallbackTrampolineTable *table) {
        size_t table_size =
            sizeof(CallbackTrampolineTable) + table->count * sizeof(CallbackTrampoline);
        munmap(table, table_size);
    }
#endif

}
