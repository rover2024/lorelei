// SPDX-License-Identifier: MIT

#ifndef LORE_TLCAPI_PASS_H
#define LORE_TLCAPI_PASS_H

#include <map>
#include <memory>
#include <string>

#include <llvm/Support/Error.h>
#include <llvm/Support/Registry.h>

#include <lorelei/TLCApi/ProcSnippet.h>
#include <lorelei/TLCApi/Global.h>

namespace lore::tool::TLC {

    class DocumentContext;

    /// PassMessage - Per-proc scratch state a pass carries between its own hooks.
    ///
    /// A pass may create a subclass instance in \c testProc and read it back in \c beginHandleProc
    /// and \c endHandleProc for the same proc. Subclass it to hold whatever a pass needs.
    class PassMessage {
    public:
        virtual ~PassMessage() = default;
    };

    /// Pass - Base class of a TLC generate pass.
    ///
    /// A pass transforms one document's procs during \c generate. The pipeline drives each pass
    /// through this lifecycle:
    /// 1. \c handleTranslationUnit
    /// 2. \c beginProcessDocument
    /// 3. per proc: \c testProc -> \c beginHandleProc -> \c endHandleProc
    /// 4. \c endProcessDocument
    ///
    /// Passes register themselves with \c llvm::Registry<Pass> and are grouped by \c Phase.
    ///
    /// Error reporting. A pass does not return errors. It reports them through the Clang
    /// \c DiagnosticsEngine, reached via \c proc.document().ast().getDiagnostics() (or
    /// \c doc.ast().getDiagnostics() in the document-level hooks) -- the same engine that carries
    /// the parse's own errors. Emit one with \c Error severity, located at the offending
    /// declaration when there is one. The pipeline checks \c DiagnosticsEngine::hasErrorOccurred()
    /// after every hook and stops, so a hook should return promptly once it has reported. This is
    /// why every hook returns
    /// \c void: \c hasErrorOccurred() is the single failure signal, and there is no second error
    /// channel to keep in sync.
    class LORETLCAPI_EXPORT Pass {
    public:
        /// The pipeline stage a pass runs in. Stages run in this order.
        enum Phase {
            Builder,
            Guard,
            Misc,
            NumPhases,
        };

        virtual ~Pass() = default;

        /// The phase this pass runs in.
        inline Phase phase() const {
            return m_phase;
        }

        /// The pass's id, unique within its phase.
        inline int id() const {
            return m_id;
        }

        /// A short human-readable name, used in diagnostics.
        virtual std::string name() const = 0;

        /// Hook run while the translation unit AST is being handled, before any proc is processed.
        /// A good place for document-scope setup (e.g. resolving a type the pass relies on); report
        /// errors via \c doc.ast().getDiagnostics() (see the class note).
        virtual void handleTranslationUnit(DocumentContext &doc) {
            (void) doc;
        }

        /// Hook run before the document's procs are processed. Report errors via the document's
        /// \c DiagnosticsEngine (see the class note).
        virtual void beginProcessDocument(DocumentContext &doc) {
            (void) doc;
        }

        /// Hook run after the document's procs are processed. Report errors via the document's
        /// \c DiagnosticsEngine (see the class note).
        virtual void endProcessDocument(DocumentContext &doc) {
            (void) doc;
        }

        /// Decides whether this pass should run on \a proc. May stash a \c PassMessage in \a msg
        /// for the handle hooks below. A predicate only -- report problems from the handle hooks,
        /// which can locate them on the proc.
        virtual bool testProc(ProcSnippet &proc, std::unique_ptr<PassMessage> &msg);

        /// Pre-body hook for one proc; runs before the proc's body is generated. Report errors via
        /// the document's \c DiagnosticsEngine (see the class note).
        virtual void beginHandleProc(ProcSnippet &proc, std::unique_ptr<PassMessage> &msg) = 0;

        /// Post-body hook for one proc; runs after the proc's body is generated. Report errors via
        /// the document's \c DiagnosticsEngine (see the class note).
        virtual void endHandleProc(ProcSnippet &proc, std::unique_ptr<PassMessage> &msg) = 0;

    protected:
        /// Binds the pass to its \a phase and \a id.
        inline Pass(Phase phase, int id) : m_phase(phase), m_id(id) {
        }

        Phase m_phase;
        int m_id;
    };

}

#endif // LORE_TLCAPI_PASS_H
