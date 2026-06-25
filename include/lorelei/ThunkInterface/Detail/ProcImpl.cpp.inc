namespace lore::thunk {

    // Declared in "Callback.h"
    inline bool isHostAddress(void *addr) {
        return reinterpret_cast<uintptr_t>(addr) >
               reinterpret_cast<uintptr_t>(detail::staticThunkContext.emuAddr);
    }

}

#include "ProcInit.cpp.inc"

#ifdef LORE_THUNK_HOST

extern "C" {
LORE_DECL_EXPORT void LoreExchangeContext(lore::StaticThunkContext *guestCtx) {
    using namespace lore::thunk::detail;

    const auto copyProcs = [](lore::ProcArrayRef src, lore::ProcArrayRef dest) {
        const size_t n = src.size < dest.size ? src.size : dest.size;
        for (size_t i = 0; i < n; ++i) {
            dest.arr[i] = src.arr[i];
        }
    };

    for (int kind = lore::Function; kind < lore::NumProcKind; ++kind) {
        for (int dir = lore::GuestToHost; dir < lore::NumProcDirection; ++dir) {
            copyProcs(guestCtx->guestProcs[kind][dir], staticThunkContext.guestProcs[kind][dir]);
            copyProcs(staticThunkContext.hostProcs[kind][dir], guestCtx->hostProcs[kind][dir]);
        }
    }

    guestCtx->emuAddr = staticThunkContext.emuAddr;
}
}

#endif // LORE_THUNK_HOST
