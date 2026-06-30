// SPDX-License-Identifier: MIT

#include <lorelei/TLCApi/Diagnostics.h>

#include <clang/Basic/Diagnostic.h>

namespace lore::tool::TLC {

    void reportError(clang::DiagnosticsEngine &de, clang::SourceLocation loc,
                     const llvm::Twine &message) {
        de.Report(loc, de.getCustomDiagID(clang::DiagnosticsEngine::Error, "lorelei: %0"))
            << message.str();
    }

}
