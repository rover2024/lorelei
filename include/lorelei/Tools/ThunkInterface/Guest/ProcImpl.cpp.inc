namespace lore::thunk::detail {

    void LocalThunkContext::preInitialize() {
#define _F(NAME)                                                                                   \
    hostFunctions_guestEntries[HostFunction_##NAME] = {                                                    \
        #NAME,                                                                                     \
        reinterpret_cast<void *>(ProcFn<NAME, GuestToHost, Entry>::invoke),                        \
    };
        LORE_THUNK_FUNCTION_G2H_FOREACH(_F)
#undef _F

#define _F(NAME)                                                                                   \
    guestFunctions_guestEntries[GuestFunction_##NAME] = {                                                  \
        #NAME,                                                                                     \
        reinterpret_cast<void *>(ProcFn<NAME, HostToGuest, Entry>::invoke),                        \
    };
        LORE_THUNK_FUNCTION_H2G_FOREACH(_F)
#undef _F

#define _F(NAME, TYPE)                                                                             \
    hostCallbacks_guestEntries[Callback_##NAME] = {                                                        \
        #NAME,                                                                                     \
        reinterpret_cast<void *>(ProcCb<TYPE, GuestToHost, Entry>::invoke),                        \
    };
        LORE_THUNK_CALLBACK_FOREACH(_F)
#undef _F

#define _F(NAME, TYPE)                                                                             \
    guestCallbacks_guestEntries[Callback_##NAME] = {                                                       \
        #NAME,                                                                                     \
        reinterpret_cast<void *>(ProcCb<TYPE, HostToGuest, Entry>::invoke),                        \
    };
        LORE_THUNK_CALLBACK_FOREACH(_F)
#undef _F

#define _F(NAME) hostFunctions_hostEntries[HostFunction_##NAME].name = #NAME;
        LORE_THUNK_FUNCTION_G2H_FOREACH(_F)
#undef _F

#define _F(NAME) guestFunctions_hostEntries[GuestFunction_##NAME].name = #NAME;
        LORE_THUNK_FUNCTION_H2G_FOREACH(_F)
#undef _F

#define _F(NAME, TYPE) hostCallbacks_hostEntries[Callback_##NAME].name = #NAME;
        LORE_THUNK_CALLBACK_FOREACH(_F)
#undef _F

#define _F(NAME, TYPE) guestCallbacks_hostEntries[Callback_##NAME].name = #NAME;
        LORE_THUNK_CALLBACK_FOREACH(_F)
#undef _F

#ifndef LORE_THUNK_CONFIG_GUEST_FUNCTION_STATIC_LINK
#  define _F(NAME) libraryFunctions[GuestFunction_##NAME].name = #NAME;
        LORE_THUNK_FUNCTION_H2G_FOREACH(_F)
#  undef _F
#else
#  define _F(NAME)                                                                                 \
      libraryFunctions[GuestFunction_##NAME] = {                                                   \
          #NAME,                                                                                   \
          reinterpret_cast<void *>(NAME),                                                          \
      };
        LORE_THUNK_FUNCTION_H2G_FOREACH(_F)
#  undef _F
#endif
    }

    void LocalThunkContext::postInitialize() {
    }

}

