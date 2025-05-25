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
    LOREUSER_CT_GetAddressBoundary,

    LOREUSER_CT_InvokeHostProc,
    LOREUSER_CT_ResumeHostProc, // internal

    LOREUSER_CT_GetLibraryData,
    LOREUSER_CT_CallHostHelper,

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
    LOREUSER_PR_HostLibraryOpen,
};

enum LOREUSER_PROC_CONVENTION {
    LOREUSER_PC_Function,
    LOREUSER_PC_ThreadEntry,
    LOREUSER_PC_HostCallback,
};

enum LOREUESR_HELPER_ID {
    LOREUSER_HI_ExtractPrintFArgs = 1,
    LOREUSER_HI_ExtractSScanFArgs,
};

union LOREUSER_PROC_NEXTCALL {
    // LOREUSER_PR_Callback
    struct {
        void *thunk;
        void *callback;
        void *args;
        void *ret;
        void *metadata;
    } callback;

    // LOREUSER_PR_PThreadCreate
    struct {
        void *thread;
        void *attr;
        void *start_routine;
        void *arg;
        int *ret;
    } pthread_create;

    // LOREUSER_PR_PThreadExit
    struct {
        void *ret;
    } pthread_exit;

    // LOREUSER_PR_HostLibraryOpen
    struct {
        const char *id;
    } host_library_open;
};

#ifdef LORELEI_GUEST_LIBRARY

//
// Loader APIs, implemented in emulator
//
LORELEI_EXPORT void *Lore_LoadLibrary(const char *path, int flags);

LORELEI_EXPORT int Lore_FreeLibrary(void *handle);

LORELEI_EXPORT void *Lore_GetProcAddress(void *handle, const char *name);

LORELEI_EXPORT char *Lore_GetErrorMessage();

LORELEI_EXPORT char *Lore_GetModulePath(void *addr, int isHandle);

LORELEI_EXPORT void *Lore_GetAddressBoundary();


//
// Native proc APIs, implemented in emulator
//
LORELEI_EXPORT void Lore_InvokeHostProc(int convention, void *func, void **args, void *ret, void **metadata);


//
// Loader metadata APIs, implemented in host runtime
//
LORELEI_EXPORT void *Lore_GetLibraryData(const char *path, int isThunk);

LORELEI_EXPORT void Lore_CallHostHelper(int id, void **args, void *ret);

//
// Guest thunk utility APIs
//
LORELEI_EXPORT void *Lore_LoadHostThunkLibrary(void *someAddr, int thunkCount, void **thunks);

LORELEI_EXPORT void Lore_FreeHostThunkLibrary(void *handle);

LORELEI_EXPORT void *Lore_GetHostThunkProcAddress(void *handle, const char *name);

LORELEI_EXPORT void *Lore_ConvertHostProcAddress(const char *name, void *addr);

#endif

#ifdef __cplusplus
}
#endif

#endif // LOREUSER_H