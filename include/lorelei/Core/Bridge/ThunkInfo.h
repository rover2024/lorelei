#ifndef LORECORE_THUNKINFO_H
#define LORECORE_THUNKINFO_H

#include <string>
#include <vector>
#include <map>
#include <optional>
#include <filesystem>

#include <lorelei/Core/Global.h>

#include <lorelei-c/Core/ThunkInfo_c.h>

namespace lore {

    struct ForwardThunkInfo {
        std::string name;
        std::vector<std::string> alias;
        std::string guestThunk;
        std::string hostThunk;
        std::string hostLibrary;
    };

    struct ReversedThunkInfo {
        std::string name;
        std::vector<std::string> alias;
        std::string fileName;
        std::vector<std::string> thunks;
    };

    struct LORECORE_EXPORT ThunkInfo {
        std::optional<ForwardThunkInfo> forward;
        std::optional<ReversedThunkInfo> reversed;

        /// Aligned memory buffer for ThunkInfo.
        /// This is used to pass ThunkInfo between memory boundaries.
        CThunkInfo *toCThunkInfo() const;
        static ThunkInfo fromCThunkInfo(const CThunkInfo *info);
        static void freeCThunkInfo(CThunkInfo *info);
    };

    class LORECORE_EXPORT ThunkInfoConfig {
    public:
        ThunkInfoConfig() = default;
        ~ThunkInfoConfig() = default;

    public:
        bool load(const std::filesystem::path &path,
                  const std::map<std::string, std::string> &vars = {});

        const std::vector<ForwardThunkInfo> &forwardThunks() const {
            return _forwardThunks;
        }

        const std::vector<ReversedThunkInfo> &reversedThunks() const {
            return _reversedThunks;
        }

        std::pair<bool, const ForwardThunkInfo &> forwardThunk(const std::string &name) const {
            auto it = _forwardThunkMap.find(name);
            if (it == _forwardThunkMap.end()) {
                static ForwardThunkInfo empty;
                return {false, empty};
            }
            return {true, _forwardThunks.at(it->second)};
        }

        std ::pair<bool, const ReversedThunkInfo &> reversedThunk(const std::string &name) const {
            auto it = _reversedThunkMap.find(name);
            if (it == _reversedThunkMap.end()) {
                static ReversedThunkInfo empty;
                return {false, empty};
            }
            return {true, _reversedThunks.at(it->second)};
        }

    protected:
        std::vector<ForwardThunkInfo> _forwardThunks;
        std::vector<ReversedThunkInfo> _reversedThunks;

        std::map<std::string, size_t> _forwardThunkMap;
        std::map<std::string, size_t> _reversedThunkMap;
    };

}

#endif // LORECORE_THUNKINFO_H
