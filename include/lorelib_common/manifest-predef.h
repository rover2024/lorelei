#ifndef LORELIB_COMMON_MANIFEST_PREDEF_H
#define LORELIB_COMMON_MANIFEST_PREDEF_H

#include <stddef.h>

#ifndef LORELIB_EXPORT
#  define LORELIB_EXPORT
#endif

#define LORELIB_POINTER_LVALUE (*((void **) 114514))

//
// Helper macros
//
#if !defined(LORELIB_GTL_BUILD) && !defined(LORELIB_HTL_BUILD)
#  define LORELIB_VISUAL
#  define LORELIB_ANNOTATE(X) __attribute__((annotate(X)))
#else
#  define LORELIB_BUILD
#  define LORELIB_ANNOTATE(X)
#endif



//
// Placeholder macros
//

#if defined(LORELIB_VISUAL)
// guest
#  define LORELIB_HTP(FUNC) NULL
#  define LORELIB_INVOKE_HTP(FUNC, ARGS, RET, METADATA)
#  define LORELIB_HCB(FUNC) NULL
#  define LORELIB_INVOKE_HCB(THUNK, FUNC, ARGS, RET, METADATA)
#  define LORELIB_LAST_HCB LORELIB_POINTER_LVALUE
// host
#  define LORELIB_API(FUNC) ((__typeof__(&FUNC)) NULL)
#  define LORELIB_GCB(FUNC) NULL
#  define LORELIB_INVOKE_GCB(THUNK, FUNC, ARGS, RET, METADATA)
#  define LORELIB_LAST_GCB LORELIB_POINTER_LVALUE
#endif

#endif // LORELIB_COMMON_MANIFEST_PREDEF_H