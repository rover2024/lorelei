#ifndef LORECORE_GLOBAL_H
#define LORECORE_GLOBAL_H

#define LORECORE_DECL_EXPORT __attribute__((visibility("default")))

#ifndef LORECORE_LIBRARY
#  define LORECORE_EXPORT
#else
#  define LORECORE_EXPORT LORECORE_DECL_EXPORT
#endif

#endif // LORECORE_GLOBAL_H
