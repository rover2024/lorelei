// SPDX-License-Identifier: MIT

#ifndef LORE_TOOLS_BASIC_GLOBAL_H
#define LORE_TOOLS_BASIC_GLOBAL_H

#include <LoreBase/CoreLib/Global.h>

#ifndef LORETOOLBASIC_EXPORT
#  ifdef LORETOOLBASIC_STATIC
#    define LORETOOLBASIC_EXPORT
#  else
#    ifdef LORETOOLBASIC_LIBRARY
#      define LORETOOLBASIC_EXPORT LORE_DECL_EXPORT
#    else
#      define LORETOOLBASIC_EXPORT LORE_DECL_IMPORT
#    endif
#  endif
#endif

#endif // LORE_TOOLS_BASIC_GLOBAL_H
