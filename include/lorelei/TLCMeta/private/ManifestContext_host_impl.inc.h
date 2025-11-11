namespace lorethunk::proc {

    void LocalThunkContext::preInitialize() {
#define _F(NAME)                                                                                   \
    hostFunctions_HTPs[HostFunction_##NAME].addr =                                                 \
        (void *) MetaProc<::NAME, CProcKind_HostFunction, CProcThunkPhase_HTP>::invoke;
        LORETHUNK_HOST_FUNCTION_FOREACH(_F)
#undef _F
#define _F(NAME)                                                                                   \
    guestFunctions_HTPs[GuestFunction_##NAME].addr =                                               \
        (void *) MetaProc<::NAME, CProcKind_GuestFunction, CProcThunkPhase_HTP>::invoke;
        LORETHUNK_GUEST_FUNCTION_FOREACH(_F)
#undef _F
#define _F(NAME, TYPE)                                                                             \
    hostCallbacks_HTPs[Callback_##NAME].addr =                                                     \
        (void *) MetaProcCB<NAME, CProcKind_HostCallback, CProcThunkPhase_HTP>::invoke;
        LORETHUNK_CALLBACK_FOREACH(_F)
#undef _F
#define _F(NAME, TYPE)                                                                             \
    guestCallbacks_HTPs[Callback_##NAME].addr =                                                    \
        (void *) MetaProcCB<NAME, CProcKind_GuestCallback, CProcThunkPhase_HTP>::invoke;
        LORETHUNK_CALLBACK_FOREACH(_F)
#undef _F

        staticProcInfoContext.emuAddr = (void *) server->runTaskEntry();
    }

    void LocalThunkContext::postInitialize() {
        LTC.callChecker.setEmuAddr(staticProcInfoContext.emuAddr);
    }

}

namespace lorethunk {

    static inline bool isHostAddress(void *addr) {
        return proc::LTC.callChecker.isHostAddress(addr);
    }

}

extern "C" {

LORETHUNK_EXPORT void __LORETHUNK_exchange(CStaticProcInfoContext *ctx) {
    using namespace lorethunk::proc;

    const auto &copyProcs = [](CStaticProcInfo *src, CStaticProcInfo *dest, size_t size) {
        for (size_t i = 0; i < size; ++i) {
            dest[i] = src[i];
        }
    };

    copyProcs(ctx->gtpEntries[CProcKind_HostFunction].arr, hostFunctions_GTPs, NumHostFunction);
    copyProcs(ctx->gtpEntries[CProcKind_GuestFunction].arr, guestFunctions_GTPs, NumGuestFunction);
    copyProcs(ctx->gtpEntries[CProcKind_HostCallback].arr, hostCallbacks_GTPs, NumCallback);
    copyProcs(ctx->gtpEntries[CProcKind_GuestCallback].arr, guestCallbacks_GTPs, NumCallback);

    copyProcs(hostFunctions_HTPs, ctx->htpEntries[CProcKind_HostFunction].arr, NumHostFunction);
    copyProcs(guestFunctions_HTPs, ctx->htpEntries[CProcKind_GuestFunction].arr, NumGuestFunction);
    copyProcs(hostCallbacks_HTPs, ctx->htpEntries[CProcKind_HostCallback].arr, NumCallback);
    copyProcs(guestCallbacks_HTPs, ctx->htpEntries[CProcKind_GuestCallback].arr, NumCallback);

    ctx->emuAddr = staticProcInfoContext.emuAddr;
}
}