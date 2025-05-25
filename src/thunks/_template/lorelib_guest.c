#define _GNU_SOURCE

#include <dlfcn.h>
#include <limits.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lorelei/loreuser.h>

#include "lorelib_defs.h"

//
// Declare helpers
//
static void LoreLib_PreInitialize();
static void LoreLib_PostInitialize();



//
// Declare API function pointers
//
enum LoreLib_ApiEnum {
#define _F(NAME) LoreLib_Api_##NAME,
    LORELIB_API_FOREACH(_F)
#undef _F

        LoreLib_ApiEnumSize,
};
enum LoreLib_GCBEnum {
#define _F(NAME, SIGNATURE) LoreLib_GCB_##NAME,
    LORELIB_GCB_FOREACH(_F)
#undef _F

        LoreLib_GCBEnumSize,
};
enum LoreLib_HCBEnum {
#define _F(NAME, SIGNATURE) LoreLib_HCB_##NAME,
    LORELIB_HCB_FOREACH(_F)
#undef _F

        LoreLib_HCBEnumSize,
};

static void *LoreLib_API_GTPs[] = {
#define _F(NAME) NULL,
    LORELIB_API_FOREACH(_F)
#undef _F
};
static const char *LoreLib_API_Names[] = {
#define _F(NAME) #NAME,
    LORELIB_API_FOREACH(_F)
#undef _F
};
static void *LoreLib_GCB_GTPs[LoreLib_GCBEnumSize];
static void *LoreLib_GCB_HTPs[LoreLib_GCBEnumSize];
static void *LoreLib_HCB_GTPs[LoreLib_HCBEnumSize];
static void *LoreLib_HCB_HTPs[LoreLib_HCBEnumSize];



//
// Declare context
//
struct LoreLib_Context {
    void *Handle;
    void *AddressBoundary;
    void *HTLExchangeCallbacks;
};
static struct LoreLib_Context LoreLibCtx = {0};



//
// Constructor/Destructor
//
static void __attribute__((constructor)) LoreLib_Inititialize() {
    LoreLib_PreInitialize();

    // 1. Load library
    char path[PATH_MAX];
    if (!Lore_RevealLibraryPath(path, LoreLib_Inititialize, 0)) {
        fprintf(stderr, "Unknown GTL: failed to get library path\n");
        abort();
    }
    void *handle = Lore_LoadHostThunkLibrary(LoreLib_Inititialize, LoreLib_ApiEnumSize, LoreLib_API_GTPs);
    if (!handle) {
        fprintf(stderr, "%s: GTL: failed to load HTL\n", path);
        abort();
    }
    LoreLibCtx.Handle = handle;

    // 2. Get function addresses
    for (int i = 0; i < LoreLib_ApiEnumSize; ++i) {
        void *proc = Lore_GetHostThunkProcAddress(handle, LoreLib_API_Names[i]);
        if (!proc) {
            fprintf(stderr, "%s: GTL: failed to resolve API \"%s\" in HTL\n", path, LoreLib_API_Names[i]);
            abort();
        }
        LoreLib_API_GTPs[i] = proc;
    }

    // 3. Set variables
    LoreLibCtx.AddressBoundary = Lore_GetAddressBoundary();

    // 4. Exchange callbacks
    {
        typedef void (*ExchangeCallbacks)(void **, void **);
        static const char *name = "LoreLib_HTLExchangeCallbacks";
        ExchangeCallbacks proc = Lore_GetProcAddress(LoreLibCtx.Handle, name);
        if (!proc) {
            fprintf(stderr, "%s: GTL: failed to resolve reserved API \"%s\" in HTL\n", path, name);
            abort();
        }

        void *args[] = {
            LoreLib_GCB_GTPs,
            LoreLib_GCB_HTPs,
            LoreLib_HCB_GTPs,
            LoreLib_HCB_HTPs,
        };
        Lore_InvokeHostProc(LOREUSER_PC_Function, proc, args, NULL, NULL);
        LoreLibCtx.HTLExchangeCallbacks = proc;
    }

    LoreLib_PostInitialize();
}

static void __attribute__((destructor)) LoreLib_Quit() {
    // Free library
    Lore_FreeHostThunkLibrary(LoreLibCtx.Handle);
}



//
// Add Api implementations
//
extern __thread void *Lore_GRTThreadCallback;

static void LoreLib_ExecuteHostCallback(void *thunk, void *callback, void **args, void *ret, void **metadata) {
    void *newArgs[] = {
        callback,
        args,
    };
    Lore_InvokeHostProc(LOREUSER_PC_HostCallback, thunk, newArgs, ret, metadata);
}

#define LORELIB_BOUNDARY  LoreLibCtx.AddressBoundary
#define LORELIB_HTP(FUNC) LoreLib_API_GTPs[LoreLib_Api_##FUNC]
#define LORELIB_INVOKE_HTP(FUNC, ARGS, RET, METADATA)                                                                  \
    Lore_InvokeHostProc(LOREUSER_PC_Function, FUNC, ARGS, RET, METADATA)
#define LORELIB_HCB(FUNC) LoreLib_HCB_HTPs[LoreLib_HCB_##FUNC]
#define LORELIB_INVOKE_HCB(THUNK, FUNC, ARGS, RET, METADATA)                                                           \
    LoreLib_ExecuteHostCallback(THUNK, FUNC, ARGS, RET, METADATA)
#define LORELIB_LAST_HCB Lore_GRTThreadCallback
#define LORELIB_GTL_BUILD
#include "lorelib_manifest.c"



//
// Post definitions
//
static void *LoreLib_GCB_GTPs[LoreLib_GCBEnumSize] = {
#define _F(NAME, SIGNATURE) __GTP_GCB_##NAME,
    LORELIB_GCB_FOREACH(_F)
#undef _F
};
static void *LoreLib_HCB_GTPs[LoreLib_HCBEnumSize] = {
#define _F(NAME, SIGNATURE) __GTP_HCB_##NAME,
    LORELIB_HCB_FOREACH(_F)
#undef _F
};



//
// Implement helpers
//
static void LoreLib_PreInitialize() {
}

static void LoreLib_PostInitialize() {
}