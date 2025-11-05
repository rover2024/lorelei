#ifndef LOREUTILS_GLOBAL_H
#define LOREUTILS_GLOBAL_H

#define LOREUTILS_DECL_EXPORT __attribute__((visibility("default")))

#ifndef LOREUTILS_LIBRARY
#  define LOREUTILS_EXPORT
#else
#  define LOREUTILS_EXPORT LOREUTILS_DECL_EXPORT
#endif

#endif // LORECORE_GLOBAL_H
