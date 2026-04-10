#include <lorelei/Base/PassThrough/c/CProc.h>

#ifndef LORE_THUNK_HOST_FUNCTION_FOREACH
#  define LORE_THUNK_HOST_FUNCTION_FOREACH(F)
#endif

#ifndef LORE_THUNK_GUEST_FUNCTION_FOREACH
#  define LORE_THUNK_GUEST_FUNCTION_FOREACH(F)
#endif

#ifndef LORE_THUNK_CALLBACK_FOREACH
#  define LORE_THUNK_CALLBACK_FOREACH(F)
#endif

namespace lore::thunk::detail {

    enum HostFunction {
#define _F(NAME) HostFunction_##NAME,
        LORE_THUNK_HOST_FUNCTION_FOREACH(_F)
#undef _F

            NumHostFunction,
    };

    enum GuestFunction {
#define _F(NAME) GuestFunction_##NAME,
        LORE_THUNK_GUEST_FUNCTION_FOREACH(_F)
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
    LORE_THUNK_HOST_FUNCTION_FOREACH(_F)
#undef _F

    template <auto T>
    constexpr int getGuestFunctionIndex();

#define _F(NAME)                                                                                   \
    template <>                                                                                    \
    constexpr int getGuestFunctionIndex<NAME>() {                                                  \
        return GuestFunction_##NAME;                                                               \
    }
    LORE_THUNK_GUEST_FUNCTION_FOREACH(_F)
#undef _F

    template <class T>
    constexpr int getCallbackIndex();

#define _F(NAME, TYPE)                                                                             \
    template <>                                                                                    \
    constexpr int getCallbackIndex<TYPE>() {                                                       \
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
    static CStaticProcInfo hostFunctions_guestEntries[NumHostFunction + 1];
    static CStaticProcInfo guestFunctions_guestEntries[NumGuestFunction + 1];
    static CStaticProcInfo hostCallbacks_guestEntries[NumCallback + 1];
    static CStaticProcInfo guestCallbacks_guestEntries[NumCallback + 1];

    // Assigned in the initializer
    static CStaticProcInfo hostFunctions_hostEntries[NumHostFunction + 1];
    static CStaticProcInfo guestFunctions_hostEntries[NumGuestFunction + 1];
    static CStaticProcInfo hostCallbacks_hostEntries[NumCallback + 1];
    static CStaticProcInfo guestCallbacks_hostEntries[NumCallback + 1];

    static CStaticProcInfo libraryFunctions[NumLibraryFunction + 1];

    static CStaticThunkContext staticThunkContext = {
        {
         {{hostFunctions_guestEntries, NumHostFunction}, {guestFunctions_guestEntries, NumGuestFunction}},
         {{hostCallbacks_guestEntries, NumCallback}, {guestCallbacks_guestEntries, NumCallback}},
         },
        {
         {{hostFunctions_hostEntries, NumHostFunction}, {guestFunctions_hostEntries, NumGuestFunction}},
         {{hostCallbacks_hostEntries, NumCallback}, {guestCallbacks_hostEntries, NumCallback}},
         },
        {libraryFunctions, NumLibraryFunction},
        nullptr,
    };

}
