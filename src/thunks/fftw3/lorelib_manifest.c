#include <fftw3.h>
#include <api/api.h>

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
const char fftw_version[] = "fftw-3.3.9";
const char fftw_cc[] = "/usr/bin/gcc";
const char fftw_codelet_optim[] = "";
#endif



//
// Custom(Host)
//
#if defined(LORELIB_HTL_BUILD) || defined(LORELIB_VISUAL)
#endif

#ifdef LORELIB_BUILD
#  include "lorelib_impl.c"
#endif