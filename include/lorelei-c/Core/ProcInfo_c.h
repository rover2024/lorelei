#ifndef PROCINFOCDEFS_H
#define PROCINFOCDEFS_H

#include <stddef.h>

#ifdef __cplusplus
#  define _PROCINFO_CONSTEXPR constexpr
#else
#  define _PROCINFO_CONSTEXPR
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum CProcKind {
    CProcKind_HostFunction,
    CProcKind_GuestFunction, // not used
    CProcKind_HostCallback,
    CProcKind_GuestCallback,
    CProcKind_NumProcKind,
};

static inline _PROCINFO_CONSTEXPR bool CProcKind_isHost(CProcKind kind) {
    return kind == CProcKind_HostFunction || kind == CProcKind_HostCallback;
}

static const char *CProcKindNames[CProcKind_NumProcKind] = {
    "HostFunction",
    "GuestFunction",
    "HostCallback",
    "GuestCallback",
};

enum CProcThunkPhase {
    CProcThunkPhase_GTP,      /// Guest Thunk Proc
    CProcThunkPhase_GTP_IMPL, /// Guest Thunk Proc Implementation
    CProcThunkPhase_HTP,      /// Host Thunk Proc
    CProcThunkPhase_HTP_IMPL, /// Host Thunk Proc Implementation
    CProcThunkPhase_NumThunkPhase,
};

static const char *CProcThunkPhaseNames[CProcThunkPhase_NumThunkPhase] = {
    "GTP",
    "GTP_IMPL",
    "HTP",
    "HTP_IMPL",
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
    CStaticProcInfoArrayRef gtpEntries[CProcKind_NumProcKind];
    CStaticProcInfoArrayRef htpEntries[CProcKind_NumProcKind];
    CStaticProcInfoArrayRef libEntries;

    void *emuAddr;
};

#ifdef __cplusplus
}
#endif

#endif // PROCINFOCDEFS_H
