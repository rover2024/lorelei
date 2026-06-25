#include <lorelei/DLCall/ProcDefs.h>

#ifndef LORE_THUNK_FUNCTION_G2H_FOREACH
#  define LORE_THUNK_FUNCTION_G2H_FOREACH(F)
#endif

#ifndef LORE_THUNK_FUNCTION_H2G_FOREACH
#  define LORE_THUNK_FUNCTION_H2G_FOREACH(F)
#endif

#ifndef LORE_THUNK_CALLBACK_FOREACH
#  define LORE_THUNK_CALLBACK_FOREACH(F)
#endif

namespace lore::thunk::detail {

    enum HostFunction {
#define _F(NAME) HostFunction_##NAME,
        LORE_THUNK_FUNCTION_G2H_FOREACH(_F)
#undef _F

            NumHostFunction,
    };

    enum GuestFunction {
#define _F(NAME) GuestFunction_##NAME,
        LORE_THUNK_FUNCTION_H2G_FOREACH(_F)
#undef _F

            NumGuestFunction,
    };

    enum Callback {
#define _F(NAME, SIGNATURE) Callback_##NAME,
        LORE_THUNK_CALLBACK_FOREACH(_F)
#undef _F

            NumCallback,
    };

    template <auto T>
    constexpr int getHostFunctionIndex();

#define _F(NAME)                                                                                   \
    template <>                                                                                    \
    constexpr int getHostFunctionIndex<NAME>() {                                                   \
        return HostFunction_##NAME;                                                                \
    }
    LORE_THUNK_FUNCTION_G2H_FOREACH(_F)
#undef _F

    template <auto T>
    constexpr int getGuestFunctionIndex();

#define _F(NAME)                                                                                   \
    template <>                                                                                    \
    constexpr int getGuestFunctionIndex<NAME>() {                                                  \
        return GuestFunction_##NAME;                                                               \
    }
    LORE_THUNK_FUNCTION_H2G_FOREACH(_F)
#undef _F

    template <class T>
    constexpr int getCallbackIndex();

#define _F(NAME, SIGNATURE)                                                                        \
    template <>                                                                                    \
    constexpr int getCallbackIndex<NAME>() {                                                       \
        return Callback_##NAME;                                                                    \
    }
    LORE_THUNK_CALLBACK_FOREACH(_F)
#undef _F

    enum LibraryFunction {
        NumLibraryFunction =

#ifdef LORE_THUNK_HOST
            NumHostFunction
#else
            NumGuestFunction
#endif
    };

    // Implemented in the generated codes
    static ProcInfoPair hostFunctions_guestEntries[NumHostFunction + 1];
    static ProcInfoPair guestFunctions_guestEntries[NumGuestFunction + 1];
    static ProcInfoPair hostCallbacks_guestEntries[NumCallback + 1];
    static ProcInfoPair guestCallbacks_guestEntries[NumCallback + 1];

    // Assigned in the initializer
    static ProcInfoPair hostFunctions_hostEntries[NumHostFunction + 1];
    static ProcInfoPair guestFunctions_hostEntries[NumGuestFunction + 1];
    static ProcInfoPair hostCallbacks_hostEntries[NumCallback + 1];
    static ProcInfoPair guestCallbacks_hostEntries[NumCallback + 1];

    static ProcInfoPair libraryFunctions[NumLibraryFunction + 1];

    static StaticThunkContext staticThunkContext = {
        {
         {{hostFunctions_guestEntries, NumHostFunction},
             {guestFunctions_guestEntries, NumGuestFunction}},
         {{hostCallbacks_guestEntries, NumCallback}, {guestCallbacks_guestEntries, NumCallback}},
         },
        {
         {{hostFunctions_hostEntries, NumHostFunction},
             {guestFunctions_hostEntries, NumGuestFunction}},
         {{hostCallbacks_hostEntries, NumCallback}, {guestCallbacks_hostEntries, NumCallback}},
         },
        {libraryFunctions, NumLibraryFunction},
        nullptr,
#ifdef LORE_THUNK_HOST
#  ifdef LORE_THUNK_AUTO_LINK
        true,
#  else
        false,
#  endif
#else
        false,
#endif
    };

}
