#ifndef LORE_BASE_PASSTHROUGH_CALLBACKTRAMPOLINE_H
#define LORE_BASE_PASSTHROUGH_CALLBACKTRAMPOLINE_H

#include <cstdint>
#include <cstddef>
#include <cassert>

namespace lore {

    /// CallbackTrampoline - A tiny executable stub handed out in place of a real callback pointer.
    ///
    /// Calling \c thunk_instr loads this instance's own address into the platform scratch register,
    /// then jumps to its table's shared \c jump_instr. That shared stub recovers \c saved_callback
    /// into the scratch register and tail-jumps to the table's common \a target, so the target
    /// learns which original callback was invoked from that register (\c %r11 on x86_64; see the
    /// per-arch \c Trampoline_codegen_*).
    struct CallbackTrampoline {
        /// The original callback this stub stands in for. Restored into the scratch register just
        /// before control reaches the table's target.
        void *saved_callback;
        /// Per-instance executable stub; its address is the surrogate callback pointer handed out.
        char thunk_instr[16]; // lea -7(%rip), %r11; jmp jump_instr
    };

    /// CallbackTrampolineTable - A contiguous, executable (\c mmap RWX) table of
    /// \c CallbackTrampoline instances that share one landing stub. Layout: \c jump_instr, then
    /// \c count instances in \c trampoline[].
    struct CallbackTrampolineTable {
        /// Shared landing stub all instances jump into: recovers the per-instance \c saved_callback
        /// from the scratch register and tail-jumps to the table's \a target.
        char jump_instr[32]; // mov -8(%r11), %r11; mov target, %rax; jmp *%rax
        /// Number of \c CallbackTrampoline instances in \c trampoline[].
        size_t count;
        /// The \c count trampoline instances (flexible array member).
        struct CallbackTrampoline trampoline[];

        /// Allocate an executable table of \a count trampolines that all route to \a target. Each
        /// instance starts with a null \c saved_callback; fill it in before handing out the stub.
        static CallbackTrampolineTable *create(size_t count, void *target);

        /// Unmap and free a table returned by \c create.
        static void destroy(CallbackTrampolineTable *table);
    };

}

#endif // LORE_BASE_PASSTHROUGH_CALLBACKTRAMPOLINE_H
