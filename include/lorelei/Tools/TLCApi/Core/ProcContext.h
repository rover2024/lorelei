#ifndef LORE_TOOLS_TLCAPI_PROCCONTEXT_H
#define LORE_TOOLS_TLCAPI_PROCCONTEXT_H

#include <string>
#include <vector>

#include <lorelei/Tools/TLCApi/Utils/ManifestStatistics.h>
#include <lorelei/Tools/TLCApi/Global.h>

namespace lore::tool::TLC {

    /// ProcContext - Minimal procedure context used by the TLC generate phase.
    class LORETLCAPI_EXPORT ProcContext {
    public:
        enum Kind {
            Function = 0,
            Callback = 1,
        };

        Kind kind = Function;
        ManifestStatistics::FunctionDirection direction = ManifestStatistics::GuestToHost;
        std::string name;
        std::string signature;
        std::string alias;
        std::string location;
        int builderPassID = -1;
        std::vector<int> passIDs;

        bool isFunction() const {
            return kind == Function;
        }

        bool isCallback() const {
            return kind == Callback;
        }

        bool hasPassID(int passID) const {
            if (passID < 0) {
                return false;
            }
            if (builderPassID == passID) {
                return true;
            }
            for (int id : passIDs) {
                if (id == passID) {
                    return true;
                }
            }
            return false;
        }
    };

}

#endif // LORE_TOOLS_TLCAPI_PROCCONTEXT_H
