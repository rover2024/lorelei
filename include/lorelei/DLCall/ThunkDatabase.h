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

        /// LoadOptions - Tunes how load() populates the database.
        struct LoadOptions {
            /// Auto-discover forward thunks by scanning GTL_DIR / HTL_DIR. The JSON, if present, is
            /// then layered on top as overrides. Turn off to use the JSON alone.
            bool autoScan = true;
        };

        /// Populate the database. The default directories (GTL_DIR / HTL_DIR in \a vars) are scanned
        /// first (unless disabled in \a opts), then the JSON at \a path is layered on top: matching
        /// names replace the scanned entry, new names are added. The JSON is optional; a missing file
        /// just means no overrides. Returns false only when a JSON file is present but cannot be parsed.
        bool load(const std::filesystem::path &path,
                  const std::map<std::string, std::string> &vars,
                  const LoadOptions &opts);

        bool load(const std::filesystem::path &path,
                  const std::map<std::string, std::string> &vars = {}) {
            return load(path, vars, LoadOptions{});
        }

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
        // Auto-discover forward thunks by scanning GTL_DIR for *.so that have a matching
        // HTL_DIR/<name>_HTL.so (both from \a vars). Upserts by name.
        void autoScan(const std::map<std::string, std::string> &vars);

        // Layer the JSON at \a path over the current entries: forward overrides (upsert by name) plus
        // reversed thunks. Returns false only if the file is present but cannot be parsed.
        bool loadJsonDatabase(const std::filesystem::path &path,
                              const std::map<std::string, std::string> &vars);

        // Add a forward thunk, or replace the existing one with the same name.
        void upsertForward(std::string name, const std::vector<std::string> &alias,
                           std::string guestThunk, std::string hostThunk, std::string hostLibrary);

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

        // name -> index into m_forwardThunks, maintained by upsertForward across a load so the JSON
        // can replace a scanned entry by name.
        std::unordered_map<std::string, size_t> m_forwardIndex;
    };

}

#endif // LORE_DLCALL_THUNKDATABASE_H
