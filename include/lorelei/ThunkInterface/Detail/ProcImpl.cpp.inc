// SPDX-License-Identifier: MIT

namespace lore::thunk {

    // Declared in "Callback.h"
    inline bool isHostAddress(void *addr) {
#ifdef LORE_QEMU_SUPPORT_ADDRESS_SEPARATION
        // Guest and host addresses live in disjoint ranges split at emuAddr.
        return reinterpret_cast<uintptr_t>(addr) >
               reinterpret_cast<uintptr_t>(detail::staticThunkContext.emuAddr);
#elif defined(LORE_THUNK_HOST)
        return mod::HostServer::isHostAddressNaive(addr);
#else
        return mod::GuestClient::isHostAddressNaive(addr);
#endif
    }

}

#include "ProcInit.cpp.inc"

#ifdef LORE_THUNK_HOST

extern "C" {
LORE_DECL_EXPORT void LoreExchangeContext(lore::thunk::StaticThunkContext *guestCtx) {
    using namespace lore::thunk;
    using namespace lore::thunk::detail;

    const auto copyProcs = [](ProcArrayRef src, ProcArrayRef dest) {
        const size_t n = src.size < dest.size ? src.size : dest.size;
        for (size_t i = 0; i < n; ++i) {
            dest.arr[i] = src.arr[i];
        }
    };

    for (int kind = Function; kind < NumProcKind; ++kind) {
        for (int dir = GuestToHost; dir < NumProcDirection; ++dir) {
            copyProcs(guestCtx->guestProcs[kind][dir], staticThunkContext.guestProcs[kind][dir]);
            copyProcs(staticThunkContext.hostProcs[kind][dir], guestCtx->hostProcs[kind][dir]);
        }
    }

    guestCtx->emuAddr = staticThunkContext.emuAddr;
}
}

#endif // LORE_THUNK_HOST
