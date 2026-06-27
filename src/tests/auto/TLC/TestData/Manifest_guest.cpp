// le_qsort / le_bsearch take a comparator the host calls back into the guest, so callback
// substitution must be on.
#define LORE_THUNK_CALLBACK_REPLACE

#include "Desc.h"
#include <lorelei/ThunkInterface/ManifestGuest.cpp.inc>

namespace lore::thunk {}
