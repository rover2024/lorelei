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
// handler returns to the `add` below (LORE_TRAMP_RESUME_OFFSET == 9 bytes into the stub), which
// unwinds the realignment and returns to the caller.
//
//     sub    $8, %rsp         ; 48 83 EC 08     (4 bytes)   re-align the stack (see below)
//     call   <shared>         ; E8 <rel32>      (5 bytes)   return address = the add below (offset 9)
//     add    $8, %rsp         ; 48 83 C4 08     (4 bytes)   at offset 9
//     ret                     ; C3              (1 byte)    at offset 13
//
// The `sub $8` is not scratch space: the caller enters at rsp % 16 == 8 (the ABI state right after a
// call), and our own `call` pushes another 8, so without it the handler would run at rsp % 16 == 0.
// A handler compiled for the ABI then spills with aligned moves (gcc -O2 movaps) relative to a stack
// it believes is aligned, and faults. Subtracting 8 first makes the handler see the ABI-required
// rsp % 16 == 8; the matching `add $8` unwinds it before returning to the caller.
static void tramp_gen_thunk(void *buf, void *shared) {
    unsigned char *p = (unsigned char *) buf;
    p[0] = 0x48; // sub $8, %rsp
    p[1] = 0x83;
    p[2] = 0xEC;
    p[3] = 0x08;
    int32_t rel = (int32_t) ((char *) shared - ((char *) buf + 9));
    p[4] = 0xE8; // CALL rel32
    memcpy(p + 5, &rel, 4);
    p[9] = 0x48; // add $8, %rsp
    p[10] = 0x83;
    p[11] = 0xC4;
    p[12] = 0x08;
    p[13] = 0xC3; // RET
}

#ifdef __cplusplus
}
#endif
