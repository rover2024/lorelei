#include "loreshared.h"

#include <sys/mman.h>

#ifdef __x86_64__
// Generate:
//     Save current Entry to R10 and jump to address Entry-N
static void generate_thunk(void *buf, int64_t N) {
    char *instr = buf;

    // _start:
    // leaq -7(%rip), %r10
    instr[0] = 0x4c; // REX.W prefix
    instr[1] = 0x8d; // LEA opcode
    instr[2] = 0x15; // ModR/M: r10 with RIP-relative addressing
    instr[3] = 0xF9; // RIP-relative displacement (little-endian)
    instr[4] = 0xFF; // -7 in 32-bit: 0xFFFFFFF9
    instr[5] = 0xFF;
    instr[6] = 0xFF;

    // jmp _start-N
    instr[7] = 0xe9; // JMP rel32 opcode

    int32_t offset = -12 - (int32_t) N;
    instr[8] = (offset) & 0xFF;
    instr[9] = (offset >> 8) & 0xFF;
    instr[10] = (offset >> 16) & 0xFF;
    instr[11] = (offset >> 24) & 0xFF;
}

// Generate:
//     Jump to address Entry
static void generate_jump(void *buf, void *target) {
    char *instr = buf;

    // movq $target, %rax
    *instr++ = 0x48;
    *instr++ = 0xB8;
    *(uint64_t *) instr = (uint64_t) target;
    instr += 8;

    // jmp *%rax
    *instr++ = 0xFF;
    *instr++ = 0xE0;
}
#elif defined(__aarch64__)
// Generate:
//     Save current Entry to X16 and jump to address Entry-N
static void generate_thunk(void *buf, size_t N) {
    uint32_t *instr = (uint32_t *) buf;

    // _start:
    // ADR X16, #0
    instr[0] = 0x10000010;

    // target address = buf - N
    // branch instruction address = buf + 4
    // actual offset = (buf - N) - (buf + 4) = -4 - N
    // instruction offset = (-4 - N) / 4
    int32_t jump_offset = (-4 - (int32_t) N) / 4;

    // B <offset>
    instr[1] = 0x14000000 | (jump_offset & 0x03FFFFFF);
}

// Generate:
//     Jump to address Entry
static void generate_jump(void *buf, void *target) {
    uint64_t *code = buf;
    uint64_t target_addr = (uint64_t) target;

    // MOVZ X17, #imm16, LSL #0
    uint64_t low16 = target_addr & 0xFFFF;
    code[0] = 0xD2800000 | (low16 << 5) | 0x11;

    if (target_addr & 0xFFFF0000) {
        // MOVK X17, #imm16, LSL #16
        uint64_t mid16 = (target_addr >> 16) & 0xFFFF;
        code[1] = 0xF2800000 | (mid16 << 5) | (1 << 21) | 0x11;

        // BR X17
        code[2] = 0xD61F0220;
    } else {
        // BR X17
        code[1] = 0xD61F0220;
    }
}

#elif defined(__riscv64)
// Generate:
//     Save current Entry to t0 and jump to address Entry-N
static void generate_thunk(char *buf, size_t N) {
    uint32_t *instr = (uint32_t *) buf;
    int32_t imm = -(int32_t) N;

    // _start:
    // AUIPC t0, 0
    instr[0] = 0x00000297;

    // ADDI t0, t0, imm_low
    int32_t imm_low = imm & 0xFFF;
    instr[1] = (imm_low << 20) | (5 << 15) | (0 << 12) | (5 << 7) | 0x13;

    // JALR zero, 0(t0)
    instr[2] = (0 << 20) | (5 << 15) | (0 << 12) | (0 << 7) | 0x67;
}

// Generate:
//     Jump to address Entry
static void generate_jump(void *buf, void *target) {
    uint32_t *code = buf;
    uint64_t addr = (uint64_t) target;

    // 1. AUIPC T1, offset[31:12]
    int32_t offset_hi = (addr - (uint64_t) buf) >> 12;
    code[0] = 0x00000317 | ((offset_hi & 0xFFFFF) << 12);

    // 2. ADDI T1, T1, offset[11:0]
    int32_t offset_lo = addr & 0xFFF;
    code[1] = 0x00030313; // addi t1, t1, 0
    code[1] |= (offset_lo & 0xFFF) << 20;

    // 3. JALR zero, t1
    code[2] = 0x00030067; // jalr zero, t1, 0
}
#endif

struct LORE_CALLBACK_TRAMPOLINE_CONTEXT *Lore_AllocCallbackTrampoline(size_t count, void *target) {
    size_t ctx_size = sizeof(struct LORE_CALLBACK_TRAMPOLINE_CONTEXT) +
                      count * sizeof(struct LORE_CALLBACK_TRAMPOLINE);
    struct LORE_CALLBACK_TRAMPOLINE_CONTEXT *trampoline = mmap(
        NULL, ctx_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    generate_jump(trampoline->jump_instr, target);
    trampoline->count = count;
    for (int i = 0; i < count; i++) {
        struct LORE_CALLBACK_TRAMPOLINE *thunk = &trampoline->trampoline[i];
        generate_thunk(thunk->thunk_instr, (intptr_t) thunk->thunk_instr - (intptr_t) trampoline);
    }
    return trampoline;
}

void Lore_FreeCallbackTrampoline(struct LORE_CALLBACK_TRAMPOLINE_CONTEXT *ctx) {
    size_t ctx_size = sizeof(struct LORE_CALLBACK_TRAMPOLINE_CONTEXT) +
                      ctx->count * sizeof(struct LORE_CALLBACK_TRAMPOLINE);
    munmap(ctx, ctx_size);
}