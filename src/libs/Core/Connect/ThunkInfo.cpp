#include "ThunkInfo.h"

#include <fstream>

#include <json11.hpp>

#include <stdcorelib/str.h>

#include <lorelei-c/Core/ThunkInfo_c.h>

namespace lore {

    const CForwardThunkInfo &ForwardThunkInfo::cinfo() const {
        if (!_cinfo.has_value()) {
            _cinfo = CForwardThunkInfo{
                .name = (char *) name.c_str(),
                .numAlias = alias.size(),
                .aliases = new char *[alias.size()],
                .guestThunk = (char *) guestThunk.c_str(),
                .hostThunk = (char *) hostThunk.c_str(),
                .hostLibrary = (char *) hostLibrary.c_str(),
            };
            for (size_t i = 0; i < alias.size(); i++) {
                _cinfo->aliases[i] = (char *) alias[i].c_str();
            }
        }
        return _cinfo.value();
    }

    ForwardThunkInfo::~ForwardThunkInfo() {
        if (_cinfo.has_value()) {
            delete[] _cinfo->aliases;
        }
    }

    const CReversedThunkInfo &ReversedThunkInfo::cinfo() const {
        if (!_cinfo.has_value()) {
            _cinfo = CReversedThunkInfo{
                .name = (char *) name.c_str(),
                .numAlias = alias.size(),
                .aliases = new char *[alias.size()],
                .fileName = (char *) fileName.c_str(),
                .numThunks = thunks.size(),
                .thunks = new char *[thunks.size()],
            };
            for (size_t i = 0; i < alias.size(); i++) {
                _cinfo->aliases[i] = (char *) alias[i].c_str();
            }
            for (size_t i = 0; i < thunks.size(); i++) {
                _cinfo->thunks[i] = (char *) thunks[i].c_str();
            }
        }
        return _cinfo.value();
    }

    ReversedThunkInfo::~ReversedThunkInfo() {
        if (_cinfo.has_value()) {
            delete[] _cinfo->aliases;
            delete[] _cinfo->thunks;
        }
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