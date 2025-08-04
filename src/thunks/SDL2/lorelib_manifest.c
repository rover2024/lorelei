#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

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
void __HINT_SDL_LogMessageV(int arg1, SDL_LogPriority arg2, const char *arg3, va_list arg4)
    LORELIB_ANNOTATE("@vprintf:3,4");
#endif



//
// Custom(Guest)
//
#if defined(LORELIB_GTL_BUILD) || defined(LORELIB_VISUAL)
#endif



//
// Custom(Host)
//
#if defined(LORELIB_HTL_BUILD) || defined(LORELIB_VISUAL)
#endif

