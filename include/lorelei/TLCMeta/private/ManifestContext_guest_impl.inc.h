
namespace lorethunk::proc {

    void LocalThunkContext::initialize() {
#define _F(NAME)                                                                                   \
    hostFunctions_GTPs[HostFunction_##NAME].addr =                                                 \
        (void *) MetaProc<::NAME, CProcKind_HostFunction, CProcThunkPhase_GTP>::invoke;
        LORETHUNK_HOST_FUNCTION_FOREACH(_F)
#undef _F
#define _F(NAME)                                                                                   \
    guestFunctions_GTPs[GuestFunction_##NAME].addr =                                               \
        (void *) MetaProc<::NAME, CProcKind_GuestFunction, CProcThunkPhase_GTP>::invoke;
        LORETHUNK_GUEST_FUNCTION_FOREACH(_F)
#undef _F
#define _F(NAME, TYPE)                                                                             \
    hostCallbacks_GTPs[Callback_##NAME].addr =                                                     \
        (void *) MetaProcCB<NAME, CProcKind_HostCallback, CProcThunkPhase_GTP>::invoke;
        LORETHUNK_CALLBACK_FOREACH(_F)
#undef _F
#define _F(NAME, TYPE)                                                                             \
    guestCallbacks_GTPs[Callback_##NAME].addr =                                                    \
        (void *) MetaProcCB<NAME, CProcKind_GuestCallback, CProcThunkPhase_GTP>::invoke;
        LORETHUNK_CALLBACK_FOREACH(_F)
#undef _F
    }

}