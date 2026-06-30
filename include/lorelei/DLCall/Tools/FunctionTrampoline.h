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
    /// Calling \c thunk_instr loads this instance's address into the platform scratch register and
    /// jumps to the table's shared \c jump_instr, which recovers \c saved_function into that register
    /// and tail-jumps to the table's \a target, so the target learns which original function was
    /// invoked from that register (\c %r11 on x86_64, see the per-arch \c Trampoline_codegen_*). The
    /// \c magic_sign tags the block so guard code holding only the stub address can recognize it as
    /// one of ours and recover the original \c saved_function (e.g. to undo a decayed function pointer
    /// back to its real address, or to revert a callback that has crossed back over the boundary).
    struct FunctionTrampoline {
        /// The original function this stub stands in for. Restored into the scratch register just
        /// before control reaches the table's target.
        void *saved_function;
        /// Per-instance executable stub. Its address is the surrogate function pointer handed out.
        char thunk_instr[16]; // lea -7(%rip), %r11; jmp jump_instr
        /// Sentinel identifying this block as one of our trampolines, so the stub address can be
        /// detected and reverted to \c saved_function.
        uintptr_t magic_sign;
    };

    /// FunctionTrampolineTable - A contiguous, executable (\c mmap RWX) table of
    /// \c FunctionTrampoline instances that share one landing stub. Layout: \c jump_instr, then
    /// \c count instances in \c trampoline[].
    struct LOREDLCALL_EXPORT FunctionTrampolineTable {
        /// Shared landing stub all instances jump into: recovers the per-instance \c saved_function
        /// from the scratch register and tail-jumps to the table's \a target.
        char jump_instr[32]; // mov -8(%r11), %r11; mov target, %rax; jmp *%rax
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

}

#endif // LORE_DLCALL_FUNCTIONTRAMPOLINE_H
