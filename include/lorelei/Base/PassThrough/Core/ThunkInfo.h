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
        mutable std::unique_ptr<CDataGuard> m_c_data;
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
        mutable std::unique_ptr<CDataGuard> m_c_data;
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
            return m_forwardThunks;
        }

        inline const std::vector<ReversedThunkInfo> &reversedThunks() const {
            return m_reversedThunks;
        }

        inline const ForwardThunkInfo *forwardThunk(const std::string &name) const {
            auto it = m_forwardThunkMap.find(name);
            if (it == m_forwardThunkMap.end()) {
                return nullptr;
            }
            return &m_forwardThunks[it->second];
        }

        inline const ReversedThunkInfo *reversedThunk(const std::string &name) const {
            auto it = m_reversedThunkMap.find(name);
            if (it == m_reversedThunkMap.end()) {
                return {};
            }
            return &m_reversedThunks[it->second];
        }

    protected:
        std::vector<ForwardThunkInfo> m_forwardThunks;
        std::vector<ReversedThunkInfo> m_reversedThunks;

        std::map<std::string, size_t> m_forwardThunkMap;
        std::map<std::string, size_t> m_reversedThunkMap;
    };

}

#endif // LORE_BASE_PASSTHROUGH_THUNKINFO_H
