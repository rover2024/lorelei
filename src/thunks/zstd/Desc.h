#pragma once

#include <zstd.h>
#include <zstd_errors.h>
#include <zdict.h>

#include <lorelei/Tools/ThunkInterface/Proc.h>
#include <lorelei/Tools/ThunkInterface/PassTags.h>

namespace lore::thunk {}

#ifdef gzgetc
#  undef gzgetc
#endif
