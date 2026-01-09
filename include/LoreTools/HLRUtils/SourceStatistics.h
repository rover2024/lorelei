// SPDX-License-Identifier: MIT

#ifndef LORE_TOOLS_HLRUTILS_SOURCESTATISTICS_H
#define LORE_TOOLS_HLRUTILS_SOURCESTATISTICS_H

#include <string>
#include <map>
#include <set>

#include <LoreTools/HLRUtils/Global.h>

namespace lore::tool::HLR {

    /// SourceStatistics - Statistics of callback check guards and function decay guards from
    /// a set of source files.
    class LOREHLRUTILS_EXPORT SourceStatistics {
    public:
        struct FunctionDecayGuardData {
            /// Each location is map from the string representation of the begin source location of
            /// a function declaration to the last referenced file name.
            std::map<std::string, std::string> locations;
        };

        SourceStatistics() = default;

    public:
        bool loadFromJson(const std::string &filePath, std::string &errorMessage);
        bool saveAsJson(const std::string &filePath, std::string &errorMessage);

        std::set<std::string> callbackCheckGuardSignatures;
        std::map<std::string, FunctionDecayGuardData> functionDecayGuardStats;
    };

}

#endif // LORE_TOOLS_HLRUTILS_SOURCESTATISTICS_H
