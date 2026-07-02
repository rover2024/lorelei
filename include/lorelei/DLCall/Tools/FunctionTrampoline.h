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
    /// Calling \c thunk_instr loads this instance's \c saved_function into a scratch register and
    /// tail-branches to the shared \c lore_tramp_shim, which parks that value in the
    /// \c thread_last_callback thread-local and tail-branches on to the table's \a target (a normal C
    /// handler). Because both branches are tail branches, the handler sees the caller's arguments
    /// exactly as passed (including any on the stack), and it recovers which callback it stands in
    /// for by reading \c thread_last_callback. No reserved register and no return-address games, so
    /// the handler compiles cleanly under any compiler.
    ///
    /// The \c magic_sign tags the block so guard code holding only the stub address can recognize it
    /// as one of ours and recover the original \c saved_function (e.g. to undo a decayed function
    /// pointer back to its real address, or to revert a callback that has crossed back over the
    /// boundary).
    struct FunctionTrampoline {
        /// The original function this stub stands in for. The stub loads it (PC-relative, from this
        /// field) into the identity register on its way to the shim, so setting it is a plain field
        /// write with no code patching.
        void *saved_function;
        /// Per-instance executable stub. Its address is the surrogate function pointer handed out.
        /// The layout is per-arch (see the \c Trampoline_codegen_*): load \c saved_function, then
        /// tail-branch to \c lore_tramp_shim carrying the table's target. Sized for the largest stub
        /// (riscv64, which loads its literals through auipc + ld pairs). x86_64 uses 29 bytes.
        char thunk_instr[40];
        /// Sentinel identifying this block as one of our trampolines, so the stub address can be
        /// detected and reverted to \c saved_function.
        uintptr_t magic_sign;
    };

    /// FunctionTrampolineTable - A contiguous, executable (\c mmap RWX) table of
    /// \c FunctionTrampoline instances that all route to one \a target through the shared shim. Each
    /// stub embeds \a target and the shim address, so there is no per-table landing block.
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

}

#endif // LORE_DLCALL_FUNCTIONTRAMPOLINE_H
