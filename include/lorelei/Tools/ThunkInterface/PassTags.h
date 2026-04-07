#ifndef LORE_TOOLS_THUNKINTERFACE_PASSTAGS_H
#define LORE_TOOLS_THUNKINTERFACE_PASSTAGS_H

namespace lore::thunk::pass {

    struct PassTagBase {
        enum BuiltinID {
            /// Builder
            ID_DefaultBuilder = 0,
            ID_printf,
            ID_vprintf,
            ID_scanf,
            ID_vscanf,

            /// Guard
            ID_CallbackSubstituter,
            ID_TypeFilter,

            /// Misc
            ID_GetProcAddress,

            /// User
            ID_User = 0x1000,
        };
    };

    template <class... Args>
    struct PassTags {};

    // Builder
    template <int FormatIndex = -1, int VariadicIndex = -1>
    struct printf : public PassTagBase {
        static constexpr const BuiltinID ID = ID_printf;
        static constexpr const bool isBuilder = true;

        static_assert((FormatIndex == -1 && VariadicIndex == -1) || (VariadicIndex > FormatIndex),
                      "VariadicIndex must be greater than FormatIndex");
    };

    template <int FormatIndex = -1, int VariadicIndex = -1>
    struct vprintf : public PassTagBase {
        static constexpr const BuiltinID ID = ID_vprintf;
        static constexpr const bool isBuilder = true;

        static_assert((FormatIndex == -1 && VariadicIndex == -1) || (VariadicIndex > FormatIndex),
                      "VariadicIndex must be greater than FormatIndex");
    };

    template <int FormatIndex = -1, int VariadicIndex = -1>
    struct scanf : public PassTagBase {
        static constexpr const BuiltinID ID = ID_scanf;
        static constexpr const bool isBuilder = true;

        static_assert((FormatIndex == -1 && VariadicIndex == -1) || (VariadicIndex > FormatIndex),
                      "VariadicIndex must be greater than FormatIndex");
    };

    template <int FormatIndex = -1, int VariadicIndex = -1>
    struct vscanf : public PassTagBase {
        static constexpr const BuiltinID ID = ID_vscanf;
        static constexpr const bool isBuilder = true;

        static_assert((FormatIndex == -1 && VariadicIndex == -1) || (VariadicIndex > FormatIndex),
                      "VariadicIndex must be greater than FormatIndex");
    };

    // Misc
    template <int NameIndex = -1>
    struct GetProcAddress : public PassTagBase {
        static constexpr const BuiltinID ID = ID_GetProcAddress;
    };

}

#endif // LORE_TOOLS_THUNKINTERFACE_PASSTAGS_H
