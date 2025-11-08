#include "ThunkInfo.h"

#include <fstream>

#include <json11.hpp>

#include <stdcorelib/str.h>

#include <lorelei-c/Core/ThunkInfo_c.h>

namespace lore {

    CThunkInfo *ThunkInfo::toCThunkInfo() const {
        auto info = new CThunkInfo();
        if (forward) {
            info->hasForward = true;
            // name
            info->forward.name = (char *) forward->name.c_str();
            // alias
            info->forward.numAlias = forward->alias.size();
            info->forward.aliases = new char *[forward->alias.size()];
            for (size_t i = 0; i < forward->alias.size(); i++) {
                info->forward.aliases[i] = (char *) forward->alias[i].c_str();
            }
            // guestThunk
            info->forward.guestThunk = (char *) forward->guestThunk.c_str();
            // hostThunk
            info->forward.hostThunk = (char *) forward->hostThunk.c_str();
            // hostLibrary
            info->forward.hostLibrary = (char *) forward->hostLibrary.c_str();
        }
        if (reversed) {
            info->hasReversed = true;
            // name
            info->reversed.name = (char *) reversed->name.c_str();
            // alias
            info->forward.numAlias = forward->alias.size();
            info->forward.aliases = new char *[forward->alias.size()];
            for (size_t i = 0; i < forward->alias.size(); i++) {
                info->forward.aliases[i] = (char *) forward->alias[i].c_str();
            }
            // fileName
            info->reversed.fileName = (char *) reversed->fileName.c_str();
            // thunks
            info->reversed.numThunks = reversed->thunks.size();
            info->reversed.thunks = new char *[reversed->thunks.size()];
            for (size_t i = 0; i < reversed->thunks.size(); i++) {
                info->reversed.thunks[i] = (char *) reversed->thunks[i].c_str();
            }
        }
        return info;
    }

    ThunkInfo ThunkInfo::fromCThunkInfo(const CThunkInfo *info) {
        ThunkInfo result;
        if (info->hasForward) {
            result.forward = ForwardThunkInfo();
            result.forward->name = info->forward.name;
            result.forward->alias.resize(info->forward.numAlias);
            for (size_t i = 0; i < info->forward.numAlias; i++) {
                result.forward->alias[i] = info->forward.aliases[i];
            }
            result.forward->guestThunk = info->forward.guestThunk;
            result.forward->hostThunk = info->forward.hostThunk;
            result.forward->hostLibrary = info->forward.hostLibrary;
        }
        if (info->hasReversed) {
            result.reversed = ReversedThunkInfo();
            result.reversed->name = info->reversed.name;
            result.reversed->alias.resize(info->reversed.numAlias);
            for (size_t i = 0; i < info->reversed.numAlias; i++) {
                result.reversed->alias[i] = info->reversed.aliases[i];
            }
            result.reversed->fileName = info->reversed.fileName;
            result.reversed->thunks.resize(info->reversed.numThunks);
            for (size_t i = 0; i < info->reversed.numThunks; i++) {
                result.reversed->thunks[i] = info->reversed.thunks[i];
            }
        }
        return result;
    }

    void ThunkInfo::freeCThunkInfo(CThunkInfo *info) {
        if (info->hasForward) {
            delete[] info->forward.aliases;
        }
        if (info->hasReversed) {
            delete[] info->reversed.aliases;
            delete[] info->reversed.thunks;
        }
        delete info;
    }

