#ifndef LORE_BASE_CORELIB_GLOBAL_H
#define LORE_BASE_CORELIB_GLOBAL_H

#ifdef _WIN32
#  define LORE_DECL_IMPORT __declspec(dllimport)
#  define LORE_DECL_EXPORT __declspec(dllexport)
#else
#  define LORE_DECL_IMPORT
#  define LORE_DECL_EXPORT __attribute__((visibility("default")))
#endif

#ifndef LORECORE_EXPORT
#  ifdef LORECORE_STATIC
#    define LORECORE_EXPORT
#  else
#    ifdef LORECORE_LIBRARY
#      define LORECORE_EXPORT LORE_DECL_EXPORT
#    else
#      define LORECORE_EXPORT LORE_DECL_IMPORT
#    endif
#  endif
#endif

#endif // LORE_BASE_CORELIB_GLOBAL_H
