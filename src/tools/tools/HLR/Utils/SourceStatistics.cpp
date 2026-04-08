#include "SourceStatistics.h"

#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/JSON.h>
#include <llvm/Support/FileSystem.h>

namespace lore::tool::HLR {

    bool SourceStatistics::loadFromJson(const std::string &filePath, std::string &errorMessage) {
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

        std::set<std::string> callbackCheckGuardSignatures;
        std::map<std::string, FunctionDecayGuardData> functionDecayGuardStats;

        auto CCGs = obj->getArray("callbackCheckGuardSignatures");
        if (CCGs) {
            for (auto &CCG : *CCGs) {
                if (auto str = CCG.getAsString()) {
                    callbackCheckGuardSignatures.insert(str->str());
                }
            }
        }

        auto FDGs = obj->getArray("functionDecayGuardStats");
        if (FDGs) {
            for (auto &FDG : *FDGs) {
                if (auto obj = FDG.getAsObject()) {
                    std::string signature;
                    if (auto sig = obj->getString("signature")) {
                        signature = sig->str();
                    } else {
                        continue;
                    }

                    FunctionDecayGuardData data;
                    if (auto locObj = obj->getObject("locations")) {
                        for (auto &loc : *locObj) {
                            if (auto locStr = loc.second.getAsString()) {
                                data.locations.insert(
                                    std::make_pair(loc.first.str(), locStr->str()));
                            }
                        }
                    }
                    if (data.locations.empty()) {
                        continue;
                    }
                    functionDecayGuardStats.insert(std::make_pair(signature, data));
                }
            }
        }

        this->callbackCheckGuardSignatures = std::move(callbackCheckGuardSignatures);
        this->functionDecayGuardStats = std::move(functionDecayGuardStats);
        return true;
    }

    bool SourceStatistics::saveAsJson(const std::string &filePath, std::string &errorMessage) {
        llvm::json::Object root;

        llvm::json::Array callbackGuardsArray;
        for (const auto &signature : callbackCheckGuardSignatures) {
            callbackGuardsArray.push_back(signature);
        }
        root["callbackCheckGuardSignatures"] = std::move(callbackGuardsArray);

        llvm::json::Array decayGuardsArray;
        for (const auto &pair : functionDecayGuardStats) {
            const std::string &signature = pair.first;
            const FunctionDecayGuardData &data = pair.second;

            llvm::json::Object decayGuardObj;
            decayGuardObj["signature"] = signature;

            llvm::json::Object locationsObj;
            for (const auto &location : data.locations) {
                locationsObj[location.first] = location.second;
            }
            decayGuardObj["locations"] = std::move(locationsObj);

            decayGuardsArray.push_back(std::move(decayGuardObj));
        }
        root["functionDecayGuardStats"] = std::move(decayGuardsArray);

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
