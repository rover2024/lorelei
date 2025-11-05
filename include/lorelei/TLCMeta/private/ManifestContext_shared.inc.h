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
    static inline HostFunction getHostFunctionIndex();

#define _F(NAME)                                                                                                       \
    template <>                                                                                                        \
    static inline HostFunction getHostFunctionIndex<NAME>() {                                                          \
        return HostFunction_##NAME;                                                                                    \
    }
    LORETHUNK_HOST_FUNCTION_FOREACH(_F)
#undef _F

    template <auto T>
    static inline GuestFunction getGuestFunctionIndex();

#define _F(NAME)                                                                                                       \
    template <>                                                                                                        \
    static inline GuestFunction getGuestFunctionIndex<NAME>() {                                                        \
        return GuestFunction_##NAME;                                                                                   \
    }
    LORETHUNK_GUEST_FUNCTION_FOREACH(_F)
#undef _F

    template <class T>
    static inline Callback getCallbackIndex();

#define _F(NAME, SIGNATURE)                                                                                            \
    template <>                                                                                                        \
    static inline Callback getCallbackIndex<SIGNATURE>() {                                                             \
        return Callback_##NAME;                                                                                        \
    }
    LORETHUNK_CALLBACK_FOREACH(_F)
#undef _F

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

    enum LibraryFunction {
        NumLibraryFunction =

#ifdef LORETHUNK_HOST
            NumHostFunction
#else
            NumGuestFunction
#endif
    };

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

    static inline void copyProcs(CStaticProcInfo *x, CStaticProcInfo *y, size_t size) {
#ifdef LORETHUNK_HOST
        for (size_t i = 0; i < size; ++i) {
            y[i] = x[i];
        }
#else
        for (size_t i = 0; i < size; ++i) {
            x[i] = y[i];
        }
#endif
    }

}

extern "C" {
#ifdef LORETHUNK_HOST

LORETHUNK_EXPORT void __LORETHUNK_initialize(CStaticProcInfoContext *ctx) {
    using namespace lorethunk::proc;

    copyProcs(ctx->gtpEntries[CProcKind_HostFunction].arr, staticProcInfoContext.gtpEntries[CProcKind_HostFunction].arr,
              NumHostFunction);
    copyProcs(ctx->gtpEntries[CProcKind_GuestFunction].arr,
              staticProcInfoContext.gtpEntries[CProcKind_GuestFunction].arr, NumGuestFunction);
    copyProcs(ctx->gtpEntries[CProcKind_HostCallback].arr, staticProcInfoContext.gtpEntries[CProcKind_HostCallback].arr,
              NumCallback);
    copyProcs(ctx->gtpEntries[CProcKind_GuestCallback].arr,
              staticProcInfoContext.gtpEntries[CProcKind_GuestCallback].arr, NumCallback);

    copyProcs(staticProcInfoContext.htpEntries[CProcKind_HostFunction].arr, ctx->htpEntries[CProcKind_HostFunction].arr,
              NumHostFunction);
    copyProcs(staticProcInfoContext.htpEntries[CProcKind_GuestFunction].arr,
              ctx->htpEntries[CProcKind_GuestFunction].arr, NumGuestFunction);
    copyProcs(staticProcInfoContext.htpEntries[CProcKind_HostCallback].arr, ctx->htpEntries[CProcKind_HostCallback].arr,
              NumCallback);
    copyProcs(staticProcInfoContext.htpEntries[CProcKind_GuestCallback].arr,
              ctx->htpEntries[CProcKind_GuestCallback].arr, NumCallback);
}

#endif
}