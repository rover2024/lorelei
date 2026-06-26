// SPDX-License-Identifier: MIT

#ifndef LORE_DLCALL_GLOBAL_H
#define LORE_DLCALL_GLOBAL_H

#include <lorelei/Support/Global.h>

#ifndef LOREDLCALL_EXPORT
#  ifdef LOREDLCALL_STATIC
#    define LOREDLCALL_EXPORT
#  else
#    ifdef LOREDLCALL_LIBRARY
#      define LOREDLCALL_EXPORT LORE_DECL_EXPORT
#    else
#      define LOREDLCALL_EXPORT LORE_DECL_IMPORT
#    endif
#  endif
#endif

#endif // LORE_DLCALL_GLOBAL_H
