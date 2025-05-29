#define ZSTD_STATIC_LINKING_ONLY
#define ZDICT_STATIC_LINKING_ONLY
#include <zstd.h>
#include <zstd_errors.h>
#include <zdict.h>

#include "pool.h"
#include "huf.h"
#include "xxhash.h"

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

#  ifdef gzgetc
#    undef gzgetc
#  endif

#endif



//
// Custom(Host)
//
#if defined(LORELIB_HTL_BUILD) || defined(LORELIB_VISUAL)
#endif

#ifdef LORELIB_BUILD
#  include "lorelib_impl.c"
#endif