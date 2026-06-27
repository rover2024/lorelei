// SPDX-License-Identifier: MIT

#ifndef LORE_DLCALL_THUNKDATABASE_H
#define LORE_DLCALL_THUNKDATABASE_H

#include <deque>
#include <filesystem>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include <lorelei/DLCall/Global.h>

namespace lore {

    /// Forward thunk: maps a guest library to its guest/host thunk pair.
    ///
    /// Plain-old-data so it can be handed across the guest/host boundary directly. Strings are
    /// NUL-terminated; \c alias is an array of \c aliasCount string pointers.
    struct CForwardThunkInfo {
        const char *name;
        const char *const *alias;
        size_t aliasCount;
        const char *guestThunk;
        const char *hostThunk;
        const char *hostLibrary;
    };

    /// Reversed thunk: maps a host library back to the forward thunks it depends on.
    struct CReversedThunkInfo {
        const char *name;
        const char *const *alias;
        size_t aliasCount;
        const char *fileName;
        const char *const *thunks;
        size_t thunksCount;
    };

    /// CThunkInfo - A pointer to one thunk entry, either a forward or a reversed description.
    union CThunkInfo {
        const CForwardThunkInfo *forward;
        const CReversedThunkInfo *reversed;
    };

    /// ThunkDatabase - In-memory database of thunk descriptions loaded from a JSON file.
    ///
    /// Entries are exposed as plain-old-data so they can cross the guest/host boundary without
    /// any layout assumptions. The database owns the backing string storage in a stable arena,
    /// so every pointer in an entry stays valid for the lifetime of the database.
    class LOREDLCALL_EXPORT ThunkDatabase {
    public:
        ThunkDatabase() = default;
        ~ThunkDatabase() = default;

        ThunkDatabase(const ThunkDatabase &) = delete;
        ThunkDatabase &operator=(const ThunkDatabase &) = delete;

        bool load(const std::filesystem::path &path,
                  const std::map<std::string, std::string> &vars = {});

        const std::vector<CForwardThunkInfo> &forwardThunks() const {
            return m_forwardThunks;
        }

        const std::vector<CReversedThunkInfo> &reversedThunks() const {
            return m_reversedThunks;
        }

        const CForwardThunkInfo *forwardThunk(const std::string &name) const {
            return lookup(m_forwardThunks, m_forwardThunkMap, name);
        }

        const CReversedThunkInfo *reversedThunk(const std::string &name) const {
            return lookup(m_reversedThunks, m_reversedThunkMap, name);
        }

    private:
        const char *intern(std::string str);
        const char *const *intern(const std::vector<std::string> &strs, size_t &count);

        template <class T>
        static const T *lookup(const std::vector<T> &items,
                               const std::unordered_map<std::string, size_t> &index,
                               const std::string &name) {
            auto it = index.find(name);
            return it == index.end() ? nullptr : &items[it->second];
        }

        // Stable-address backing storage for everything the entries point at.
        std::deque<std::string> m_stringArena;
        std::deque<std::vector<const char *>> m_listArena;

        std::vector<CForwardThunkInfo> m_forwardThunks;
        std::vector<CReversedThunkInfo> m_reversedThunks;

        std::unordered_map<std::string, size_t> m_forwardThunkMap;
        std::unordered_map<std::string, size_t> m_reversedThunkMap;
    };

}

#endif // LORE_DLCALL_THUNKDATABASE_H
