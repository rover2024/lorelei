#include <lorelei-c/Core/ProcInfo_c.h>

namespace lorethunk::proc {

    enum HostFunction {
#define _F(NAME) HostFunction_##NAME,
        LORETHUNK_HOST_FUNCTION_FOREACH(_F)
#undef _F

            NumHostFunction,
    };

    enum GuestFunction {
#define _F(NAME) GuestFunction_##NAME,
        LORETHUNK_GUEST_FUNCTION_FOREACH(_F)
#undef _F

            NumGuestFunction,
    };

    enum Callback {
#define _F(NAME, SIGNATURE) Callback_##NAME,
        LORETHUNK_CALLBACK_FOREACH(_F)
#undef _F

            NumCallback,
    };

    template <auto T>
    constexpr int getHostFunctionIndex();

#define _F(NAME)                                                                                   \
    template <>                                                                                    \
    constexpr int getHostFunctionIndex<::NAME>() {                                                 \
        return HostFunction_##NAME;                                                                \
    }
    LORETHUNK_HOST_FUNCTION_FOREACH(_F)
#undef _F

    template <auto T>
    constexpr int getGuestFunctionIndex();

#define _F(NAME)                                                                                   \
    template <>                                                                                    \
    constexpr int getGuestFunctionIndex<::NAME>() {                                                \
        return GuestFunction_##NAME;                                                               \
    }
    LORETHUNK_GUEST_FUNCTION_FOREACH(_F)
#undef _F

    template <class T>
    constexpr int getCallbackIndex();

#define _F(NAME, TYPE)                                                                             \
    template <>                                                                                    \
    constexpr int getCallbackIndex<TYPE>() {                                                       \
        return Callback_##NAME;                                                                    \
    }
    LORETHUNK_CALLBACK_FOREACH(_F)
#undef _F

    enum LibraryFunction {
        NumLibraryFunction =

#ifdef LORETHUNK_HOST
            NumHostFunction
#else
            NumGuestFunction
#endif
    };

    // Implemented in the generated codes
    static CStaticProcInfo hostFunctions_GTPs[NumHostFunction + 1];
    static CStaticProcInfo guestFunctions_GTPs[NumGuestFunction + 1];
    static CStaticProcInfo hostCallbacks_GTPs[NumCallback + 1];
    static CStaticProcInfo guestCallbacks_GTPs[NumCallback + 1];

    // Assigned in the initializer
    static CStaticProcInfo hostFunctions_HTPs[NumHostFunction + 1];
    static CStaticProcInfo guestFunctions_HTPs[NumGuestFunction + 1];
    static CStaticProcInfo hostCallbacks_HTPs[NumCallback + 1];
    static CStaticProcInfo guestCallbacks_HTPs[NumCallback + 1];

    static CStaticProcInfo libraryFunctions[NumLibraryFunction + 1];

    static CStaticProcInfoContext staticProcInfoContext = {
        {
         {hostFunctions_GTPs, NumHostFunction},
         {guestFunctions_GTPs, NumGuestFunction},
         {hostCallbacks_GTPs, NumCallback},
         {guestCallbacks_GTPs, NumCallback},
         },
        {
         {hostFunctions_HTPs, NumHostFunction},
         {guestFunctions_HTPs, NumGuestFunction},
         {hostCallbacks_HTPs, NumCallback},
         {guestCallbacks_HTPs, NumCallback},
         },
        {libraryFunctions, NumLibraryFunction},
        nullptr,
    };

}