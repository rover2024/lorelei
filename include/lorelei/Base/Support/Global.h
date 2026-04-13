#ifndef LORE_BASE_CORELIB_GLOBAL_H
#define LORE_BASE_CORELIB_GLOBAL_H

#ifdef _WIN32
#  define LORE_DECL_IMPORT __declspec(dllimport)
#  define LORE_DECL_EXPORT __declspec(dllexport)
#  define LORE_USED
#else
#  define LORE_DECL_IMPORT
#  define LORE_DECL_EXPORT __attribute__((visibility("default")))
#  define LORE_USED        __attribute((used))
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

#endif // LORE_BASE_CORELIB_GLOBAL_H
