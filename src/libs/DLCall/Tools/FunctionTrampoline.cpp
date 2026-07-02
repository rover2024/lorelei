// SPDX-License-Identifier: MIT

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
        // RWX so the synthesized machine code is both writable here and executable when handed out.
        auto trampoline =
            (FunctionTrampolineTable *) mmap(NULL, table_size, PROT_READ | PROT_WRITE | PROT_EXEC,
                                             MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        trampoline->count = count;
        for (size_t i = 0; i < count; i++) {
            auto thunk = &trampoline->trampoline[i];
            thunk->magic_sign = magic_sign;
            thunk->saved_function = NULL;
            // Each stub calls `target` directly and returns; the handler recovers this instance from
            // its own return address, so no shared landing stub is needed.
            tramp_gen_thunk(thunk->thunk_instr, target);
        }
        // Flush the instruction cache over the just-written code so it is visible to execution on
        // architectures without a coherent I-cache (aarch64, riscv64), a no-op on x86_64.
        __builtin___clear_cache((char *) trampoline, (char *) trampoline + table_size);
        return trampoline;
    }

    void FunctionTrampolineTable::destroy(FunctionTrampolineTable *table) {
        size_t table_size =
            sizeof(FunctionTrampolineTable) + table->count * sizeof(FunctionTrampoline);
        munmap(table, table_size);
    }
#endif

}
