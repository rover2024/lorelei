// SPDX-License-Identifier: MIT

#ifndef LORE_DLCALL_PROCDEFS_H
#define LORE_DLCALL_PROCDEFS_H

#include <cstddef>

namespace lore::thunk {

    /// ProcKind - What kind of thunked proc an entry describes.
    enum ProcKind {
        Function, ///< A real exported library function.
        Callback, ///< A function pointer passed across the boundary (called back later).
        NumProcKind,
    };

    /// ProcDirection - Which side calls which across the boundary.
    enum ProcDirection {
        GuestToHost, ///< Guest invokes a host proc (the common case).
        HostToGuest, ///< Host invokes a guest proc (a callback the guest registered).
        NumProcDirection,
    };

    /// ProcPhase - The per-proc thunk layers, chained Entry -> Adapt -> Caller -> Exec.
    ///
    /// Each layer is a distinct \c ProcFn / \c ProcCb specialization with its own \c invoke. A
    /// manifest can override a single layer without rewriting the others.
    enum ProcPhase {
        Entry,  ///< Wire boundary: converts the raw \c args[] buffer to and from typed arguments.
        Adapt,  ///< Typed adaptation injected by the non-Builder passes (callback substitution,
                ///< type/handle filtering, GetProcAddress), a pass-through skeleton by default.
        Caller, ///< Constructs the actual call (default forwarding, or the printf wrapper).
        Exec,   ///< The real library call, or the cross-boundary invoke into the other side.
        NumProcPhase,
        /// \note The generator emits only Entry, Adapt and Caller. \c Exec is fixed boilerplate,
        /// so its value (3) also serves as the count of generated phases (the per-phase arrays are
        /// sized to \c Exec).
    };

    /// ProcInfoPair - One (symbol name, address) entry in a proc table.
    struct ProcInfoPair {
        const char *key; ///< The proc's symbol name.
        void *addr;      ///< Its resolved thunk / invoke address.
    };

    /// ProcArrayRef - A non-owning counted view over a \c ProcInfoPair array.
    ///
    /// \c size is the number of valid entries. Generated tables may reserve an extra zeroed
    /// sentinel slot, but consumers should use \c size rather than walking to a null key.
    struct ProcArrayRef {
        ProcInfoPair *arr;
        size_t size;
    };

    /// StaticThunkContext - The per-manifest cross-side contract shared by the guest and host.
    ///
    /// Each side fills its own entries, and at exchange time the guest and host copy each other's
    /// tables so both can resolve the other side's proc addresses.
    struct StaticThunkContext {
        /// The guest-side entries, indexed by [kind][direction], populated in the generated guest
        /// code.
        ProcArrayRef guestProcs[NumProcKind][NumProcDirection];
        /// The host-side entries, indexed by [kind][direction], assigned in the initializer.
        ProcArrayRef hostProcs[NumProcKind][NumProcDirection];
        /// This side's real library functions, resolved via \c dlsym from the host library when
        /// \c autoLink is false (used by \c ProcFn<GuestToHost, Exec>).
        ProcArrayRef thisProcs;
        /// The emulator address boundary, used by the optional address-separation \c isHostAddress
        /// path.
        void *emuAddr;
        /// Host manifest only: true when AUTO_LINK folded the real symbol addresses in at link
        /// time, so the host runtime skips \c dlopen / \c dlsym of the real library.
        bool autoLink;
        /// The next library this thunk forwards to, baked in at build time (a guest thunk's host thunk,
        /// a host thunk's real library). How the value resolves:
        /// - a bare filename is taken to be on the loader's search path
        /// - an absolute path is used as is
        /// - a relative path (any value containing '/', so a same-directory target is written "./name")
        ///   is joined with this thunk's own directory
        /// Null falls back to the name convention: <name>_HTL.so for a guest thunk, <name>.so for a
        /// host thunk.
        const char *nextLibraryPath;
    };

}

#endif // LORE_DLCALL_PROCDEFS_H
