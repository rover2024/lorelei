#include "FunctionTrampoline.h"

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
    FunctionTrampolineTable *FunctionTrampolineTable::create(size_t count, void *target,
                                                             uintptr_t magic_sign) {
        return {};
    }
    void FunctionTrampolineTable::destroy(FunctionTrampolineTable *table) {
    }
#else
    FunctionTrampolineTable *FunctionTrampolineTable::create(size_t count, void *target,
                                                             uintptr_t magic_sign) {
        size_t table_size = sizeof(FunctionTrampolineTable) + count * sizeof(FunctionTrampoline);
        auto trampoline =
            (FunctionTrampolineTable *) mmap(NULL, table_size, PROT_READ | PROT_WRITE | PROT_EXEC,
                                             MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        tramp_gen_jump(trampoline->jump_instr, target);
        trampoline->count = count;
        for (int i = 0; i < count; i++) {
            auto thunk = &trampoline->trampoline[i];
            thunk->magic_sign = magic_sign;
            thunk->saved_function = NULL;
            tramp_gen_thunk(thunk->thunk_instr,
                            (intptr_t) thunk->thunk_instr - (intptr_t) trampoline);
        }
        return trampoline;
    }

    void FunctionTrampolineTable::destroy(FunctionTrampolineTable *table) {
        size_t table_size =
            sizeof(FunctionTrampolineTable) + table->count * sizeof(FunctionTrampoline);
        munmap(table, table_size);
    }
#endif

}
