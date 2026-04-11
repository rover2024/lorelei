#ifndef LORE_TOOLS_TLC_MANIFESTSTATISTICS_H
#define LORE_TOOLS_TLC_MANIFESTSTATISTICS_H

#include <map>
#include <string>
#include <vector>
#include <array>

#include <lorelei/Tools/TLCApi/Global.h>

namespace lore::tool::TLC {

    /// ManifestStatistics - Serializable result model for TLC `stat`.
    ///
    /// This class keeps two categories of data:
    /// 1. `functions`: configured thunk functions partitioned by direction.
    /// 2. `callbacks`: deduplicated callback signatures discovered from function types and
    ///    explicit callback config seeds.
    ///
    /// \example Sample `stat.json`
    /// \code
    /// {
    ///   "functions": {
    ///     "GuestToHost": [
    ///       {
    ///         "name": "SDL_SetWindowHitTest",
    ///         "signature": "int (*)(struct SDL_Window *, SDL_HitTest, void *)",
    ///         "location": "/path/to/SDL_video.h:123:1"
    ///       }
    ///     ],
    ///     "HostToGuest": []
    ///   },
    ///   "callbacks": [
    ///     {
    ///       "signature": "SDL_HitTestResult (*)(struct SDL_Window *, const struct SDL_Point *,
    ///       void *)", "alias": "SDL_HitTest", "origin":
    ///       "Config[Function]:SDL_SetWindowHitTest@arg2"
    ///     }
    ///   ],
    ///   "version": 1
    /// }
    /// \endcode
    class LORETLCAPI_EXPORT ManifestStatistics {
    public:
        enum FunctionDirection {
            GuestToHost = 0,
            HostToGuest = 1,
            NumFunctionDirection = 2,
        };

        /// FunctionInfo - Metadata of one configured thunk function.
        ///
        /// \note Function entries are sourced from:
        /// 1. `[Function]`        -> `GuestToHost`
        /// 2. `[Guest Function]`  -> `HostToGuest`
        struct FunctionInfo {
            /// Canonical function pointer type string.
            /// \example "int (*)(void *, size_t)"
            std::string signature;
            /// Source location of the matched function declaration.
            /// \example "/path/to/header.h:42:1"
            std::string location;
        };

        /// CallbackInfo - One callback signature summary entry.
        struct CallbackInfo {
            /// Canonical callback signature.
            std::string signature;
            /// Preferred readable alias of the callback type.
            std::string alias;
            /// First matched origin token from stat collection.
            /// \example "Config[Function]:SDL_SetWindowHitTest@arg2"
            std::string origin;
        };

        inline void clear() {
            fileName.clear();
            for (auto &map : functions) {
                map.clear();
            }
            callbacks.clear();
        }

        void addFunction(FunctionDirection direction, std::string name, std::string signature,
                         std::string location);
        void addCallbackSignature(const std::string &signature, const std::string &origin,
                                  const std::string &preferredAlias = {});

        bool loadFromJson(const std::string &filePath, std::string &errorMessage);
        bool saveAsJson(const std::string &filePath, std::string &errorMessage) const;

        std::string fileName;
        std::array<std::map<std::string, FunctionInfo>, NumFunctionDirection> functions;
        std::map<std::string, CallbackInfo> callbacks;
    };

}

#endif // LORE_TOOLS_TLC_MANIFESTSTATISTICS_H
