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
    /// Calling \c thunk_instr calls the table's shared \a target (a normal C handler) and then
    /// returns to the original caller. The identity of the original function is carried on the stack:
    /// the handler recovers it from its own return address, which points back into this stub (see
    /// \c origin).
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
        /// The layout is per-arch (see the \c Trampoline_codegen_*): it calls the table's target and
        /// then returns.
        char thunk_instr[16];
        /// Sentinel identifying this block as one of our trampolines, so the stub address can be
        /// detected and reverted to \c saved_function.
        uintptr_t magic_sign;

        /// Offset, within \c thunk_instr, of the point a called handler returns to (the instruction
        /// right after the stub's \c call). \c origin subtracts this to recover the trampoline from a
        /// return address. It is fixed by the per-arch stub template in \c Trampoline_codegen_* and
        /// MUST match it.
#if defined(__x86_64__)
        static constexpr size_t kResumeOffset = 13; // movabs $target,%r11 (10) + call *%r11 (3)
#elif defined(__aarch64__)
        static constexpr size_t kResumeOffset = 0; // TODO: set from the aarch64 stub template
#elif defined(__riscv) && __riscv_xlen == 64
        static constexpr size_t kResumeOffset = 0; // TODO: set from the riscv64 stub template
#endif

        /// Recover the function a stub stands in for, from a handler's return address. \a ra is the
        /// handler's \c __builtin_return_address(0), which points \c kResumeOffset bytes into the
        /// stub. The handler's address is taken to build the stub, so it is out of line and this
        /// return address is the stub.
        static inline void *origin(void *ra);
    };

    /// FunctionTrampolineTable - A contiguous, executable (\c mmap RWX) table of
    /// \c FunctionTrampoline instances that all route to one \a target. Each instance embeds the
    /// target itself, so there is no shared landing stub.
    struct LOREDLCALL_EXPORT FunctionTrampolineTable {
        /// Number of \c FunctionTrampoline instances in \c trampoline[].
        size_t count;
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
