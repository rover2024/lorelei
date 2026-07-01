// SPDX-License-Identifier: MIT

#ifndef LORE_THUNKINTERFACE_VARIADIC_H
#define LORE_THUNKINTERFACE_VARIADIC_H

#include <lorelei/BuildConfig.h>
#include <lorelei/DLCall/Tools/VariadicAdaptor.h>

#ifdef LORE_CONFIG_THUNK_VARG_MAX
#  define LORE_THUNK_VARG_MAX LORE_CONFIG_THUNK_VARG_MAX
#else
#  define LORE_THUNK_VARG_MAX 64
#endif

#endif // LORE_THUNKINTERFACE_VARIADIC_H
