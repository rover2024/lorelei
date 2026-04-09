#ifndef LORE_TOOLS_TLCAPI_GLOBAL_H
#define LORE_TOOLS_TLCAPI_GLOBAL_H

#include <lorelei/Base/Support/Global.h>

#ifndef LORETLCAPI_EXPORT
#  ifdef LORETLCAPI_STATIC
#    define LORETLCAPI_EXPORT
#  else
#    ifdef LORETLCAPI_LIBRARY
#      define LORETLCAPI_EXPORT LORE_DECL_EXPORT
#    else
#      define LORETLCAPI_EXPORT LORE_DECL_IMPORT
#    endif
#  endif
#endif

#endif // LORE_TOOLS_TLCAPI_GLOBAL_H
