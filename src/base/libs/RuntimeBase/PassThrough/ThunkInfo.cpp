#include "ThunkInfo.h"

#include <fstream>
#include <sstream>

#include <json11/json11.hpp>

#include <LoreBase/CoreLib/ADT/StringExtras.h>

#include "LThunkInfo.h"

namespace lore {

    static inline LString buildString(const std::string &src) {
        LString res;
        res.data = (char *) src.c_str();
        res.size = src.size();
        return res;
    }

    static inline LStringList buildStringList(const std::vector<std::string> &src) {
        LStringList res;
        res.data = new LString[src.size()];
        for (size_t i = 0; i < src.size(); i++) {
            res.data[i] = buildString(src[i]);
        }
        res.size = src.size();
        return res;
    }

    struct ForwardThunkInfo::CDataGuard {
        explicit CDataGuard(const ForwardThunkInfo &info) {
            data.name = (char *) info.name.c_str();
            data.alias = buildStringList(info.alias);
            data.guestThunk = (char *) info.guestThunk.c_str();
            data.hostThunk = (char *) info.hostThunk.c_str();
            data.hostLibrary = (char *) info.hostLibrary.c_str();
        }

        ~CDataGuard() {
            delete[] data.alias.data;
        }

        LForwardThunkInfo data;
    };

    const LForwardThunkInfo &ForwardThunkInfo::c_data() const {
        if (!_c_data) {
            _c_data = std::make_unique<CDataGuard>(*this);
        }
        return _c_data->data;
    }

    ForwardThunkInfo::~ForwardThunkInfo() = default;

    struct ReversedThunkInfo::CDataGuard {
        explicit CDataGuard(const ReversedThunkInfo &info) {
            data.name = (char *) info.name.c_str();
            data.alias = buildStringList(info.alias);
            data.fileName = (char *) info.fileName.c_str();
            data.thunks = buildStringList(info.thunks);
        }

        ~CDataGuard() {
            delete[] data.alias.data;
            delete[] data.thunks.data;
        }

        LReversedThunkInfo data;
    };

    const LReversedThunkInfo &ReversedThunkInfo::c_data() const {
        if (!_c_data) {
            _c_data = std::make_unique<CDataGuard>(*this);
        }
        return _c_data->data;
    }

    ReversedThunkInfo::~ReversedThunkInfo() = default;

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
            return str::varexp(path, vars);
        };

        std::string defaultGuestThunkPath;
        std::string defaultHostThunkPath;
        std::string defaultSysLibPath;
        if (auto it = vars.find("GTL_DIR"); it != vars.end()) {
            defaultGuestThunkPath = it->second;
        }
        if (auto it = vars.find("HTL_DIR"); it != vars.end()) {
            defaultHostThunkPath = it->second;
        }
        if (auto it = vars.find("SYSLIB"); it != vars.end()) {
            defaultSysLibPath = it->second;
        }
        bool allHasDefault = !defaultGuestThunkPath.empty() && !defaultHostThunkPath.empty() &&
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
                        result.guestThunk = defaultGuestThunkPath + "/" + name + ".so";
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
                if (it == forwardObj.end() && !defaultGuestThunkPath.empty()) {
                    result.guestThunk = defaultGuestThunkPath + "/" + result.name + ".so";
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
