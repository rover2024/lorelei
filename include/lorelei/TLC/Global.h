#ifndef LORELIBTLC_GLOBAL_H
#define LORELIBTLC_GLOBAL_H

#ifndef LORELIBTLC_LIBRARY
#  define LORELIBTLC_EXPORT
#else
#  define LORELIBTLC_EXPORT __attribute__((visibility("default")))
#endif

#endif // LORELIBTLC_GLOBAL_H
