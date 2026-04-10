#ifndef LORE_BASE_PASSTHROUGH_CPROC_H
#define LORE_BASE_PASSTHROUGH_CPROC_H

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
    struct CStaticProcInfo *arr;
    size_t size;
};

struct CStaticCallCheckGuardInfo {
    const char *signature;
    void **pTramp;
};

struct CStaticFunctionDecayGuardPair {
    void *hostAddr;
    void *guestTramp;
};

struct CStaticFunctionDecayGuardInfo {
    const char *signature;
    int count;
    CStaticFunctionDecayGuardPair **pProcs;
};

struct CStaticThunkContext {
    // Thunk library data
    struct CStaticProcInfoArrayRef guestProcs[CProcKind_NumProcKind]
                                             [CProcDirection_NumProcDirection];
    struct CStaticProcInfoArrayRef hostProcs[CProcKind_NumProcKind]
                                            [CProcDirection_NumProcDirection];
    struct CStaticProcInfoArrayRef thisProcs;

    // Emulator data
    void *emuAddr;

    // Patched host library data
    size_t numCCGs;
    CStaticCallCheckGuardInfo *CCGs;
    size_t numFDGs;
    CStaticFunctionDecayGuardInfo *FDGs;
};

#ifdef __cplusplus
}
#endif

#endif // LORE_BASE_PASSTHROUGH_CPROC_H
