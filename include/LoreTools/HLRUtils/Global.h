// SPDX-License-Identifier: MIT

#ifndef LORE_TOOLS_HLRUTILS_GLOBAL_H
#define LORE_TOOLS_HLRUTILS_GLOBAL_H

#include <LoreBase/CoreLib/Global.h>

#ifndef LOREHLRUTILS_EXPORT
#  ifdef LOREHLRUTILS_STATIC
#    define LOREHLRUTILS_EXPORT
#  else
#    ifdef LOREHLRUTILS_LIBRARY
#      define LOREHLRUTILS_EXPORT LORE_DECL_EXPORT
#    else
#      define LOREHLRUTILS_EXPORT LORE_DECL_IMPORT
#    endif
#  endif
#endif

#endif // LORE_TOOLS_HLRUTILS_GLOBAL_H
