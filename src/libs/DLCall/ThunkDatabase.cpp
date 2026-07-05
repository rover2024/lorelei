// SPDX-License-Identifier: MIT

#include "ThunkDatabase.h"

#include <fstream>
#include <optional>
#include <sstream>

#include <json11/json11.hpp>

#include <lorelei/Support/StringExtras.h>

namespace lore {

    using json11::Json;

    namespace {

        // Returns the value of a non-empty string field, or nullopt if the field is missing,
        // not a string, or empty.
        std::optional<std::string> jsonString(const Json::object &obj, const char *key) {
            auto it = obj.find(key);
            if (it == obj.end() || !it->second.is_string()) {
                return std::nullopt;
            }
            auto val = it->second.string_value();
            if (val.empty()) {
                return std::nullopt;
            }
            return val;
        }

        // Collects the non-empty string elements of an array field (empty if absent).
        std::vector<std::string> jsonStringArray(const Json::object &obj, const char *key) {
            std::vector<std::string> out;
            auto it = obj.find(key);
            if (it == obj.end() || !it->second.is_array()) {
                return out;
            }
            for (const auto &item : it->second.array_items()) {
                if (item.is_string() && !item.string_value().empty()) {
                    out.push_back(item.string_value());
                }
            }
            return out;
        }

    }

    const char *ThunkDatabase::intern(std::string str) {
        return m_stringArena.emplace_back(std::move(str)).c_str();
    }

    const char *const *ThunkDatabase::intern(const std::vector<std::string> &strs, size_t &count) {
        std::vector<const char *> &list = m_listArena.emplace_back();
        list.reserve(strs.size());
        for (const auto &s : strs) {
            list.push_back(intern(s));
        }
        count = list.size();
        return list.data();
    }

    void ThunkDatabase::upsertForward(std::string name, const std::vector<std::string> &alias,
                                      std::string guestThunk, std::string hostThunk,
                                      std::string hostLibrary) {
        CForwardThunkInfo info;
        info.name = intern(name);
        info.alias = intern(alias, info.aliasCount);
        info.guestThunk = intern(std::move(guestThunk));
        info.hostThunk = intern(std::move(hostThunk));
        info.hostLibrary = intern(std::move(hostLibrary));
        // Replace an entry an earlier source added under the same name, rather than duplicate it.
        if (auto it = m_forwardIndex.find(name); it != m_forwardIndex.end()) {
            m_forwardThunks[it->second] = info;
        } else {
            m_forwardIndex.emplace(std::move(name), m_forwardThunks.size());
            m_forwardThunks.push_back(info);
        }
    }

    void ThunkDatabase::autoScan(
        const std::vector<std::pair<std::filesystem::path, std::filesystem::path>> &dirPairs) {
        for (const auto &[gtlDir, htlDir] : dirPairs) {
            if (gtlDir.empty() || htlDir.empty() || !std::filesystem::is_directory(gtlDir)) {
                continue;
            }

            // Every GTL/*.so with a matching HTL/<name>_HTL.so becomes a forward thunk. A name an
            // earlier (higher-priority) pair already claimed is skipped, so the first match wins.
            for (const auto &entry : std::filesystem::directory_iterator(gtlDir)) {
                if (!entry.is_regular_file() && !entry.is_symlink()) {
                    continue;
                }
                const auto fileName = entry.path().filename().string();
                if (!str::ends_with(fileName, ".so")) {
                    continue;
                }
                const auto name = fileName.substr(0, fileName.size() - 3);
                if (name.empty() || m_forwardIndex.count(name)) {
                    continue;
                }
                auto hostThunk = (htlDir / (name + "_HTL.so")).string();
                if (!std::filesystem::exists(hostThunk)) {
                    continue;
                }
                upsertForward(name, {}, entry.path().string(), std::move(hostThunk), name + ".so");
            }
        }
    }

