// SPDX-License-Identifier: MIT

#pragma once

#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// Emit the table's shared landing: load the handler `target` and tail-jump to it. Reached by a
// `call` from a per-instance stub, so the return address on the stack still points into that stub;
// the `jmp` leaves it there, so the handler recovers the instance from its own return address.
//
//     movabs $target, %r11    ; 49 BB <imm64>   (10 bytes)  r11 is scratch, not an argument reg
//     jmp    *%r11            ; 41 FF E3        (3 bytes)   tail call, does not touch the stack
//
// r11 is chosen so the callback's own argument registers (and %al for a variadic callee) are left
// untouched on the way into the handler.
static void tramp_gen_shared(void *buf, void *target) {
    unsigned char *p = (unsigned char *) buf;
    p[0] = 0x49; // REX.W
    p[1] = 0xBB; // MOV imm64, %r11
    memcpy(p + 2, &target, 8);
    p[10] = 0x41; // REX.B
    p[11] = 0xFF; // JMP r/m64
    p[12] = 0xE3; // ModR/M: jmp *%r11
}

// Emit the per-instance stub: call the shared landing, then return to the original caller. The
// handler returns to the `ret` below (LORE_TRAMP_RESUME_OFFSET == 5 bytes into the stub), which then
// returns to the caller.
//
//     call   <shared>         ; E8 <rel32>      (5 bytes)   return address = the ret below (offset 5)
//     ret                     ; C3              (1 byte)    at offset 5
static void tramp_gen_thunk(void *buf, void *shared) {
    unsigned char *p = (unsigned char *) buf;
    int32_t rel = (int32_t) ((char *) shared - ((char *) buf + 5));
    p[0] = 0xE8; // CALL rel32
    memcpy(p + 1, &rel, 4);
    p[5] = 0xC3; // RET
}

#ifdef __cplusplus
}
#endif
