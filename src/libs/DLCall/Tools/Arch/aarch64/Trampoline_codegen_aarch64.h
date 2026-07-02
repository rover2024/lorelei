// SPDX-License-Identifier: MIT

#pragma once

#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// Offset of the epilogue within the shared entry (right after the prologue and its 8-byte literal).
#define LORE_TRAMP_AARCH64_EPILOGUE_OFF 24

// Emit the table's shared landing: a prologue plus an epilogue.
//
// prologue (reached by `b` from a stub, so x30 still holds the caller's return address and x9 holds
// the per-instance resume address):
//     str x30, [sp, #-16]!    ; f81f0ffe   save the caller's return address
//     mov x30, x9             ; aa0903fe   x30 = the stub's resume point (the callback identity)
//     ldr x17, #8             ; 58000051   x17 = target (literal at offset 16)   x17 is IP1, scratch
//     br  x17                 ; d61f0220   tail branch to the handler, x30 = resume point
//     <target>                ; .quad      offset 16, 8-byte aligned for the ldr literal
// epilogue (the stub branches here after the handler returns):
//     ldr x30, [sp], #16      ; f84107fe   offset 24: restore the caller's return address
//     ret                     ; d65f03c0   offset 28: return to the caller
//
// The handler is entered with x30 = the stub's resume point, so __builtin_return_address(0) inside it
// identifies the instance. `br` (not `blr`) leaves x30 untouched on the way in.
static void tramp_gen_shared(void *buf, void *target) {
    uint32_t *w = (uint32_t *) buf;
    w[0] = 0xF81F0FFE; // str x30, [sp, #-16]!
    w[1] = 0xAA0903FE; // mov x30, x9
    w[2] = 0x58000051; // ldr x17, #8
    w[3] = 0xD61F0220; // br x17
    memcpy((char *) buf + 16, &target, 8);
    w[6] = 0xF84107FE; // ldr x30, [sp], #16   (epilogue, offset 24)
    w[7] = 0xD65F03C0; // ret                  (offset 28)
}

// Emit the per-instance stub: hand the shared prologue this stub's resume address in x9, branch to
// it, and (after the handler returns to that resume address) branch to the shared epilogue.
//
//     adr x9, #8              ; 10000049   x9 = the b epilogue below (offset 8)
//     b   <prologue>          ; 14......   x30 untouched (caller's return address)
//     b   <epilogue>          ; 14......   offset 8: the handler returns here
static void tramp_gen_thunk(void *buf, void *shared) {
    uint32_t *w = (uint32_t *) buf;
    char *prologue = (char *) shared;
    char *epilogue = (char *) shared + LORE_TRAMP_AARCH64_EPILOGUE_OFF;
    int32_t off_p = (int32_t) ((prologue - ((char *) buf + 4)) >> 2); // b is at offset 4
    int32_t off_e = (int32_t) ((epilogue - ((char *) buf + 8)) >> 2); // b is at offset 8
    w[0] = 0x10000049;                        // adr x9, #8
    w[1] = 0x14000000 | (off_p & 0x03FFFFFF); // b <prologue>
    w[2] = 0x14000000 | (off_e & 0x03FFFFFF); // b <epilogue>
}

#ifdef __cplusplus
}
#endif
