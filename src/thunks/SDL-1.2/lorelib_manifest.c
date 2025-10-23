#include <SDL/SDL.h>
#include <dlfcn.h>

#include <lorelei/loreshared.h>
#include <lorelei/loreuser.h>

#include <lorelib_common/manifest-predef.h>



//
// Option macros
//
#define LORELIB_GCB_AUTO_DEPTH 1
#define LORELIB_HCB_AUTO_DEPTH 1



//
// Annotations
//
#ifdef LORELIB_VISUAL
#endif



//
// Custom(Guest)
//
#if defined(LORELIB_GTL_BUILD) || defined(LORELIB_VISUAL)

static void * __GTP_SDL_LoadObject(const char *arg1) { return dlopen(arg1, RTLD_NOW); }

static void __GTP_SDL_UnloadObject(void *arg1) { dlclose(arg1); }

static void * __GTP_SDL_LoadFunction(void * arg1, const char * arg2) { return dlsym(arg1, arg2); }

static char * __GTP_SDL_GetError() { return dlerror(); }

#endif



//
// Custom(Host)
//
#if defined(LORELIB_HTL_BUILD) || defined(LORELIB_VISUAL)
#endif

