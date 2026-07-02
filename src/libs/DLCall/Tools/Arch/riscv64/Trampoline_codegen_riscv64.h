// SPDX-License-Identifier: MIT

#pragma once

#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// The shared shim (Trampoline_shim_riscv64.S): parks the identity in thread_last_callback and tail-
// branches to the handler. Its address is baked into every stub.
extern void lore_tramp_shim(void);

// Emit the per-instance stub: load this instance's saved_function into t0 (the identity), the handler
// into t1, and tail-branch to the shim. t0/t1/t2/t3 are temporaries, not argument registers, and
// nothing is pushed, so this is a tail-branch that leaves every argument, including stack-passed
// ones, exactly where the caller put them.
//
//     auipc t3, 0     ; 00000e17   t3 = pc, the base for the loads below
//     ld t0, -8(t3)   ; ff8e3283   load saved_function from this struct field (8 bytes back)
//     ld t1, 24(t3)   ; 018e3303   load the handler   (literal at offset 24)
//     ld t2, 32(t3)   ; 020e3383   load the shim      (literal at offset 32)
//     jr t2           ; 00038067   tail-branch to the shim
//     <handler>       ; .quad      offset 24, 8-byte aligned for the ld
//     <shim>          ; .quad      offset 32, 8-byte aligned for the ld
//
// saved_function sits at offset 0 of the FunctionTrampoline and thunk_instr at offset 8, so from the
// auipc (at thunk_instr+0) the field is a constant 8 bytes back. Loading it (rather than baking it)
// keeps setting saved_function a plain field write.
static void tramp_gen_thunk(void *buf, void *target) {
    uint32_t *w = (uint32_t *) buf;
    w[0] = 0x00000E17; // auipc t3, 0
    w[1] = 0xFF8E3283; // ld t0, -8(t3)
    w[2] = 0x018E3303; // ld t1, 24(t3)
    w[3] = 0x020E3383; // ld t2, 32(t3)
    w[4] = 0x00038067; // jr t2
    void *shim = (void *) lore_tramp_shim;
    memcpy((char *) buf + 24, &target, 8); // handler literal
    memcpy((char *) buf + 32, &shim, 8);   // shim literal
}

#ifdef __cplusplus
}
#endif
