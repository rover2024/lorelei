// SPDX-License-Identifier: MIT

#pragma once

#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// Offset of the epilogue within the shared entry (right after the prologue and its 8-byte literal).
#define LORE_TRAMP_RISCV64_EPILOGUE_OFF 32

// Encode a J-type jump (jal): scatter imm into imm[20|10:1|11|19:12], with the given rd, opcode 0x6F.
static uint32_t tramp_gen_riscv_jal_helper(int32_t imm, uint32_t rd) {
    uint32_t imm20 = ((imm >> 20) & 0x1) << 31;
    uint32_t imm10_1 = ((imm >> 1) & 0x3FF) << 21;
    uint32_t imm11 = ((imm >> 11) & 0x1) << 20;
    uint32_t imm19_12 = ((imm >> 12) & 0xFF) << 12;
    return imm20 | imm19_12 | imm11 | imm10_1 | ((rd & 0x1F) << 7) | 0x6F;
}

// Emit the table's shared landing: a prologue plus an epilogue.
//
// prologue (reached by `jal t0` from a stub, so ra still holds the caller's return address and t0
// holds the per-instance resume address):
//     addi sp, sp, -16        ; ff010113   open a small frame
//     sd   ra, 0(sp)          ; 00113023   save the caller's return address
//     mv   ra, t0             ; 00028093   ra = the stub's resume point (the callback identity)
//     auipc t1, 0             ; 00000317   t1 = pc of this instruction (offset 12)
//     ld   t1, 12(t1)         ; 00c33303   t1 = target (literal at offset 24)
//     jr   t1                 ; 00030067   tail branch to the handler, ra = resume point
//     <target>               ; .quad       offset 24, 8-byte aligned for the ld literal
// epilogue (the stub jumps here after the handler returns):
//     ld   ra, 0(sp)          ; 00013083   offset 32: restore the caller's return address
//     addi sp, sp, 16         ; 01010113   offset 36: close the frame
//     ret                     ; 00008067   offset 40: return to the caller
//
// The handler is entered with ra = the stub's resume point, so __builtin_return_address(0) inside it
// identifies the instance. `jr` (jalr x0) leaves ra untouched on the way in.
static void tramp_gen_shared(void *buf, void *target) {
    uint32_t *w = (uint32_t *) buf;
    w[0] = 0xFF010113; // addi sp, sp, -16
    w[1] = 0x00113023; // sd ra, 0(sp)
    w[2] = 0x00028093; // mv ra, t0
    w[3] = 0x00000317; // auipc t1, 0
    w[4] = 0x00C33303; // ld t1, 12(t1)
    w[5] = 0x00030067; // jr t1
    memcpy((char *) buf + 24, &target, 8);
    w[8] = 0x00013083;  // ld ra, 0(sp)     (epilogue, offset 32)
    w[9] = 0x01010113;  // addi sp, sp, 16  (offset 36)
    w[10] = 0x00008067; // ret              (offset 40)
}

// Emit the per-instance stub: link into the shared prologue via t0 (which leaves the caller's ra
// intact and records this stub's resume address), then (after the handler returns to that resume
// address) jump to the shared epilogue.
//
//     jal t0, <prologue>      ; ........   t0 = the j below (offset 4), ra untouched
//     j   <epilogue>          ; ........   offset 4: the handler returns here
static void tramp_gen_thunk(void *buf, void *shared) {
    uint32_t *w = (uint32_t *) buf;
    char *prologue = (char *) shared;
    char *epilogue = (char *) shared + LORE_TRAMP_RISCV64_EPILOGUE_OFF;
    w[0] = tramp_gen_riscv_jal_helper((int32_t) (prologue - ((char *) buf + 0)), 5); // jal t0, <prologue>
    w[1] = tramp_gen_riscv_jal_helper((int32_t) (epilogue - ((char *) buf + 4)), 0); // j <epilogue>
}

#ifdef __cplusplus
}
#endif
