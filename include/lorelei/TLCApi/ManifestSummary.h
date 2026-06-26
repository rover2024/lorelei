#ifndef LORE_TLCAPI_MANIFESTSUMMARY_H
#define LORE_TLCAPI_MANIFESTSUMMARY_H

#include <map>
#include <string>
#include <array>
#include <set>

#include <lorelei/DLCall/ProcDefs.h>
#include <lorelei/TLCApi/Global.h>

namespace lore::tool::TLC {

    /// ManifestSummary - The serializable result of TLC `stat`.
    ///
    /// For one manifest it records the source file name, the configured thunk functions per
    /// direction (both the ones that matched a declaration and the ones that did not), and the
    /// deduplicated callback signatures. It round-trips through JSON via loadFromJson() and
    /// saveAsJson().
    ///
    /// \code
    /// {
    ///   "fileName": "manifest.cpp",
    ///   "functions": {
    ///     "GuestToHost": [{ "name": "SDL_CreateWindow", "location": "SDL_video.h:42:1" }],
    ///     "HostToGuest": []
    ///   },
    ///   "missingFunctions": { "GuestToHost": ["SDL_Unmatched"], "HostToGuest": [] },
    ///   "callbacks": [{ "signature": "int (*)(void *)", "alias": "SDL_HitTest",
    ///                   "origin": "Config[Function]:SDL_SetWindowHitTest@arg2" }]
    /// }
    /// \endcode
    class LORETLCAPI_EXPORT ManifestSummary {
    public:
        using Direction = lore::thunk::ProcDirection;
        using enum lore::thunk::ProcDirection;

        /// FunctionEntry - Metadata of one configured thunk function.
        ///
        /// \note Function entries are sourced from:
        /// 1. `[Function]`        -> `GuestToHost`
        /// 2. `[Guest Function]`  -> `HostToGuest`
        struct FunctionEntry {
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

        inline void addFunction(Direction direction, std::string name, std::string location);
        inline void addCallbackSignature(const std::string &signature, const std::string &origin,
                                         const std::string &preferredAlias = {});

        bool loadFromJson(const std::string &filePath, std::string &errorMessage);
        bool saveAsJson(const std::string &filePath, std::string &errorMessage) const;

        /// The manifest source file this summary was produced from.
        std::string fileName;

        /// Configured functions that matched a declaration, keyed by name, per direction.
        std::array<std::map<std::string, FunctionEntry>, NumProcDirection> functions;

        /// Configured function names that were requested but never matched, per direction.
        std::array<std::set<std::string>, NumProcDirection> missingFunctions;

        /// Deduplicated callback signatures discovered during stat, keyed by signature.
        std::map<std::string, CallbackInfo> callbacks;
    };

    inline void ManifestSummary::addFunction(Direction direction, std::string name,
                                             std::string location) {
        auto &bucket = functions[direction];
        auto &info = bucket[name];
        info.location = std::move(location);
    }

    inline void ManifestSummary::addCallbackSignature(const std::string &signature,
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

#endif // LORE_TLCAPI_MANIFESTSUMMARY_H
