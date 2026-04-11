#include "ManifestStatistics.h"

#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/JSON.h>

namespace lore::tool::TLC {

    void ManifestStatistics::addFunction(FunctionDirection direction, std::string name,
                                         std::string signature, std::string location) {
        auto &bucket = functions[direction];
        auto &info = bucket[name];
        info.signature = std::move(signature);
        info.location = std::move(location);
    }

    void ManifestStatistics::addCallbackSignature(const std::string &signature,
                                                  const std::string &origin,
                                                  const std::string &preferredAlias) {
        auto &info = callbacks[signature];
        info.signature = signature;
        if (!preferredAlias.empty() && info.alias.empty()) {
            info.alias = preferredAlias;
        }
        if (!origin.empty() && info.origin.empty()) {
            info.origin = origin;
        }
    }

    bool ManifestStatistics::loadFromJson(const std::string &filePath, std::string &errorMessage) {
        auto buffer = llvm::MemoryBuffer::getFile(filePath);
        if (std::error_code ec = buffer.getError()) {
            errorMessage = ec.message();
            return false;
        }

        auto json = llvm::json::parse(buffer.get()->getBuffer());
        if (!json) {
            errorMessage = llvm::toString(json.takeError());
            return false;
        }

        auto obj = json.get().getAsObject();
        if (!obj) {
            errorMessage = "not an object";
            return false;
        }

        std::string loadedFileName;
        std::array<std::map<std::string, FunctionInfo>, NumFunctionDirection> loadedFunctions;
        std::map<std::string, CallbackInfo> loadedCallbacks;

        if (auto value = obj->getString("fileName")) {
            loadedFileName = value->str();
        }

        if (auto functionsObj = obj->getObject("functions")) {
            const auto parseFunctionArray = [&](FunctionDirection dir, const char *key) {
                auto arr = functionsObj->getArray(key);
                if (!arr) {
                    return;
                }
                for (const auto &item : *arr) {
                    auto itemObj = item.getAsObject();
                    if (!itemObj) {
                        continue;
                    }
                    auto name = itemObj->getString("name");
                    auto signature = itemObj->getString("signature");
                    auto location = itemObj->getString("location");
                    if (!name || !signature) {
                        continue;
                    }

                    auto &info = loadedFunctions[dir][name->str()];
                    info.signature = signature->str();
                    info.location = location ? location->str() : "";
                }
            };

            parseFunctionArray(GuestToHost, "GuestToHost");
            parseFunctionArray(HostToGuest, "HostToGuest");
        }

        if (auto callbackArray = obj->getArray("callbacks")) {
            for (const auto &item : *callbackArray) {
                auto itemObj = item.getAsObject();
                if (!itemObj) {
                    continue;
                }
                auto signature = itemObj->getString("signature");
                if (!signature) {
                    continue;
                }

                auto &info = loadedCallbacks[signature->str()];
                info.signature = signature->str();
                if (auto alias = itemObj->getString("alias")) {
                    info.alias = alias->str();
                }
                if (auto origin = itemObj->getString("origin")) {
                    info.origin = origin->str();
                }
            }
        }

        fileName = std::move(loadedFileName);
        functions = std::move(loadedFunctions);
        callbacks = std::move(loadedCallbacks);
        return true;
    }

    bool ManifestStatistics::saveAsJson(const std::string &filePath, std::string &errorMessage) const {
        llvm::json::Object root;
        root["version"] = 1;
        root["fileName"] = fileName;

        const auto serializeFunctionMap = [](const auto &map) {
            llvm::json::Array array;
            for (const auto &pair : map) {
                const auto &name = pair.first;
                const auto &info = pair.second;
                llvm::json::Object obj;
                obj["name"] = name;
                obj["signature"] = info.signature;
                obj["location"] = info.location;
                array.push_back(std::move(obj));
            }
            return array;
        };

        llvm::json::Object functionsObj;
        functionsObj["GuestToHost"] = serializeFunctionMap(functions[GuestToHost]);
        functionsObj["HostToGuest"] = serializeFunctionMap(functions[HostToGuest]);
        root["functions"] = std::move(functionsObj);

        llvm::json::Array callbackArray;
        for (const auto &pair : callbacks) {
            const auto &info = pair.second;
            llvm::json::Object obj;
            obj["signature"] = info.signature;
            obj["alias"] = info.alias;
            obj["origin"] = info.origin;
            callbackArray.push_back(std::move(obj));
        }
        root["callbacks"] = std::move(callbackArray);

        std::error_code ec;
        llvm::raw_fd_ostream os(filePath, ec, llvm::sys::fs::OF_Text);
        if (ec) {
            errorMessage = ec.message();
            return false;
        }

        llvm::json::OStream jsonStream(os, 4);
        jsonStream.value(std::move(root));

        if (os.has_error()) {
            errorMessage = "Failed to write JSON to file";
            return false;
        }
        return true;
    }

}
