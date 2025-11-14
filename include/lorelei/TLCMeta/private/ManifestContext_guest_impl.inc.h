namespace lorethunk::proc {

    void LocalThunkContext::preInitialize() {
#define _F(NAME)                                                                                   \
    hostFunctions_GTPs[HostFunction_##NAME] = {                                                    \
        #NAME,                                                                                     \
        (void *) MetaProc<::NAME, CProcKind_HostFunction, CProcThunkPhase_GTP>::invoke,            \
    };
        LORETHUNK_HOST_FUNCTION_FOREACH(_F)
#undef _F
#define _F(NAME)                                                                                   \
    guestFunctions_GTPs[GuestFunction_##NAME] = {                                                  \
        #NAME,                                                                                     \
        (void *) MetaProc<::NAME, CProcKind_GuestFunction, CProcThunkPhase_GTP>::invoke,           \
    };
        LORETHUNK_GUEST_FUNCTION_FOREACH(_F)
#undef _F
#define _F(NAME, TYPE)                                                                             \
    hostCallbacks_GTPs[Callback_##NAME] = {                                                        \
        #NAME,                                                                                     \
        (void *) MetaProcCB<NAME, CProcKind_HostCallback, CProcThunkPhase_GTP>::invoke,            \
    };
        LORETHUNK_CALLBACK_FOREACH(_F)
#undef _F
#define _F(NAME, TYPE)                                                                             \
    guestCallbacks_GTPs[Callback_##NAME] = {                                                       \
        #NAME,                                                                                     \
        (void *) MetaProcCB<NAME, CProcKind_GuestCallback, CProcThunkPhase_GTP>::invoke,           \
    };
        LORETHUNK_CALLBACK_FOREACH(_F)
#undef _F

#define _F(NAME) hostFunctions_HTPs[HostFunction_##NAME].name = #NAME;
        LORETHUNK_HOST_FUNCTION_FOREACH(_F)
#undef _F
#define _F(NAME) guestFunctions_HTPs[GuestFunction_##NAME].name = #NAME;
        LORETHUNK_GUEST_FUNCTION_FOREACH(_F)
#undef _F
#define _F(NAME, TYPE) hostCallbacks_HTPs[Callback_##NAME].name = #NAME;
        LORETHUNK_CALLBACK_FOREACH(_F)
#undef _F
#define _F(NAME, TYPE) guestCallbacks_HTPs[Callback_##NAME].name = #NAME;
        LORETHUNK_CALLBACK_FOREACH(_F)
#undef _F

#ifndef LORETHUNK_CONFIG_GUEST_FUNCTION_STATIC_LINK
#  define _F(NAME) libraryFunctions[GuestFunction_##NAME].name = #NAME;
        LORETHUNK_GUEST_FUNCTION_FOREACH(_F)
#  undef _F
#else
#  define _F(NAME)                                                                                 \
      libraryFunctions[GuestFunction_##NAME] = {                                                   \
          #NAME,                                                                                   \
          (void *) ::NAME,                                                                         \
      };
        LORETHUNK_GUEST_FUNCTION_FOREACH(_F)
#  undef _F
#endif
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