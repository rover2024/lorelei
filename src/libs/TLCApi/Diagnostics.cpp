// SPDX-License-Identifier: MIT

#include <lorelei/TLCApi/Diagnostics.h>

#include <clang/Basic/Diagnostic.h>

namespace lore::tool::TLC {

    void reportError(clang::DiagnosticsEngine &DE, clang::SourceLocation loc,
                     const llvm::Twine &message) {
        DE.Report(loc, DE.getCustomDiagID(clang::DiagnosticsEngine::Error, "LoreTLC: %0"))
            << message.str();
    }

}
