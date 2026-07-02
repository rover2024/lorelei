// SPDX-License-Identifier: MIT

#ifndef LORE_DLCALL_FUNCTIONTRAMPOLINE_H
#define LORE_DLCALL_FUNCTIONTRAMPOLINE_H

#include <cstdint>
#include <cstddef>
#include <cassert>

#include <lorelei/DLCall/Global.h>

namespace lore {

    /// FunctionTrampoline - A tiny executable stub handed out in place of a real function pointer.
    ///
    /// Calling \c thunk_instr calls the table's shared entry (see \c FunctionTrampolineTable), which
    /// tail-branches to the table's one \a target (a normal C handler); the stub then returns to the
    /// original caller. The identity of the original function is carried on the stack: the handler
    /// recovers it from its own return address, which points back into this stub (see \c origin).
    ///
    /// The per-instance stub therefore holds only the link into the shared entry and the point the
    /// handler returns through. Loading \a target, its 8-byte literal, and (on the callee-return
    /// arches) the stack frame that preserves the caller's return address all live once in the
    /// shared entry, not in every stub.
    ///
    /// The \c magic_sign tags the block so guard code holding only the stub address can recognize it
    /// as one of ours and recover the original \c saved_function (e.g. to undo a decayed function
    /// pointer back to its real address, or to revert a callback that has crossed back over the
    /// boundary).
    struct FunctionTrampoline {
        /// The original function this stub stands in for. Recovered by the handler from its return
        /// address (which lands \c kResumeOffset bytes into \c thunk_instr).
        void *saved_function;
        /// Per-instance executable stub. Its address is the surrogate function pointer handed out.
        /// The layout is per-arch (see the \c Trampoline_codegen_*): it links into the table's shared
        /// entry and the handler returns back through it. It is tiny: riscv64 uses 8 bytes, aarch64
        /// 12, x86_64 6. Sized to 16 so the following \c magic_sign stays naturally aligned.
        char thunk_instr[16];
        /// Sentinel identifying this block as one of our trampolines, so the stub address can be
        /// detected and reverted to \c saved_function.
        uintptr_t magic_sign;

        /// Offset, within \c thunk_instr, of the point a called handler returns to (the instruction
        /// right after the stub's call to the shared entry). \c origin subtracts this to recover the
        /// trampoline from a return address. It is fixed by the per-arch stub template in
        /// \c Trampoline_codegen_* and MUST match it.
#if defined(__x86_64__)
        static constexpr size_t kResumeOffset = 5; // call <shared> (5)
#elif defined(__aarch64__)
        static constexpr size_t kResumeOffset = 8; // adr x9 (4) + b <prologue> (4)
#elif defined(__riscv) && __riscv_xlen == 64
        static constexpr size_t kResumeOffset = 4; // jal t0, <prologue> (4)
#endif

        /// Recover the function a stub stands in for, from a handler's return address. \a ra is the
        /// handler's \c __builtin_return_address(0), which points \c kResumeOffset bytes into the
        /// stub. The handler's address is taken to build the shared entry, so it is out of line and
        /// this return address is the stub, not the handler itself.
        static inline void *origin(void *ra);
    };

    /// FunctionTrampolineTable - A contiguous, executable (\c mmap RWX) table of
    /// \c FunctionTrampoline instances that all route to one \a target through a shared entry.
    ///
    /// The \c shared_entry, built once, does everything common to the instances so each stub stays
    /// tiny. On x86_64 it just loads \a target and tail-jumps to it. On the callee-return arches it
    /// is a prologue plus an epilogue: the prologue opens a frame, saves the caller's return address,
    /// moves the per-instance identity the stub passed it into the return-address register, and
    /// tail-branches to \a target; the handler returns into the stub, which jumps to the epilogue to
    /// restore the caller's return address and return. Keeping the target load, its literal, and the
    /// frame here (rather than in every stub) is what keeps \c thunk_instr small.
    struct LOREDLCALL_EXPORT FunctionTrampolineTable {
        /// Number of \c FunctionTrampoline instances in \c trampoline[].
        size_t count;
        /// Shared landing built once (see the struct comment). Sized for the largest arch entry
        /// (riscv64: a 6-instruction prologue, an 8-byte target literal, and a 3-instruction
        /// epilogue).
        char shared_entry[48];
        /// The \c count trampoline instances (flexible array member).
        struct FunctionTrampoline trampoline[];

        /// Allocate an executable table of \a count trampolines that all route to \a target, each
        /// stamped with \a magic_sign. Every instance starts with a null \c saved_function. Fill it
        /// in before handing out the stub.
        static FunctionTrampolineTable *create(size_t count, void *target, uintptr_t magic_sign);

        /// Unmap and free a table returned by \c create.
        static void destroy(FunctionTrampolineTable *table);
    };

    // Defined out of class: offsetof needs the complete type.
    inline void *FunctionTrampoline::origin(void *ra) {
        auto *t = reinterpret_cast<FunctionTrampoline *>(
            reinterpret_cast<char *>(ra) - kResumeOffset - offsetof(FunctionTrampoline, thunk_instr));
        return t->saved_function;
    }

}

#endif // LORE_DLCALL_FUNCTIONTRAMPOLINE_H
