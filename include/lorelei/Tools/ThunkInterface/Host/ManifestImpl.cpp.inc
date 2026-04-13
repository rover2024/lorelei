namespace lore::thunk {

    // Declared in "Callback.h"
    inline bool isHostAddress(void *addr) {
        return reinterpret_cast<uintptr_t>(addr) >
               reinterpret_cast<uintptr_t>(detail::staticThunkContext.emuAddr);
    }

}

#include "ProcImpl.cpp.inc"

extern "C" {
LORE_DECL_EXPORT void __Lore_ExchangeContext(CStaticThunkContext *guestCtx) {
    using namespace lore::thunk::detail;

    const auto copyProcs = [](CStaticProcInfoArrayRef src, CStaticProcInfoArrayRef dest) {
        const size_t n = src.size < dest.size ? src.size : dest.size;
        for (size_t i = 0; i < n; ++i) {
            dest.arr[i] = src.arr[i];
        }
    };

    for (int kind = CProcKind_Function; kind < CProcKind_NumProcKind; ++kind) {
        for (int dir = CProcDirection_GuestToHost; dir < CProcDirection_NumProcDirection; ++dir) {
            copyProcs(guestCtx->guestProcs[kind][dir], staticThunkContext.guestProcs[kind][dir]);
            copyProcs(staticThunkContext.hostProcs[kind][dir], guestCtx->hostProcs[kind][dir]);
        }
    }

    guestCtx->emuAddr = staticThunkContext.emuAddr;

    guestCtx->numCCGs = staticThunkContext.numCCGs;
    guestCtx->CCGs = staticThunkContext.CCGs;
    guestCtx->numFDGs = staticThunkContext.numFDGs;
    guestCtx->FDGs = staticThunkContext.FDGs;
}
}
