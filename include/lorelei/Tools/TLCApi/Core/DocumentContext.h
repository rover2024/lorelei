#ifndef LORE_TOOLS_TLCAPI_DOCUMENTCONTEXT_H
#define LORE_TOOLS_TLCAPI_DOCUMENTCONTEXT_H

#include <map>
#include <string>
#include <vector>

#include <lorelei/Tools/ToolUtils/SourceLineList.h>
#include <lorelei/Tools/TLCApi/Global.h>
#include <lorelei/Tools/TLCApi/Utils/ManifestStatistics.h>
#include <lorelei/Tools/TLCApi/Core/ProcContext.h>

namespace lore::tool::TLC {

    /// DocumentContext - Shared mutable state for a single TLC `generate` execution.
    class LORETLCAPI_EXPORT DocumentContext {
    public:
        enum GenerateMode {
            Guest = 0,
            Host = 1,
        };

        struct DocumentSource {
            std::map<std::string, std::string> properties;
            SourceLineList<> head;
            SourceLineList<> tail;
        };

        void clear() {
            fileName.clear();
            mode = Guest;
            manifestStat = nullptr;
            procs.clear();
            source = {};
        }

        std::string fileName;
        GenerateMode mode = Guest;
        const ManifestStatistics *manifestStat = nullptr;
        std::vector<ProcContext> procs;
        DocumentSource source;
    };

}

#endif // LORE_TOOLS_TLCAPI_DOCUMENTCONTEXT_H
