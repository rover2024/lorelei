#define _GNU_SOURCE

#include <dlfcn.h>
#include <limits.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <lorelei/loreuser.h>
#include <lorelei/lorehapi.h>

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

static void *LoreLib_API_HTPs[] = {
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
    void (*ExecuteCallback)(void * /*thunk*/, void * /*callback*/, void * /*args*/, void * /*ret*/,
                            void * /*metadata*/);
};
static struct LoreLib_Context LoreLibCtx = {0};



//
// Constructor/Destructor
//
static void __attribute__((constructor)) LoreLib_Inititialize() {
    LoreLib_PreInitialize();

    // 1. Load library
    char path[PATH_MAX];
    if (!Lore_RevealLibraryPath(path, LoreLib_Inititialize, false)) {
        fprintf(stderr, "Unknown HTL: failed to get library path\n");
        abort();
    }
    void *handle = Lore_LoadHostLibrary(LoreLib_Inititialize, LoreLib_ApiEnumSize, LoreLib_API_HTPs);
    if (!handle) {
        fprintf(stderr, "%s: HTL: failed to load host library\n", path);
        abort();
    }
    LoreLibCtx.Handle = handle;

    // 2. Get function addresses
    for (int i = 0; i < LoreLib_ApiEnumSize; ++i) {
        void *proc = dlsym(handle, LoreLib_API_Names[i]);
        if (!proc) {
            fprintf(stderr, "%s: HTL: failed to resolve API \"%s\" in host library\n", path, LoreLib_API_Names[i]);
            abort();
        }
        LoreLib_API_HTPs[i] = proc;
    }

    // 3. Set variables
    struct _LoreEmuApis {
        void *apis[5];
    };
    struct _LoreEmuApis *emuApis = (struct _LoreEmuApis *) Lore_HrtGetEmuApis();
    LoreLibCtx.AddressBoundary = emuApis->apis[0];
    LoreLibCtx.ExecuteCallback = emuApis->apis[0];

    LoreLib_PostInitialize();
}

static void __attribute__((destructor)) LoreLib_Quit() {
    // Free library
    Lore_FreeHostLibrary(LoreLibCtx.Handle);
}



//
// Add Api implementations
//
extern __thread void *Lore_HRTThreadCallback;

LORELEI_DECL_EXPORT void LoreLib_HTLExchangeCallbacks(void **args, void *ret, void **metadata) {
    (void) ret;
    (void) metadata;

    void **GCB_GTPs = (void **) args[0]; // initialized by the guest
    void **GCB_HTPs = (void **) args[1];
    void **HCB_GTPs = (void **) args[2]; // initialized by the guest
    void **HCB_HTPs = (void **) args[3];

    for (int i = 0; i < LoreLib_GCBEnumSize; ++i) {
        LoreLib_GCB_GTPs[i] = GCB_GTPs[i];
        GCB_HTPs[i] = LoreLib_GCB_HTPs[i];
    }
    for (int i = 0; i < LoreLib_HCBEnumSize; ++i) {
        LoreLib_HCB_GTPs[i] = HCB_GTPs[i];
        HCB_HTPs[i] = LoreLib_HCB_HTPs[i];
    }
}

#define LORELIB_BOUNDARY  LoreLibCtx.AddressBoundary
#define LORELIB_API(FUNC) ((__typeof__(&FUNC)) LoreLib_API_HTPs[LoreLib_Api_##FUNC])
#define LORELIB_GCB(FUNC) LoreLib_GCB_GTPs[LoreLib_GCB_##FUNC]
#define LORELIB_INVOKE_GCB(THUNK, FUNC, ARGS, RET, METADATA)                                                           \
    LoreLibCtx.ExecuteCallback(THUNK, FUNC, ARGS, RET, METADATA)
#define LORELIB_LAST_GCB Lore_HRTThreadCallback
#define LORELIB_HTL_BUILD
#include "lorelib_manifest.c"



//
// Post definitions
//
static void *LoreLib_GCB_HTPs[LoreLib_GCBEnumSize] = {
#define _F(NAME, SIGNATURE) __HTP_GCB_##NAME,
    LORELIB_GCB_FOREACH(_F)
#undef _F
};
static void *LoreLib_HCB_HTPs[LoreLib_HCBEnumSize] = {
#define _F(NAME, SIGNATURE) __HTP_HCB_##NAME,
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