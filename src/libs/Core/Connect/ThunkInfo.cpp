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

        const auto &resolvePath = [&](const std::string &path) -> std::string {
            return stdc::str::varexp(path, vars);
        };

        std::string defualtGuestThunkPath;
        std::string defaultHostThunkPath;
        std::string defaultSysLibPath;
        if (auto it = vars.find("GTL_DIR"); it != vars.end()) {
            defualtGuestThunkPath = it->second;
        }
        if (auto it = vars.find("HTL_DIR"); it != vars.end()) {
            defaultHostThunkPath = it->second;
        }
        if (auto it = vars.find("SYSLIB"); it != vars.end()) {
            defaultSysLibPath = it->second;
        }
        bool allHasDefault = !defualtGuestThunkPath.empty() && !defaultHostThunkPath.empty() &&
                             !defaultSysLibPath.empty();

        // read thunk data
        const auto &docObj = json.object_items();
        if (auto it0 = docObj.find("forwardThunks");
            it0 != docObj.end() && it0->second.is_array()) {
            const auto &forwardItems = it0->second.array_items();
            for (const auto &forwardItem : forwardItems) {
                if (forwardItem.is_string()) {
                    std::string name = forwardItem.string_value();
                    if (name.empty()) {
                        continue;
                    }

                    if (allHasDefault) {
                        ForwardThunkInfo result;
                        result.name = name;
                        result.guestThunk = defualtGuestThunkPath + "/" + name + ".so";
                        result.hostThunk = defaultHostThunkPath + "/" + name + "_HTL.so";
                        result.hostLibrary = defaultSysLibPath + "/" + name + ".so";
                        _forwardThunks.push_back(result);
                    }
                }

                if (!forwardItem.is_object()) {
                    continue;
                }
                const auto &forwardObj = forwardItem.object_items();
                ForwardThunkInfo result;

                // name
                auto it = forwardObj.find("name");
                if (it == forwardObj.end() || !it->second.is_string()) {
                    continue;
                }
                if (auto val = it->second.string_value(); val.empty()) {
                    continue;
                } else {
                    result.name = val;
                }

                // alias
                it = forwardObj.find("alias");
                if (it != forwardObj.end() && it->second.is_array()) {
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

                // guestThunk
                it = forwardObj.find("guestThunk");
                if (it == forwardObj.end() && !defualtGuestThunkPath.empty()) {
                    result.guestThunk = defualtGuestThunkPath + "/" + result.name + ".so";
                } else {
                    if (!it->second.is_string()) {
                        continue;
                    }
                    if (auto val = it->second.string_value(); val.empty()) {
                        continue;
                    } else {
                        result.guestThunk = resolvePath(val);
                    }
                }

                // hostThunk
                it = forwardObj.find("hostThunk");
                if (it == forwardObj.end() && !defaultHostThunkPath.empty()) {
                    result.hostThunk = defaultHostThunkPath + "/" + result.name + "_HTL.so";
                } else {
                    if (!it->second.is_string()) {
                        continue;
                    }
                    if (auto val = it->second.string_value(); val.empty()) {
                        continue;
                    } else {
                        result.hostThunk = resolvePath(val);
                    }
                }

                // hostLibrary
                it = forwardObj.find("hostLibrary");
                if (it == forwardObj.end() && !defaultSysLibPath.empty()) {
                    result.hostLibrary = defaultSysLibPath + "/" + result.name + ".so";
                } else {
                    if (!it->second.is_string()) {
                        continue;
                    }
                    if (auto val = it->second.string_value(); val.empty()) {
                        continue;
                    } else {
                        result.hostLibrary = resolvePath(val);
                    }
                }
                _forwardThunks.push_back(result);
            }
        }

        // read host data
        if (auto it0 = docObj.find("reversedThunks");
            it0 != docObj.end() && it0->second.is_array()) {
            const auto &reversedItems = it0->second.array_items();
            for (const auto &reversedItem : reversedItems) {
                if (!reversedItem.is_object()) {
                    continue;
                }
                const auto &reversedObj = reversedItem.object_items();
                ReversedThunkInfo result;

                // name
                auto it = reversedObj.find("name");
                if (it == reversedObj.end() || !it->second.is_string()) {
                    continue;
                }
                if (auto val = it->second.string_value(); val.empty()) {
                    continue;
                } else {
                    result.name = val;
                }

                // alias
                it = reversedObj.find("alias");
                if (it != reversedObj.end() && it->second.is_array()) {
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
                it = reversedObj.find("fileName");
                if (it == reversedObj.end() || !it->second.is_string()) {
                    continue;
                }
                if (auto val = it->second.string_value(); val.empty()) {
                    continue;
                } else {
                    result.fileName = val;
                }

                // dependencies
                it = reversedObj.find("thunks");
                if (it != reversedObj.end() && it->second.is_array()) {
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