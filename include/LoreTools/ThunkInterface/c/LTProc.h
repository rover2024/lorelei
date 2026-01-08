// SPDX-License-Identifier: MIT

#ifndef LORE_TOOLS_THUNKINTERFACE_LTPROC_H
#define LORE_TOOLS_THUNKINTERFACE_LTPROC_H

#include <stddef.h>

#include <LoreBase/CoreLib/Global.h>

LORE_BEGIN_EXTERN_C

enum LTProcKind {
    LTProcKind_Function,
    LTProcKind_Callback,
    LTProcKind_NumProcKind,
};

enum LTProcDirection {
    LTProcDirection_GuestToHost,
    LTProcDirection_HostToGuest,
    LTProcDirection_NumProcDirection,
};

enum LTProcPhase {
    LTProcPhase_Entry,
    LTProcPhase_Caller,
    LTProcPhase_Exec,
    LTProcPhase_NumPhases,
};

struct LTProcInfo {
    const char *name;
    void *addr;
};

struct LTProcInfoArrayRef {
    LTProcInfo *arr;
    size_t size;
};

struct LTProcInfoContext {
    LTProcInfoArrayRef guestProcs[LTProcKind_NumProcKind][LTProcDirection_NumProcDirection];
    LTProcInfoArrayRef hostProcs[LTProcKind_NumProcKind][LTProcDirection_NumProcDirection];
    LTProcInfoArrayRef thisProcs;
    void *emuAddr;
};

LORE_END_EXTERN_C

#endif // LORE_TOOLS_THUNKINTERFACE_LTPROC_H
