#ifndef LORE_TLCAPI_MANIFESTFILE_H
#define LORE_TLCAPI_MANIFESTFILE_H

#include <map>
#include <string>
#include <vector>
#include <array>
#include <set>

#include <lorelei/TLCApi/Global.h>

namespace lore::tool::TLC {

    /// ManifestFile - Serializable result model for TLC `stat`.
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
    class LORETLCAPI_EXPORT ManifestFile {
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
            for (auto &set : missingFunctions) {
                set.clear();
            }
            callbacks.clear();
        }

        inline void addFunction(FunctionDirection direction, std::string name,
                                std::string location);
        inline void addCallbackSignature(const std::string &signature, const std::string &origin,
                                         const std::string &preferredAlias = {});

        bool loadFromJson(const std::string &filePath, std::string &errorMessage);
        bool saveAsJson(const std::string &filePath, std::string &errorMessage) const;

        std::string fileName;
        std::array<std::map<std::string, FunctionInfo>, NumFunctionDirection> functions;
        std::array<std::set<std::string>, NumFunctionDirection> missingFunctions;
        std::map<std::string, CallbackInfo> callbacks;
    };

    inline void ManifestFile::addFunction(FunctionDirection direction, std::string name,
                                          std::string location) {
        auto &bucket = functions[direction];
        auto &info = bucket[name];
        info.location = std::move(location);
    }

    inline void ManifestFile::addCallbackSignature(const std::string &signature,
                                                   const std::string &origin,
                                                   const std::string &preferredAlias) {
        auto &info = callbacks[signature];
        info.signature = signature;
        if (!preferredAlias.empty() && info.alias.empty()) {
            info.alias = preferredAlias;
        }
        if (!origin.empty() && info.origin.empty()) {
            info.origin = origin;
        }
    }

}

#endif // LORE_TLCAPI_MANIFESTFILE_H
