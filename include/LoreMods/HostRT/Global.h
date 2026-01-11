#ifndef LORE_MODULES_HOSTRT_GLOBAL_H
#define LORE_MODULES_HOSTRT_GLOBAL_H

#include <LoreBase/CoreLib/Global.h>

#ifndef LOREHOSTRT_EXPORT
#  ifdef LOREHOSTRT_STATIC
#    define LOREHOSTRT_EXPORT
#  else
#    ifdef LOREHOSTRT_LIBRARY
#      define LOREHOSTRT_EXPORT LORE_DECL_EXPORT
#    else
#      define LOREHOSTRT_EXPORT LORE_DECL_IMPORT
#    endif
#  endif
#endif

#endif // LORE_MODULES_HOSTRT_GLOBAL_H
