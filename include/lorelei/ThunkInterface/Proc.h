// SPDX-License-Identifier: MIT

#ifndef LORE_THUNKINTERFACE_PROC_H
#define LORE_THUNKINTERFACE_PROC_H

#include <tuple>

#include <lorelei/Support/Global.h>
#include <lorelei/DLCall/ProcDefs.h>

#define _PROC LORE_USED static
#define _DESC static constexpr const

namespace lore::thunk {

    /// ProcFnDesc - Per-function description block a manifest specializes to shape a proc's thunk.
    ///
    /// Specialize for a function \c F to attach passes: a \c builder_pass that picks the Caller
    /// form (for example \c pass::printf for a variadic), and a \c passes list of guard / misc
    /// passes. The empty primary template selects the default forwarding thunk.
    ///
    /// Supported fields:
    ///     static constexpr const pass::printf<> builder_pass = {};
    ///     static constexpr const pass::PassTagList<pass::GetProcAddress<>> passes = {};
    template <auto F>
    struct ProcFnDesc {};

    /// ProcCbDesc - Per-callback description block, the \c ProcFnDesc analogue for callbacks.
    ///
    /// Same shape and fields as \c ProcFnDesc, specialized on a callback function-pointer type.
    template <class F>
    struct ProcCbDesc {};

    /// ProcFn - One phase of a function's thunk, specialized per direction and phase.
    ///
    /// For a function \c F, a \c (Direction, Phase) pair names one layer of the
    /// Entry -> Adapt -> Caller -> Exec chain (see \c ProcPhase). The generator emits a
    /// specialization per phase, and a manifest may override a single one.
    template <auto F, ProcDirection Direction, ProcPhase Phase>
    struct ProcFn;

    /// ProcCb - One phase of a callback's thunk, the \c ProcFn analogue for callbacks.
    template <class F, ProcDirection Direction, ProcPhase Phase>
    struct ProcCb;

    /// ProcArgContext - A view over a call's live arguments, handed to \c TypeConverter passes.
    ///
    /// Wraps references to the marshalled arguments so a filter can reach them by position
    /// (\c at) or by type (\c type / \c hasType, with an occurrence index when a type repeats).
    template <class... Args>
    struct ProcArgContext {
    private:
        std::tuple<Args &...> args_tuple;

        template <class T, size_t N, size_t Index = 0, size_t Count = 0>
        static constexpr size_t find_type_index() {
            if constexpr (Count == N &&
                          std::is_same_v<T, std::tuple_element_t<Index, std::tuple<Args...>>>) {
                return Index;
            } else if constexpr (Index == sizeof...(Args) - 1) {
                return sizeof...(Args);
            } else {
                if constexpr (std::is_same_v<T, std::tuple_element_t<Index, std::tuple<Args...>>>) {
                    return find_type_index<T, N, Index + 1, Count + 1>();
                } else {
                    return find_type_index<T, N, Index + 1, Count>();
                }
            }
        }

    public:
        constexpr ProcArgContext(Args &...args) : args_tuple(args...) {
        }

        static constexpr size_t Size = sizeof...(Args);

        template <class T, size_t N = 0, size_t From = 0>
        constexpr T &type() {
            static_assert(N < Size, "N must be less than number of arguments");
            constexpr size_t index = find_type_index<T, N, From>();
            return std::get<index>(args_tuple);
        }

        template <class T, size_t N = 0, size_t From = 0>
        constexpr bool hasType() const {
            static_assert(N < Size, "N must be less than number of arguments");
            constexpr size_t index = find_type_index<T, N, From>();
            return index < Size;
        };

        template <size_t N>
        constexpr auto &at() {
            static_assert(N < Size, "Index out of range");
            return std::get<N>(args_tuple);
        }
    };

    /// ProcArgFilter - Per-argument customization point for \c TypeConverter passes.
    ///
    /// Specialize for a type \c T to rewrite an argument of that type in place, given the
    /// surrounding \c ProcArgContext. The primary template leaves the argument untouched.
    template <class T>
    struct ProcArgFilter {
        using type = T;

        template <class Desc, size_t Index, ProcKind Kind, ProcDirection Direction, class... Args>
        static constexpr void filter(T &arg, ProcArgContext<Args...> ctx) {
            static_assert(Index < sizeof...(Args), "Index out of range");
        }
    };

    /// ProcReturnFilter - Return-value customization point for \c TypeConverter passes.
    ///
    /// The \c ProcArgFilter analogue for the return value: specialize for \c T to rewrite the
    /// result in place. The primary template leaves it untouched.
    template <class T>
    struct ProcReturnFilter {
        using type = T;

        template <class Desc, ProcKind Kind, ProcDirection Direction, class... Args>
        static constexpr void filter(T &ret, ProcArgContext<Args...> ctx) {
        }
    };

}

#endif // LORE_THUNKINTERFACE_PROC_H
