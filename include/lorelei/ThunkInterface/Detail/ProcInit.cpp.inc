// Side aliases: "THIS" is the local side (its entry tables receive real invoke addresses),
// "PEER" is the opposite side (only names now; addresses filled later by LoreExchangeContext).
#ifdef LORE_THUNK_HOST
#  define _LORE_THUNK_THIS_ENTRY(x) x##_hostEntries
#  define _LORE_THUNK_PEER_ENTRY(x) x##_guestEntries
#else
#  define _LORE_THUNK_THIS_ENTRY(x) x##_guestEntries
#  define _LORE_THUNK_PEER_ENTRY(x) x##_hostEntries
#endif

namespace lore::thunk::detail {

    void LocalThunkContext::preInitialize() {
        // Local entry tables: name + this side's invoke address.
#define _F(NAME)                                                                                   \
    _LORE_THUNK_THIS_ENTRY(hostFunctions)[HostFunction_##NAME] = {                                  \
        #NAME,                                                                                     \
        reinterpret_cast<void *>(ProcFn<NAME, GuestToHost, Entry>::invoke),                        \
    };
        LORE_THUNK_FUNCTION_G2H_FOREACH(_F)
#undef _F

#define _F(NAME)                                                                                   \
    _LORE_THUNK_THIS_ENTRY(guestFunctions)[GuestFunction_##NAME] = {                                \
        #NAME,                                                                                     \
        reinterpret_cast<void *>(ProcFn<NAME, HostToGuest, Entry>::invoke),                        \
    };
        LORE_THUNK_FUNCTION_H2G_FOREACH(_F)
#undef _F

#define _F(NAME, SIGNATURE)                                                                        \
    _LORE_THUNK_THIS_ENTRY(hostCallbacks)[Callback_##NAME] = {                                      \
        SIGNATURE,                                                                                 \
        reinterpret_cast<void *>(ProcCb<NAME, GuestToHost, Entry>::invoke),                        \
    };
        LORE_THUNK_CALLBACK_FOREACH(_F)
#undef _F

#define _F(NAME, SIGNATURE)                                                                        \
    _LORE_THUNK_THIS_ENTRY(guestCallbacks)[Callback_##NAME] = {                                     \
        SIGNATURE,                                                                                 \
        reinterpret_cast<void *>(ProcCb<NAME, HostToGuest, Entry>::invoke),                        \
    };
        LORE_THUNK_CALLBACK_FOREACH(_F)
#undef _F

        // Peer entry tables: name only (addresses arrive during context exchange).
#define _F(NAME) _LORE_THUNK_PEER_ENTRY(hostFunctions)[HostFunction_##NAME].key = #NAME;
        LORE_THUNK_FUNCTION_G2H_FOREACH(_F)
#undef _F

#define _F(NAME) _LORE_THUNK_PEER_ENTRY(guestFunctions)[GuestFunction_##NAME].key = #NAME;
        LORE_THUNK_FUNCTION_H2G_FOREACH(_F)
#undef _F

#define _F(NAME, SIGNATURE) _LORE_THUNK_PEER_ENTRY(hostCallbacks)[Callback_##NAME].key = SIGNATURE;
        LORE_THUNK_CALLBACK_FOREACH(_F)
#undef _F

#define _F(NAME, SIGNATURE) _LORE_THUNK_PEER_ENTRY(guestCallbacks)[Callback_##NAME].key = SIGNATURE;
        LORE_THUNK_CALLBACK_FOREACH(_F)
#undef _F

        // This side's real library symbols (static-link folds the address in directly).
#ifdef LORE_THUNK_HOST
#  ifndef LORE_THUNK_CONFIG_HOST_FUNCTION_STATIC_LINK
#    define _F(NAME) libraryFunctions[HostFunction_##NAME].key = #NAME;
        LORE_THUNK_FUNCTION_G2H_FOREACH(_F)
#    undef _F
#  else
#    define _F(NAME)                                                                               \
        libraryFunctions[HostFunction_##NAME] = {                                                  \
            #NAME,                                                                                 \
            reinterpret_cast<void *>(NAME),                                                        \
        };
        LORE_THUNK_FUNCTION_G2H_FOREACH(_F)
#    undef _F
#  endif
#else
#  ifndef LORE_THUNK_CONFIG_GUEST_FUNCTION_STATIC_LINK
#    define _F(NAME) libraryFunctions[GuestFunction_##NAME].key = #NAME;
        LORE_THUNK_FUNCTION_H2G_FOREACH(_F)
#    undef _F
#  else
#    define _F(NAME)                                                                               \
        libraryFunctions[GuestFunction_##NAME] = {                                                 \
            #NAME,                                                                                 \
            reinterpret_cast<void *>(NAME),                                                        \
        };
        LORE_THUNK_FUNCTION_H2G_FOREACH(_F)
#    undef _F
#  endif
#endif
    }

    void LocalThunkContext::postInitialize() {
    }

}

#undef _LORE_THUNK_THIS_ENTRY
#undef _LORE_THUNK_PEER_ENTRY
