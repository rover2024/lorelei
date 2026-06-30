// SPDX-License-Identifier: MIT

#ifndef LORE_THUNKINTERFACE_PASSTAGS_H
#define LORE_THUNKINTERFACE_PASSTAGS_H

namespace lore::thunk::pass {

    /// PassID - Stable identifier for each built-in pass, grouped by role (Builder, Guard, Misc),
    /// with \c ID_User as the base for user-defined passes.
    enum PassID {
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

    /// PassTagBase - Base of every pass tag.
    struct PassTagBase {};

    /// PassTagList - Carries the extra passes for a proc, used as the \c passes field of a
    /// \c ProcFnDesc / \c ProcCbDesc.
    template <class... Args>
    struct PassTagList {};

    // --- Builder -------------------------------------------------------------------

    /// printf - Builder tag for a printf-style variadic function (the variadic tail is marshalled
    /// according to the format string). \a FormatIndex is the parameter holding the format string
    /// and \a VariadicIndex is where the variadic part begins. \c -1 lets TLC infer them.
    template <int FormatIndex = -1, int VariadicIndex = -1>
    struct printf : public PassTagBase {
        static constexpr const PassID ID = ID_printf;
        static constexpr const bool isBuilder = true;

        static_assert((FormatIndex == -1 && VariadicIndex == -1) || (VariadicIndex > FormatIndex),
                      "VariadicIndex must be greater than FormatIndex");
    };

    /// vprintf - Like \c printf, but for a function that takes a \c va_list instead of \c ...
    template <int FormatIndex = -1, int VariadicIndex = -1>
    struct vprintf : public PassTagBase {
        static constexpr const PassID ID = ID_vprintf;
        static constexpr const bool isBuilder = true;

        static_assert((FormatIndex == -1 && VariadicIndex == -1) || (VariadicIndex > FormatIndex),
                      "VariadicIndex must be greater than FormatIndex");
    };

    /// scanf - Like \c printf, but for a scanf-style function whose format describes output
    /// pointers rather than input values.
    template <int FormatIndex = -1, int VariadicIndex = -1>
    struct scanf : public PassTagBase {
        static constexpr const PassID ID = ID_scanf;
        static constexpr const bool isBuilder = true;

        static_assert((FormatIndex == -1 && VariadicIndex == -1) || (VariadicIndex > FormatIndex),
                      "VariadicIndex must be greater than FormatIndex");
    };

    /// vscanf - Like \c scanf, but for a function that takes a \c va_list instead of \c ...
    template <int FormatIndex = -1, int VariadicIndex = -1>
    struct vscanf : public PassTagBase {
        static constexpr const PassID ID = ID_vscanf;
        static constexpr const bool isBuilder = true;

        static_assert((FormatIndex == -1 && VariadicIndex == -1) || (VariadicIndex > FormatIndex),
                      "VariadicIndex must be greater than FormatIndex");
    };

    // --- Misc -------------------------------------------------------------------

    /// GetProcAddress - Misc tag for a \c *GetProcAddress*-style function that returns a host proc
    /// address. The pass rewrites the result into a guest-callable address. \a NameIndex is the
    /// argument holding the symbol name. \c -1 lets TLC infer it.
    template <int NameIndex = -1>
    struct GetProcAddress : public PassTagBase {
        static constexpr const PassID ID = ID_GetProcAddress;
    };

}

#endif // LORE_THUNKINTERFACE_PASSTAGS_H
