#ifndef LOREUSER_H
#define LOREUSER_H

#include <lorelei/loreshared.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LOREUSER_SYSCALL_NUM 114514

enum LOREUSER_CALL_TYPE {
    LOREUSER_CT_CheckHealth,

    LOREUSER_CT_LoadLibrary,
    LOREUSER_CT_FreeLibrary,
    LOREUSER_CT_GetProcAddress,
    LOREUSER_CT_GetErrorMessage,
    LOREUSER_CT_GetModulePath,

    LOREUSER_CT_CallHostProc,
    LOREUSER_CT_ResumeHostProc, // internal

    LOREUSER_CT_AddCallbackThunk,
    LOREUSER_CT_GetGuestLibraryData,

    // internal
    LOREUSER_CT_LA_ObjOpen,
    LOREUSER_CT_LA_ObjClose,
    LOREUSER_CT_LA_PreInit,
    LOREUSER_CT_LA_SymBind,
};

enum LOREUSER_PROC_RESULT {
    LOREUSER_PR_Finished = 0,
    LOREUSER_PR_Callback,
    LOREUSER_PR_PThreadCreate,
    LOREUSER_PR_PThreadExit,
};

enum LOREUSER_PROC_CONVENTION {
    LOREUSER_PC_Function,
    LOREUSER_PC_ThreadEntry,
};

#ifdef LORELEI_GUEST_LIBRARY

//
// Loader APIs, implemented in emulator
//
LORELEI_EXPORT void *Lore_LoadLibrary(const char *path, int flags);

LORELEI_EXPORT int Lore_FreeLibrary(void *handle);

LORELEI_EXPORT void *Lore_GetProcAddress(void *handle, const char *name);

LORELEI_EXPORT char *Lore_GetErrorMessage();

LORELEI_EXPORT char *Lore_GetModulePath(void *addr, bool isHandle);


//
// Native proc APIs, implemented in emulator
//
LORELEI_EXPORT void Lore_CallHostProc(void *func, void **args, void *ret, int convention, void **metadata);


//
// Loader metadata APIs, implemented in host runtime
//
LORELEI_EXPORT void Lore_AddCallbackThunk(const char *sign, void *func);

LORELEI_EXPORT struct LORE_GUEST_LIBRARY_DATA *Lore_GetGuestLibraryData(const char *path, int libType);

#endif

#ifdef __cplusplus
}
#endif

#endif // LOREUSER_H