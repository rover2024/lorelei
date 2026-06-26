// SPDX-License-Identifier: MIT

#include <lorelei/ThunkInterface/Detail/Callback.h>
#include <lorelei/ThunkInterface/Detail/Variadic.h>

#ifdef LORE_THUNK_BUILD
#  include <lorelei/ThunkInterface/Detail/ManifestOnBuild.cpp.inc>
#else
#  include <lorelei/ThunkInterface/Detail/ManifestPreBuild.cpp.inc>
#endif
