// SPDX-License-Identifier: MIT

#ifndef LORE_SUPPORT_GLOBAL_H
#define LORE_SUPPORT_GLOBAL_H

#ifdef _WIN32
#  define LORE_DECL_IMPORT  __declspec(dllimport)
#  define LORE_DECL_EXPORT  __declspec(dllexport)
#  define LORE_USED
#  define LORE_FORCE_INLINE __forceinline
#  define LORE_NO_INLINE    __declspec(noinline)
#else
#  define LORE_DECL_IMPORT
#  define LORE_DECL_EXPORT  __attribute__((visibility("default")))
#  define LORE_USED         __attribute((used))
#  define LORE_FORCE_INLINE inline __attribute__((always_inline))
#  define LORE_NO_INLINE    __attribute__((noinline))
#endif

#ifndef LORESUPPORT_EXPORT
#  ifdef LORESUPPORT_STATIC
#    define LORESUPPORT_EXPORT
#  else
#    ifdef LORESUPPORT_LIBRARY
#      define LORESUPPORT_EXPORT LORE_DECL_EXPORT
#    else
#      define LORESUPPORT_EXPORT LORE_DECL_IMPORT
#    endif
#  endif
#endif

#endif // LORE_SUPPORT_GLOBAL_H
