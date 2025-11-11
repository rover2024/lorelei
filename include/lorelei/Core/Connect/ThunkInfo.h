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

    struct LORECORE_EXPORT ForwardThunkInfo {
        std::string name;
        std::vector<std::string> alias;
        std::string guestThunk;
        std::string hostThunk;
        std::string hostLibrary;

        const CForwardThunkInfo &cinfo() const;
        ~ForwardThunkInfo();

    protected:
        mutable std::optional<CForwardThunkInfo> _cinfo;
    };

    struct LORECORE_EXPORT ReversedThunkInfo {
        std::string name;
        std::vector<std::string> alias;
        std::string fileName;
        std::vector<std::string> thunks;

        const CReversedThunkInfo &cinfo() const;
        ~ReversedThunkInfo();

    protected:
        mutable std::optional<CReversedThunkInfo> _cinfo;
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

        std::optional<std::reference_wrapper<const ForwardThunkInfo>>
            forwardThunk(const std::string &name) const {
            auto it = _forwardThunkMap.find(name);
            if (it == _forwardThunkMap.end()) {
                return {};
            }
            return _forwardThunks.at(it->second);
        }

        std::optional<std::reference_wrapper<const ReversedThunkInfo>>
            reversedThunk(const std::string &name) const {
            auto it = _reversedThunkMap.find(name);
            if (it == _reversedThunkMap.end()) {
                return {};
            }
            return _reversedThunks.at(it->second);
        }

    protected:
        std::vector<ForwardThunkInfo> _forwardThunks;
        std::vector<ReversedThunkInfo> _reversedThunks;

        std::map<std::string, size_t> _forwardThunkMap;
        std::map<std::string, size_t> _reversedThunkMap;
    };

}

#endif // LORECORE_THUNKINFO_H
