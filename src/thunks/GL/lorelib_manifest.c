#include <dlfcn.h>

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

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
static void __attribute__((constructor)) LoreLib_Init2() {
    // ### FIXME: implement GLX and GLEW
    dlopen("libGLX.so", RTLD_NOW | RTLD_GLOBAL);
}
#endif



//
// Custom(Host)
//
#if defined(LORELIB_HTL_BUILD) || defined(LORELIB_VISUAL)
#endif

