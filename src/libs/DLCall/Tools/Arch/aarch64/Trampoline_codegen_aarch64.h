// SPDX-License-Identifier: MIT

#pragma once

#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// The shared shim (Trampoline_shim_aarch64.S): parks the identity in thread_last_callback and tail-
// branches to the handler. Its address is baked into every stub.
extern void lore_tramp_shim(void);

// Emit the per-instance stub: load this instance's saved_function into x16 (the identity), the
// handler into x17, and tail-branch to the shim. x9/x16/x17 are scratch (IP registers), not argument
// registers, and nothing is pushed, so this is a tail-branch that leaves every argument, including
// stack-passed ones, exactly where the caller put them.
//
//     ldr x16, #-8    ; 58ffffd0   load saved_function from this struct field (8 bytes back)
//     ldr x17, #12    ; 58000071   load the handler   (literal at offset 16)
//     ldr x9,  #16    ; 58000089   load the shim      (literal at offset 24)
//     br  x9          ; d61f0120   tail-branch to the shim
//     <handler>       ; .quad      offset 16, 8-byte aligned for the ldr literal
//     <shim>          ; .quad      offset 24, 8-byte aligned for the ldr literal
//
// saved_function sits at offset 0 of the FunctionTrampoline and thunk_instr at offset 8, so from the
// first ldr (at thunk_instr+0) the field is a constant 8 bytes back. Loading it (rather than baking
// it) keeps setting saved_function a plain field write.
static void tramp_gen_thunk(void *buf, void *target) {
    uint32_t *w = (uint32_t *) buf;
    w[0] = 0x58FFFFD0; // ldr x16, #-8
    w[1] = 0x58000071; // ldr x17, #12
    w[2] = 0x58000089; // ldr x9, #16
    w[3] = 0xD61F0120; // br x9
    void *shim = (void *) lore_tramp_shim;
    memcpy((char *) buf + 16, &target, 8); // handler literal
    memcpy((char *) buf + 24, &shim, 8);   // shim literal
}

#ifdef __cplusplus
}
#endif
