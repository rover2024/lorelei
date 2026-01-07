// SPDX-License-Identifier: MIT

#ifndef LORELEI_THUNKINTEFACE_PROC_H
#define LORELEI_THUNKINTEFACE_PROC_H

#include <tuple>

#include "Proc_c.h"

namespace lore::thunk {

    enum ProcKind {
        Function = CProcKind_Function,
        Callback = CProcKind_Callback,
        NumProcKind = CProcKind_NumProcKind,
    };

    enum ProcDirection {
        GuestToHost = CProcDirection_GuestToHost,
        HostToGuest = CProcDirection_HostToGuest,
        NumProcDirection = CProcDirection_NumProcDirection,
    };

    enum ProcPhase {
        Entry = CProcPhase_Entry,
        Caller = CProcPhase_Caller,
        Exec = CProcPhase_Exec,
        NumPhases = CProcPhase_NumPhases,
    };

    /// ProcFnDesc - Thunk proc description block for functions.
    template <auto F>
    struct ProcFnDesc {
        /// \example Supported fields
        ///
        /// \code
        ///     static constexpr const auto BUILDER_PASS = pass::printf<>;
        ///     static constexpr const auto MISC_PASSES =
        ///         pass::PassTags<pass::GetProcAddress<>>
        ///     ;
        /// \endcode
    };

    /// ProcFnDesc - Thunk proc description block for callbacks.
    template <class F>
    struct ProcCbDesc {
        /// \sa ProcFnDesc
    };

    /// ProcFn - Thunk proc for functions.
    template <auto F, ProcKind Kind, ProcPhase Phase>
    struct ProcFn;

    /// ProcCb - Thunk proc for callbacks.
    template <class F, ProcKind Kind, ProcPhase Phase>
    struct ProcCb;

    /// ProcArgContext - The thunk proc argument context, for \c TypeConverter passes.
    template <class... Args>
    struct ProcArgContext {
    private:
        std::tuple<Args &...> _args_tuple;

        static constexpr size_t s_size = sizeof...(Args);

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
        constexpr ProcArgContext(Args &...args) : _args_tuple(args...) {
        }

        static constexpr size_t size() {
            return s_size;
        }

        template <class T, size_t N = 0, size_t From = 0>
        constexpr T &type() {
            static_assert(N < s_size, "N must be less than number of arguments");
            constexpr size_t index = find_type_index<T, N, From>();
            return std::get<index>(_args_tuple);
        }

        template <class T, size_t N = 0, size_t From = 0>
        constexpr bool hasType() const {
            static_assert(N < s_size, "N must be less than number of arguments");
            constexpr size_t index = find_type_index<T, N, From>();
            return index < s_size;
        };

        template <size_t N>
        constexpr auto &at() {
            static_assert(N < s_size, "Index out of range");
            return std::get<N>(_args_tuple);
        }
    };

    /// ProcArgFilter - The thunk proc argument filter, for \c TypeConverter passes.
    template <class T>
    struct ProcArgFilter {
        using type = T;

        template <class Desc, size_t Index, ProcKind Kind, class... Args>
        static constexpr void filter(T &arg, ProcArgContext<Args...> ctx) {
            static_assert(Index < sizeof...(Args), "Index out of range");
        }
    };

    /// ProcReturnFilter - The thunk proc return filter, for \c TypeConverter passes.
    template <class T>
    struct ProcReturnFilter {
        using type = T;

        template <class Desc, ProcKind Kind, class... Args>
        static constexpr void filter(T &ret, ProcArgContext<Args...> ctx) {
        }
    };

}

#endif // LORELEI_THUNKINTEFACE_PROC_H
