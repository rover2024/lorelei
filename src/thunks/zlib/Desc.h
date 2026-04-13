#include <zlib.h>

#include <lorelei/Tools/ThunkInterface/Proc.h>
#include <lorelei/Tools/ThunkInterface/PassTags.h>

namespace lore::thunk {}

#ifdef gzgetc
#  undef gzgetc
#endif
