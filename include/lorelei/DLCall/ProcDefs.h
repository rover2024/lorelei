// SPDX-License-Identifier: MIT

#ifndef LORE_DLCALL_PROCDEFS_H
#define LORE_DLCALL_PROCDEFS_H

#include <cstddef>

namespace lore::thunk {

    enum ProcKind {
        Function,
        Callback,
        NumProcKind,
    };

    enum ProcDirection {
        GuestToHost,
        HostToGuest,
        NumProcDirection,
    };

    enum ProcPhase {
        Entry,
        Adapt,
        Caller,
        Exec,
        NumProcPhase,
    };

    struct ProcInfoPair {
        const char *key;
        void *addr;
    };

    struct ProcArrayRef {
        ProcInfoPair *arr;
        size_t size;
    };

    struct StaticThunkContext {
        ProcArrayRef guestProcs[NumProcKind][NumProcDirection];
        ProcArrayRef hostProcs[NumProcKind][NumProcDirection];
        ProcArrayRef thisProcs;
        void *emuAddr;
        bool autoLink; // available in host manifest
    };

}

#endif // LORE_DLCALL_PROCDEFS_H
