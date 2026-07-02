// SPDX-License-Identifier: MIT

#pragma once

#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// The shared shim (Trampoline_shim_x86_64.S): parks the identity in thread_last_callback and tail-
// branches to the handler. Its address is baked into every stub.
extern void lore_tramp_shim(void);

// Emit the per-instance stub: load this instance's saved_function into %r11 (the identity), the
// handler into %r10, and tail-jump to the shim. r10/r11/rax are scratch (not argument registers) and
// there is no push, so this is a tail-branch that leaves every argument, including stack-passed ones,
// exactly where the caller put them.
//
//     mov  -15(%rip), %r11    ; 4C 8B 1D F1 FF FF FF   load saved_function from this struct field
//     movabs target, %r10     ; 49 BA <imm64>          the table's handler
//     movabs shim,   %rax     ; 48 B8 <imm64>          the shared shim
//     jmp    *%rax            ; FF E0                   tail-branch to the shim
//
// The saved_function load is PC-relative to this stub. saved_function sits at offset 0 of the
// FunctionTrampoline and thunk_instr at offset 8, so from the `mov` (at thunk_instr+0) the field is a
// constant -15 bytes back (offset 8 minus the 7-byte instruction minus the 8-byte field... i.e.
// 0 - (8 + 7)). Loading it (rather than baking it) keeps setting saved_function a plain field write.
static void tramp_gen_thunk(void *buf, void *target) {
    unsigned char *p = (unsigned char *) buf;
    int32_t disp = -15;
    p[0] = 0x4C; p[1] = 0x8B; p[2] = 0x1D; memcpy(p + 3, &disp, 4); // mov -15(%rip), %r11
    p[7] = 0x49; p[8] = 0xBA; memcpy(p + 9, &target, 8);            // movabs target, %r10
    void *shim = (void *) lore_tramp_shim;
    p[17] = 0x48; p[18] = 0xB8; memcpy(p + 19, &shim, 8);           // movabs shim, %rax
    p[27] = 0xFF; p[28] = 0xE0;                                      // jmp *%rax
}

#ifdef __cplusplus
}
#endif
