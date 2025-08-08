#include "loreshared.h"

#include <sys/mman.h>

#ifdef __x86_64__
// Generate:
//     Save current Entry to R11 and jump to address Entry-N
static void generate_thunk(void *buf, int64_t N) {
    struct instr {
        struct {
            char op[3];
            int32_t displacement;
        } __attribute__((packed)) lea; // leaq -N(%rip), %r11
        struct {
            char op[1];
            int32_t offset;
        } __attribute__((packed)) jmp_rel32; // jmp N(%rip)
    } __attribute__((packed));

    struct instr *instr = (struct instr *) buf;
    instr->lea.op[0] = 0x4C; // REX.W prefix
    instr->lea.op[1] = 0x8D; // LEA opcode
    instr->lea.op[2] = 0x1D; // ModR/M: r11 with RIP-relative addressing
    instr->lea.displacement = -7;

    instr->jmp_rel32.op[0] = 0xE9;     // JMP opcode
    instr->jmp_rel32.offset = -12 - N; // N bytes from last instruction
}

// Generate:
//     Jump to address Entry
static void generate_jump(void *buf, void *target) {
    struct instr {
        char mov[4]; // mov -0x8(%r11),%r11
        struct {
            char op[2];
            uint64_t imm;
        } __attribute__((packed)) mov_target; // movabs $target, %rax
        char jmp[2];                          // jmp *%rax
    } __attribute__((packed));

    struct instr *instr = (struct instr *) buf;
    instr->mov[0] = 0x4D; // REX.W prefix
    instr->mov[1] = 0x8B; // MOV opcode
    instr->mov[2] = 0x5B; // ModR/M: r11 with RIP-relative addressing
    instr->mov[3] = 0xF8; // -8

    instr->mov_target.op[0] = 0x48;            // REX.W prefix
    instr->mov_target.op[1] = 0xB8;            // MOV opcode
    instr->mov_target.imm = (uint64_t) target; // target address

    instr->jmp[0] = 0xFF; // JMP opcode
    instr->jmp[1] = 0xE0; // ModR/M: indirect jump with RAX
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
    struct instr {
        uint32_t ldr_x16;
        uint32_t ldr_x17;
        uint32_t br_x17;
        uint64_t target_addr;
    } __attribute__((packed));
    struct instr *instr = (struct instr *) buf;
    instr->ldr_x16 = 0xF85F8210; // ldr x16, [x16, #-8]
    instr->ldr_x17 = 0x58000051; // ldr x17, 8 <pc+8>
    instr->br_x17 = 0xD61F0200;  // br x17
    instr->target_addr = (uint64_t) target;
}

#elif defined(__riscv)
// Generate:
//     Save current Entry to t1 and jump to address Entry-N
static void generate_thunk(char *buf, size_t N) {
    uint32_t *instr = (uint32_t *) buf;
    int32_t imm = -(int32_t) N;

    // _start:
    // auipc t1, 0
    instr[0] = 0x00000317;

    // jal x0, offset
    int32_t offset = (-4 - N) / 2;
    instr[1] = 0x0000006F | ((offset & 0xFFFFF) << 12);
}

// Generate:
//     Jump to address Entry
static void generate_jump(void *buf, void *target) {
    struct instr {
        uint32_t auipc_t0;
        uint32_t ld_t0_12;
        uint32_t jr_t0;
        uint64_t target_addr;
    } __attribute__((packed));
    struct instr *instr = (struct instr *) buf;
    instr->auipc_t0 = 0x00000297; // auipc t0, 0
    instr->ld_t0_12 = 0x00C2B283; // ld t0, 12(t0)
    instr->jr_t0 = 0x00028067;    // jr t0
    instr->target_addr = (uint64_t) target;
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