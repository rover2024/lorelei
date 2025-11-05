#ifndef MANIFESTPREDEFS_H
#define MANIFESTPREDEFS_H

#include <lorelei-c/Core/ProcInfo_c.h>
#include <lorelei/Utils/Generic/Traits.h>

namespace lorethunk {

    /// MetaProc - The meta thunk proc invoker for functions.
    template <auto F, CProcKind ProcKind, CProcThunkPhase Phase>
    struct MetaProc;

    /// MetaProcCB - The meta thunk proc invoker for callbacks.
    template <class F, CProcKind ProcKind, CProcThunkPhase Phase>
    struct MetaProcCB;

    /// MetaProcBridge - The meta thunk proc bridge to the real functions.
    template <auto F, CProcKind ProcKind>
    struct MetaProcBridge;

    /// MetaProcCBBridge - The meta thunk proc bridge to the real callbacks.
    template <class F, CProcKind ProcKind>
    struct MetaProcCBBridge;

    /// MetaProcArgContext - The meta thunk proc argument context, for \c TypeConverter passes.
    template <class... Args>
    struct MetaProcArgContext {
    private:
        std::tuple<Args &...> _args_tuple;

        static constexpr size_t _size = sizeof...(Args);

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
        MetaProcArgContext(Args &...args) : _args_tuple(args...) {
        }

        static constexpr size_t size() {
            return _size;
        }

        template <class T, size_t N = 0, size_t From = 0>
        constexpr T &type() {
            static_assert(N < _size, "N must be less than number of arguments");
            constexpr size_t index = find_type_index<T, N, From>();
            return std::get<index>(_args_tuple);
        }

        template <class T, size_t N = 0, size_t From = 0>
        constexpr bool hasType() const {
            static_assert(N < _size, "N must be less than number of arguments");
            constexpr size_t index = find_type_index<T, N, From>();
            return index < _size;
        };

        template <size_t N>
        constexpr auto &at() {
            static_assert(N < _size, "Index out of range");
            return std::get<N>(_args_tuple);
        }
    };

    /// MetaProcArgFilter - The meta thunk proc argument filter, for \c TypeConverter passes.
    template <class T>
    struct MetaProcArgFilter {
        using type = T;

        template <class MetaDesc, size_t Index, class... Args>
        static constexpr void filter(T &arg, MetaProcArgContext<Args...> &ctx) {
            static_assert(Index < sizeof...(Args), "Index out of range");
        }
    };

    /// MetaProcReturnFilter - The meta thunk proc return filter, for \c TypeConverter passes.
    template <class T>
    struct MetaProcReturnFilter {
        using type = T;

        template <class MetaDesc, class... Args>
        static constexpr void filter(T &ret, MetaProcArgContext<Args...> &ctx) {
            // Do nothing
        }
    };

}

#endif // MANIFESTPREDEFS_H
