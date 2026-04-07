#ifndef LORE_TOOLS_THUNKINTERFACE_CTPROC_H
#define LORE_TOOLS_THUNKINTERFACE_CTPROC_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

enum CTProcKind {
    CTProcKind_Function,
    CTProcKind_Callback,
    CTProcKind_NumProcKind,
};

enum CTProcDirection {
    CTProcDirection_GuestToHost,
    CTProcDirection_HostToGuest,
    CTProcDirection_NumProcDirection,
};

enum CTProcPhase {
    CTProcPhase_Entry,
    CTProcPhase_Caller,
    CTProcPhase_Exec,
    CTProcPhase_NumPhases,
};

struct CTProcInfo {
    const char *name;
    void *addr;
};

struct CTProcInfoArrayRef {
    struct CTProcInfo *arr;
    size_t size;
};

struct CTProcInfoContext {
    struct CTProcInfoArrayRef guestProcs[CTProcKind_NumProcKind][CTProcDirection_NumProcDirection];
    struct CTProcInfoArrayRef hostProcs[CTProcKind_NumProcKind][CTProcDirection_NumProcDirection];
    struct CTProcInfoArrayRef thisProcs;
    void *emuAddr;
};

#ifdef __cplusplus
}
#endif

#endif // LORE_TOOLS_THUNKINTERFACE_CTPROC_H
