#ifndef LORE_TOOLS_HLR_SOURCESTATISTICS_H
#define LORE_TOOLS_HLR_SOURCESTATISTICS_H

#include <string>
#include <map>
#include <set>

namespace lore::tool::HLR {

    /// SourceStatistics - Statistics of callback check guards and function decay guards from
    /// a set of source files.
    ///
    /// \example Sample `stat.json`
    /// \code
    /// {
    ///   "callbackCheckGuardSignatures": [
    ///     "void (*)(void *)",
    ///     "void (*)(void *, void *)"
    ///   ],
    ///   "functionDecayGuardStats": [
    ///     {
    ///       "signature": "void (*)(void *)",
    ///       "locations": {
    ///         "foo.h<1:1>": "a.c",
    ///         "bar.h<2:1>": "b.c"
    ///       }
    ///     }
    ///   ]
    /// }
    /// \endcode
    class SourceStatistics {
    public:
        struct FunctionDecayGuardData {
            /// Each location is map from the string representation of the begin source location of
            /// a function declaration to the last referenced file name.
            std::map<std::string, std::string> locations;
        };

        SourceStatistics() = default;

    public:
        inline void clear() {
            callbackCheckGuardSignatures.clear();
            functionDecayGuardStats.clear();
        }

        bool loadFromJson(const std::string &filePath, std::string &errorMessage);
        bool saveAsJson(const std::string &filePath, std::string &errorMessage);

        std::set<std::string> callbackCheckGuardSignatures;
        std::map<std::string, FunctionDecayGuardData> functionDecayGuardStats;
    };

}

#endif // LORE_TOOLS_HLR_SOURCESTATISTICS_H
