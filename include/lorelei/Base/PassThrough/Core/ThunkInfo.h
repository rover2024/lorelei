#ifndef LORE_BASE_PASSTHROUGH_THUNKINFO_H
#define LORE_BASE_PASSTHROUGH_THUNKINFO_H

#include <string>
#include <vector>
#include <map>
#include <filesystem>
#include <memory>

#include <lorelei/Base/PassThrough/c/CThunkInfo.h>
#include <lorelei/Base/PassThrough/Global.h>

namespace lore {

    struct LOREPASSTHROUGH_EXPORT ForwardThunkInfo {
        std::string name;
        std::vector<std::string> alias;
        std::string guestThunk;
        std::string hostThunk;
        std::string hostLibrary;

        const CForwardThunkInfo &c_data() const;

        ForwardThunkInfo() = default;
        ForwardThunkInfo(ForwardThunkInfo &&) noexcept = default;
        ForwardThunkInfo &operator=(ForwardThunkInfo &&) noexcept = default;
        ~ForwardThunkInfo();

    protected:
        struct CDataGuard;
        mutable std::unique_ptr<CDataGuard> _c_data;
    };

    struct LOREPASSTHROUGH_EXPORT ReversedThunkInfo {
        std::string name;
        std::vector<std::string> alias;
        std::string fileName;
        std::vector<std::string> thunks;

        const CReversedThunkInfo &c_data() const;

        ReversedThunkInfo() = default;
        ReversedThunkInfo(ReversedThunkInfo &&) noexcept = default;
        ReversedThunkInfo &operator=(ReversedThunkInfo &&) noexcept = default;
        ~ReversedThunkInfo();

    protected:
        struct CDataGuard;
        mutable std::unique_ptr<CDataGuard> _c_data;
    };

    class LOREPASSTHROUGH_EXPORT ThunkInfoConfig {
    public:
        ThunkInfoConfig() = default;
        ~ThunkInfoConfig() = default;

        ThunkInfoConfig(const ThunkInfoConfig &) = delete;
        ThunkInfoConfig &operator=(const ThunkInfoConfig &) = delete;

    public:
        bool load(const std::filesystem::path &path,
                  const std::map<std::string, std::string> &vars = {});

        inline const std::vector<ForwardThunkInfo> &forwardThunks() const {
            return _forwardThunks;
        }

        inline const std::vector<ReversedThunkInfo> &reversedThunks() const {
            return _reversedThunks;
        }

        inline const ForwardThunkInfo *forwardThunk(const std::string &name) const {
            auto it = _forwardThunkMap.find(name);
            if (it == _forwardThunkMap.end()) {
                return nullptr;
            }
            return &_forwardThunks[it->second];
        }

        inline const ReversedThunkInfo *reversedThunk(const std::string &name) const {
            auto it = _reversedThunkMap.find(name);
            if (it == _reversedThunkMap.end()) {
                return {};
            }
            return &_reversedThunks[it->second];
        }

    protected:
        std::vector<ForwardThunkInfo> _forwardThunks;
        std::vector<ReversedThunkInfo> _reversedThunks;

        std::map<std::string, size_t> _forwardThunkMap;
        std::map<std::string, size_t> _reversedThunkMap;
    };

}

#endif // LORE_BASE_PASSTHROUGH_THUNKINFO_H
