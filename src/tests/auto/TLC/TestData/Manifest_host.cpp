// le_qsort / le_bsearch take a comparator the host calls back into the guest, so callback
// substitution must be on. AUTO_LINK folds the real le_* addresses (from the example host
// implementation) into the HTL instead of resolving them at run time.
#define LORE_THUNK_CALLBACK_REPLACE
#define LORE_THUNK_AUTO_LINK

#include "Desc.h"
#include <lorelei/ThunkInterface/ManifestHost.cpp.inc>

#include "LongDoubleConvert.h"

namespace lore::thunk {

    // le_mix takes and returns long double, which the guest passes as the x86 80-bit extended
    // representation. These type filters convert it to and from the host's native long double. The
    // TypeFilter pass discovers them and injects them into the Adapt layer of every proc that has a
    // long double argument or return. A plain pass-through manifest cannot express this, because the
    // conversion has to happen after Entry has unpacked the raw bytes but before the real call.
    template <>
    struct ProcArgFilter<long double> {
        template <class Desc, size_t Index, ProcKind Kind, ProcDirection Direction, class... Args>
        static void filter(long double &arg, ProcArgContext<Args...> ctx) {
            (void) ctx;
            if constexpr (Direction == GuestToHost) {
                // arg currently aliases the guest's 80-bit bytes in the shared address space.
                arg = example::f80ToHost(&arg);
            }
        }
    };

    template <>
    struct ProcReturnFilter<long double> {
        template <class Desc, ProcKind Kind, ProcDirection Direction, class... Args>
        static void filter(long double &ret, ProcArgContext<Args...> ctx) {
            (void) ctx;
            if constexpr (Direction == GuestToHost) {
                // Re-encode the host result as 80-bit for the guest to read back.
                example::hostToF80(ret, &ret);
            }
        }
    };

}
