// SPDX-License-Identifier: MIT

#ifndef LORELEI_THUNKINTEFACE_PROC_C_H
#define LORELEI_THUNKINTEFACE_PROC_C_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

enum CProcKind {
    CProcKind_Function,
    CProcKind_Callback,
    CProcKind_NumProcKind,
};

enum CProcDirection {
    CProcDirection_GuestToHost,
    CProcDirection_HostToGuest,
    CProcDirection_NumProcDirection,
};

enum CProcPhase {
    CProcPhase_Entry,
    CProcPhase_Caller,
    CProcPhase_Exec,
    CProcPhase_NumPhases,
};

struct CStaticProcInfo {
    const char *name;
    void *addr;
};

struct CStaticProcInfoArrayRef {
    CStaticProcInfo *arr;
    size_t size;
};

struct CStaticProcInfoContext {
    CStaticProcInfoArrayRef gtpEntries[CProcKind_NumProcKind][CProcDirection_NumProcDirection];
    CStaticProcInfoArrayRef htpEntries[CProcKind_NumProcKind][CProcDirection_NumProcDirection];
    CStaticProcInfoArrayRef libEntries;
    void *emuAddr;
};

#ifdef __cplusplus
}
#endif

#endif // LORELEI_THUNKINTEFACE_PROC_C_H
