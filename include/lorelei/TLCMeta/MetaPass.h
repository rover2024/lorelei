#ifndef METAPASS_H
#define METAPASS_H

namespace lorethunk {

    struct MetaPass {
        enum BulitinID {
            // Builder
            DefaultBuilder = 0,
            Printf,
            VPrintf,
            Scanf,
            VScanf,

            // Guard
            CallbackSubstituter,
            TypeFilter,

            // Misc
            GetProcAddress,

            UserID = 0x1000,
        };
    };

    template <class... Args>
    struct MetaPassList {};

    // Builder
    template <int FormatIndex = -1, int VariadicIndex = -1>
    struct MetaPass_printf : public MetaPass {
        static constexpr const BulitinID ID = Printf;
        static constexpr const bool isBuilder = true;

        static_assert((FormatIndex == -1 && VariadicIndex == -1) || (VariadicIndex > FormatIndex),
                      "VariadicIndex must be greater than FormatIndex");
    };

    // Builder
    template <int FormatIndex = -1, int VariadicIndex = -1>
    struct MetaPass_vprintf : public MetaPass {
        static constexpr const BulitinID ID = VPrintf;
        static constexpr const bool isBuilder = true;

        static_assert((FormatIndex == -1 && VariadicIndex == -1) || (VariadicIndex > FormatIndex),
                      "VariadicIndex must be greater than FormatIndex");
    };

    template <int FormatIndex = -1, int VariadicIndex = -1>
    struct MetaPass_scanf : public MetaPass {
        static constexpr const BulitinID ID = Scanf;
        static constexpr const bool isBuilder = true;

        static_assert((FormatIndex == -1 && VariadicIndex == -1) || (VariadicIndex > FormatIndex),
                      "VariadicIndex must be greater than FormatIndex");
    };

    template <int FormatIndex = -1, int VariadicIndex = -1>
    struct MetaPass_vscanf : public MetaPass {
        static constexpr const BulitinID ID = VScanf;
        static constexpr const bool isBuilder = true;

        static_assert((FormatIndex == -1 && VariadicIndex == -1) || (VariadicIndex > FormatIndex),
                      "VariadicIndex must be greater than FormatIndex");
    };

    // Guard
    template <bool Enabled = true>
    struct MetaPass_CallbackSubstituter : public MetaPass {
        static constexpr const BulitinID ID = CallbackSubstituter;
    };

    template <bool Enabled = true>
    struct MetaPass_TypeFilter : public MetaPass {
        static constexpr const BulitinID ID = TypeFilter;
    };

    // Misc
    template <int NameIndex = -1>
    struct MetaPass_GetProcAddress : public MetaPass {
        static constexpr const BulitinID ID = GetProcAddress;
    };

}

#endif // METAPASS_H
