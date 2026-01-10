#ifndef LORE_BASE_RUNTIMEBASE_THUNKINFO_H
#define LORE_BASE_RUNTIMEBASE_THUNKINFO_H

#include <string>
#include <vector>
#include <map>
#include <filesystem>
#include <memory>

#include <LoreBase/CoreLib/ADT/LinkedMap.h>
#include <LoreBase/RuntimeBase/c/LThunkInfo.h>
#include <LoreBase/RuntimeBase/Global.h>

namespace lore {

    struct LORERTBASE_EXPORT ForwardThunkInfo {
        std::string name;
        std::vector<std::string> alias;
        std::string guestThunk;
        std::string hostThunk;
        std::string hostLibrary;

        const LForwardThunkInfo &c_data() const;

        ForwardThunkInfo() = default;
        ForwardThunkInfo(ForwardThunkInfo &&) noexcept = default;
        ForwardThunkInfo &operator=(ForwardThunkInfo &&) noexcept = default;
        ~ForwardThunkInfo();

    protected:
        struct CDataGuard;
        mutable std::unique_ptr<CDataGuard> _c_data;
    };

    struct LORERTBASE_EXPORT ReversedThunkInfo {
        std::string name;
        std::vector<std::string> alias;
        std::string fileName;
        std::vector<std::string> thunks;

        const LReversedThunkInfo &c_data() const;

        ReversedThunkInfo() = default;
        ReversedThunkInfo(ReversedThunkInfo &&) noexcept = default;
        ReversedThunkInfo &operator=(ReversedThunkInfo &&) noexcept = default;
        ~ReversedThunkInfo();

    protected:
        struct CDataGuard;
        mutable std::unique_ptr<CDataGuard> _c_data;
    };

    class LORERTBASE_EXPORT ThunkInfoConfig {
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

#endif // LORE_BASE_RUNTIMEBASE_THUNKINFO_H
