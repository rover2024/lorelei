// SPDX-License-Identifier: MIT

#ifndef LORE_TLCAPI_DIAGNOSTICS_H
#define LORE_TLCAPI_DIAGNOSTICS_H

#include <clang/Basic/SourceLocation.h>
#include <llvm/ADT/Twine.h>

#include <lorelei/TLCApi/Global.h>

namespace clang {
    class DiagnosticsEngine;
}

namespace lore::tool::TLC {

    /// Report a fatal lorelei error on \a de, prefixed with "lorelei: ". \a loc may be invalid for an
    /// error not tied to a source construct. Generate (passes and \c DocumentContext alike) reports
    /// every error this way: \c DiagnosticsEngine::hasErrorOccurred() is the pipeline's single failure
    /// signal, so there is no error-return channel to keep in sync.
    LORETLCAPI_EXPORT void reportError(clang::DiagnosticsEngine &de, clang::SourceLocation loc,
                                       const llvm::Twine &message);

}

#endif // LORE_TLCAPI_DIAGNOSTICS_H