    bool ThunkDatabase::loadJsonDatabase(const std::filesystem::path &path,
                                         const std::map<std::string, std::string> &vars) {
        std::ifstream file(path);
        if (!file.is_open()) {
            return true;  // no file just means no overrides, which is not an error
        }

        std::stringstream ss;
        ss << file.rdbuf();
        std::string err;
        auto json = Json::parse(ss.str(), err);
        if (!err.empty() || !json.is_object()) {
            return false;
        }

        const auto resolvePath = [&](const std::string &p) { return str::varexp(p, vars); };

        std::string defaultGuestThunkPath;
        std::string defaultHostThunkPath;
        if (auto it = vars.find("GTL_DIR"); it != vars.end()) {
            defaultGuestThunkPath = it->second;
        }
        if (auto it = vars.find("HTL_DIR"); it != vars.end()) {
            defaultHostThunkPath = it->second;
        }
        const bool allHasDefault = !defaultGuestThunkPath.empty() && !defaultHostThunkPath.empty();

        const auto &docObj = json.object_items();

        // ---- forward thunks ----
        if (auto it = docObj.find("forwardThunks"); it != docObj.end() && it->second.is_array()) {
            for (const auto &item : it->second.array_items()) {
                // Shorthand form: just a name, filled out from the default directories.
                if (item.is_string()) {
                    const std::string name = item.string_value();
                    if (!name.empty() && allHasDefault) {
                        upsertForward(name, {}, defaultGuestThunkPath + "/" + name + ".so",
                                      defaultHostThunkPath + "/" + name + "_HTL.so", name + ".so");
                    }
                    continue;
                }
                if (!item.is_object()) {
                    continue;
                }
                const auto &obj = item.object_items();

                auto name = jsonString(obj, "name");
                if (!name) {
                    continue;
                }

                std::string guestThunk;
                if (auto found = obj.find("guestThunk"); found == obj.end()) {
                    if (defaultGuestThunkPath.empty()) {
                        continue;
                    }
                    guestThunk = defaultGuestThunkPath + "/" + *name + ".so";
                } else if (auto val = jsonString(obj, "guestThunk")) {
                    guestThunk = resolvePath(*val);
                } else {
                    continue;
                }

                std::string hostThunk;
                if (auto found = obj.find("hostThunk"); found == obj.end()) {
                    if (defaultHostThunkPath.empty()) {
                        continue;
                    }
                    hostThunk = defaultHostThunkPath + "/" + *name + "_HTL.so";
                } else if (auto val = jsonString(obj, "hostThunk")) {
                    hostThunk = resolvePath(*val);
                } else {
                    continue;
                }

                std::string hostLibrary;
                if (auto found = obj.find("hostLibrary"); found == obj.end()) {
                    hostLibrary = *name + ".so";
                } else if (auto val = jsonString(obj, "hostLibrary")) {
                    hostLibrary = resolvePath(*val);
                } else {
                    continue;
                }

                upsertForward(*name, jsonStringArray(obj, "alias"), std::move(guestThunk),
                              std::move(hostThunk), std::move(hostLibrary));
            }
        }

        // ---- reversed thunks ----
        if (auto it = docObj.find("reversedThunks"); it != docObj.end() && it->second.is_array()) {
            for (const auto &item : it->second.array_items()) {
                if (!item.is_object()) {
                    continue;
                }
                const auto &obj = item.object_items();

                auto name = jsonString(obj, "name");
                if (!name) {
                    continue;
                }
                auto fileName = jsonString(obj, "fileName");
                if (!fileName) {
                    continue;
                }

                CReversedThunkInfo info;
                info.name = intern(std::move(*name));
                info.alias = intern(jsonStringArray(obj, "alias"), info.aliasCount);
                info.fileName = intern(std::move(*fileName));
                info.thunks = intern(jsonStringArray(obj, "thunks"), info.thunksCount);
                m_reversedThunks.push_back(info);
            }
        }

        return true;
    }

    bool ThunkDatabase::load(const std::filesystem::path &path,
                             const std::map<std::string, std::string> &vars,
                             const ThunkDatabase::LoadOptions &opts) {
        m_stringArena.clear();
        m_listArena.clear();
        m_forwardThunks.clear();
        m_reversedThunks.clear();
        m_forwardThunkMap.clear();
        m_reversedThunkMap.clear();
        m_forwardIndex.clear();

        // Scan the search path for a baseline (first match for a name wins), then layer the JSON
        // overrides on top. An empty search path just leaves the JSON as the only source.
        autoScan(opts.scanDirs);
        const bool jsonOk = loadJsonDatabase(path, vars);

        // Build the name/alias lookup indexes from the final entries.
        const auto indexEntries = [](const auto &items, auto &index) {
            for (size_t i = 0; i < items.size(); ++i) {
                const auto &entry = items[i];
                index.emplace(entry.name, i);
                for (size_t j = 0; j < entry.aliasCount; ++j) {
                    index.emplace(entry.alias[j], i);
                }
            }
        };
        indexEntries(m_forwardThunks, m_forwardThunkMap);
        indexEntries(m_reversedThunks, m_reversedThunkMap);

        return jsonOk;
    }

}
