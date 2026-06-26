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
        virtual void handleTranslationUnit(DocumentContext &doc) {
            (void) doc;
        }

        /// Hook run before the document's procs are processed.
        virtual void beginProcessDocument(DocumentContext &doc) {
            (void) doc;
        }

        /// Hook run after the document's procs are processed.
        virtual void endProcessDocument(DocumentContext &doc) {
            (void) doc;
        }

        /// Decides whether this pass should run on \a proc. May stash a \c PassMessage in \a msg
        /// for the handle hooks below.
        virtual bool testProc(ProcSnippet &proc, std::unique_ptr<PassMessage> &msg);

        /// Pre-body hook for one proc; runs before the proc's body is generated.
        virtual llvm::Error beginHandleProc(ProcSnippet &proc,
                                            std::unique_ptr<PassMessage> &msg) = 0;

        /// Post-body hook for one proc; runs after the proc's body is generated.
        virtual llvm::Error endHandleProc(ProcSnippet &proc, std::unique_ptr<PassMessage> &msg) = 0;

    protected:
        /// Binds the pass to its \a phase and \a id.
        inline Pass(Phase phase, int id) : m_phase(phase), m_id(id) {
        }

        Phase m_phase;
        int m_id;
    };

}

#endif // LORE_TLCAPI_PASS_H
