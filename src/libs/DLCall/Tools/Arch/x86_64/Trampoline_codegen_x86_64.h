// SPDX-License-Identifier: MIT

#pragma once

#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// Emit the per-instance stub that calls the shared handler `target` and returns to the original
// caller. The handler recovers which trampoline it is from its own return address, which lands on
// the `ret` below (LORE_TRAMP_RESUME_OFFSET == 13 bytes into the stub).
//
//     movabs $target, %r11    ; 49 BB <imm64>   (10 bytes)  r11 is scratch, not an argument reg
//     call   *%r11            ; 41 FF D3        (3 bytes)   return address = the ret below
//     ret                     ; C3              (1 byte)    at offset 13
//
// r11 is chosen so the callback's own argument registers (and %al for a variadic callee) are left
// untouched on the way into the handler.
static void tramp_gen_thunk(void *buf, void *target) {
    unsigned char *p = (unsigned char *) buf;
    p[0] = 0x49; // REX.W
    p[1] = 0xBB; // MOV imm64, %r11
    memcpy(p + 2, &target, 8);
    p[10] = 0x41; // REX.B
    p[11] = 0xFF; // CALL r/m64
    p[12] = 0xD3; // ModR/M: call *%r11
    p[13] = 0xC3; // RET
}

#ifdef __cplusplus
}
#endif
