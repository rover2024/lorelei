#ifndef LORELEI_GLOBAL_H
#define LORELEI_GLOBAL_H

#define LORELEI_DECL_EXPORT __attribute__((visibility("default")))

// #define LORELEI_CONSTRUCTOR __attribute__((constructor))
// #define LORELEI_DESTRUCTOR  __attribute__((destructor))

#ifdef LORELEI_LIBRARY
#  define LORELEI_EXPORT LORELEI_DECL_EXPORT
#else
#  define LORELEI_EXPORT
#endif

#endif // LORELEI_GLOBAL_H