    bool ThunkInfoConfig::load(const std::filesystem::path &path,
                               const std::map<std::string, std::string> &vars) {
        std::ifstream file(path);
        if (!file.is_open()) {
            return false;
        }

        std::stringstream ss;
        ss << file.rdbuf();

        std::string err;
        auto json = json11::Json::parse(ss.str(), err);
        if (!err.empty()) {
            return false;
        }
        if (!json.is_object()) {
            return false;
        }

        _forwardThunks.clear();
        _reversedThunks.clear();
        _forwardThunkMap.clear();
        _reversedThunkMap.clear();

        const auto &resolvePath = [&vars](const std::string &path) -> std::string {
            return stdc::str::varexp(path, vars);
        };

        // read thunk data
        const auto &docObj = json.object_items();
        if (auto it0 = docObj.find("forwardThunks");
            it0 != docObj.end() && it0->second.is_array()) {
            const auto &guestItems = it0->second.array_items();
            for (const auto &guestItem : guestItems) {
                if (!guestItem.is_object()) {
                    continue;
                }
                const auto &guestItemObj = guestItem.object_items();
                ForwardThunkInfo result;

                // name
                auto it = guestItemObj.find("name");
                if (it == guestItemObj.end() || !it->second.is_string()) {
                    continue;
                }
                if (auto val = it->second.string_value(); val.empty()) {
                    continue;
                } else {
                    result.name = val;
                }

                // alias
                it = guestItemObj.find("alias");
                if (it != guestItemObj.end() && it->second.is_array()) {
                    const auto &aliasItems = it->second.array_items();
                    for (const auto &aliasItem : aliasItems) {
                        if (!aliasItem.is_string()) {
                            continue;
                        }
                        auto val = aliasItem.string_value();
                        if (val.empty()) {
                            continue;
                        }
                        result.alias.push_back(val);
                    }
                }

                // path
                it = guestItemObj.find("guestThunk");
                if (it == guestItemObj.end() || !it->second.is_string()) {
                    continue;
                }
                if (auto val = it->second.string_value(); val.empty()) {
                    continue;
                } else {
                    result.guestThunk = resolvePath(val);
                }

                // hostThunk
                it = guestItemObj.find("hostThunk");
                if (it == guestItemObj.end() || !it->second.is_string()) {
                    continue;
                }
                if (auto val = it->second.string_value(); val.empty()) {
                    continue;
                } else {
                    result.hostThunk = resolvePath(val);
                }

                // host
                it = guestItemObj.find("hostLibrary");
                if (it == guestItemObj.end() || !it->second.is_string()) {
                    continue;
                }
                if (auto val = it->second.string_value(); val.empty()) {
                    continue;
                } else {
                    result.hostLibrary = resolvePath(val);
                }

                _forwardThunks.push_back(result);
            }
        }

        // read host data
        if (auto it0 = docObj.find("reversedThunks");
            it0 != docObj.end() && it0->second.is_array()) {
            const auto &hostItems = it0->second.array_items();
            for (const auto &hostItem : hostItems) {
                if (!hostItem.is_object()) {
                    continue;
                }
                const auto &hostItemObj = hostItem.object_items();
                ReversedThunkInfo result;

                // name
                auto it = hostItemObj.find("name");
                if (it == hostItemObj.end() || !it->second.is_string()) {
                    continue;
                }
                if (auto val = it->second.string_value(); val.empty()) {
                    continue;
                } else {
                    result.name = val;
                }

                // alias
                it = hostItemObj.find("alias");
                if (it != hostItemObj.end() && it->second.is_array()) {
                    const auto &aliasItems = it->second.array_items();
                    for (const auto &aliasItem : aliasItems) {
                        if (!aliasItem.is_string()) {
                            continue;
                        }
                        auto val = aliasItem.string_value();
                        if (val.empty()) {
                            continue;
                        }
                        result.alias.push_back(val);
                    }
                }

                // fileName
                it = hostItemObj.find("fileName");
                if (it == hostItemObj.end() || !it->second.is_string()) {
                    continue;
                }
                if (auto val = it->second.string_value(); val.empty()) {
                    continue;
                } else {
                    result.fileName = val;
                }

                // dependencies
                it = hostItemObj.find("thunks");
                if (it != hostItemObj.end() && it->second.is_array()) {
                    const auto &items = it->second.array_items();
                    for (const auto &item : items) {
                        if (!item.is_string()) {
                            continue;
                        }
                        auto val = item.string_value();
                        if (val.empty()) {
                            continue;
                        }
                        result.thunks.push_back(val);
                    }
                }

                _reversedThunks.push_back(result);
            }
        }

        // Build indexes
        for (size_t i = 0; i < _forwardThunks.size(); i++) {
            auto &data = _forwardThunks[i];
            _forwardThunkMap[data.name] = i;
            for (const auto &alias : data.alias) {
                _forwardThunkMap[alias] = i;
            }
        }
        for (size_t i = 0; i < _reversedThunks.size(); i++) {
            auto &data = _reversedThunks[i];
            _reversedThunkMap[data.name] = i;
            for (const auto &alias : data.alias) {
                _reversedThunkMap[alias] = i;
            }
        }
        return true;
    }

}