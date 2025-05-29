#include <libavcodec/avcodec.h>
#include <libavcodec/bsf.h>
#include <libavcodec/mediacodec.h>
#include <libavcodec/avdct.h>
#include <libavcodec/jni.h>
#include <libavcodec/adts_parser.h>
#include <libavcodec/dv_profile.h>
#include <libavcodec/vorbis_parser.h>
// #include <libavcodec/qsv.h>
#include <libavcodec/ac3_parser.h>
// #include <libavcodec/d3d11va.h>
#include <libavcodec/dirac.h>

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
#endif



//
// Custom(Host)
//
#if defined(LORELIB_HTL_BUILD) || defined(LORELIB_VISUAL)
#endif

#ifdef LORELIB_BUILD
#  include "lorelib_impl.c"
#endif