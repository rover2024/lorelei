// SPDX-License-Identifier: MIT

#ifndef LORE_DLCALL_THUNKDATABASE_H
#define LORE_DLCALL_THUNKDATABASE_H

#include <deque>
#include <filesystem>
#include <map>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <lorelei/DLCall/Global.h>

namespace lore {

    /// Forward thunk: maps a guest library to its guest/host thunk pair.
    ///
    /// Plain-old-data so it can be handed across the guest/host boundary directly. Strings are
    /// NUL-terminated. \c alias is an array of \c aliasCount string pointers.
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

        /// Load \a jsonPath as the whole database, discarding any previous contents. \a vars supplies
        /// the JSON ${...} substitutions (including GTL_DIR / HTL_DIR for a JSON's shorthand entries).
        /// Returns false if the file cannot be opened (empty, missing, or unreadable path) or parsed.
        /// Whether a missing file is acceptable is the caller's decision. This is the top-level
        /// (override) load.
        bool load(const std::filesystem::path &jsonPath,
                  const std::map<std::string, std::string> &vars = {});

        /// Layer one thunk pack's JSON on top of the loaded database without discarding it. Existing
        /// entries win (insert-if-absent), so a pack loaded later never displaces the override JSON or
        /// an earlier pack. \a gtlDir / \a hostThunkDir supply that JSON's shorthand defaults
        /// (GTL_DIR / HTL_DIR). Used for run-time discovery, the first time a guest thunk from the pack
        /// appears. Returns false if the JSON cannot be opened or parsed. The caller decides whether a
        /// pack without a JSON is acceptable (usually it is, the convention covering it).
        bool loadPack(const std::filesystem::path &jsonPath, const std::filesystem::path &guestThunkDir,
                      const std::filesystem::path &hostThunkDir,
                      const std::map<std::string, std::string> &vars);

        const std::deque<CForwardThunkInfo> &forwardThunks() const {
            return m_forwardThunks;
        }

        const std::deque<CReversedThunkInfo> &reversedThunks() const {
            return m_reversedThunks;
        }

        const CForwardThunkInfo *forwardThunk(const std::string &name) const {
            return lookup(m_forwardThunks, m_forwardThunkMap, name);
        }

        const CReversedThunkInfo *reversedThunk(const std::string &name) const {
            return lookup(m_reversedThunks, m_reversedThunkMap, name);
        }

    private:
        // Layer the JSON at \a path over the current entries: forward entries (upsert by name) plus
        // reversed thunks. With \a overwrite an entry replaces any existing one of the same name (the
        // top-level load). Without it, an existing entry is kept (a pack layered underneath). Returns
        // false if the file cannot be opened or parsed.
        bool loadJsonDatabase(const std::filesystem::path &path,
                              const std::map<std::string, std::string> &vars, bool overwrite);

        // Add a forward thunk. If one with the same name exists, replace it when \a overwrite is true,
        // otherwise keep the existing entry.
        void upsertForward(std::string name, const std::vector<std::string> &alias,
                           std::string guestThunk, std::string hostThunk, std::string hostLibrary,
                           bool overwrite);

        // Rebuild the name/alias lookup indexes from the current entries.
        void rebuildIndexes();

        const char *intern(std::string str);
        const char *const *intern(const std::vector<std::string> &strs, size_t &count);

        template <class T>
        static const T *lookup(const std::deque<T> &items,
                               const std::unordered_map<std::string, size_t> &index,
                               const std::string &name) {
            auto it = index.find(name);
            return it == index.end() ? nullptr : &items[it->second];
        }

        // Stable-address backing storage for everything the entries point at. The entry containers are
        // deques too: run-time discovery appends to them (loadPack) while the guest may still hold a
        // pointer to an earlier entry, and a deque never relocates existing elements.
        std::deque<std::string> m_stringArena;
        std::deque<std::vector<const char *>> m_listArena;

        std::deque<CForwardThunkInfo> m_forwardThunks;
        std::deque<CReversedThunkInfo> m_reversedThunks;

        std::unordered_map<std::string, size_t> m_forwardThunkMap;
        std::unordered_map<std::string, size_t> m_reversedThunkMap;

        // name -> index into m_forwardThunks, maintained by upsertForward so a later JSON or pack can
        // find an existing entry by name.
        std::unordered_map<std::string, size_t> m_forwardIndex;
    };

}

#endif // LORE_DLCALL_THUNKDATABASE_H
