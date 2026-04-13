#ifndef LORE_TOOLS_TLCAPI_PASS_H
#define LORE_TOOLS_TLCAPI_PASS_H

#include <map>
#include <memory>
#include <string>

#include <llvm/Support/Error.h>
#include <llvm/Support/Registry.h>

#include <lorelei/Tools/TLCApi/Core/ProcSnippet.h>
#include <lorelei/Tools/TLCApi/Global.h>

namespace lore::tool::TLC {

    class DocumentContext;

    /// PassMessage - Per-pass temporary data exchanged by pass hooks.
    class PassMessage {
    public:
        virtual ~PassMessage() = default;
    };

    /// Pass - Base class of TLC passes.
    ///
    /// Execution lifecycle follows the v1 behavior:
    /// 1. `handleTranslationUnit`
    /// 2. `beginProcessDocument`
    /// 3. per-proc: `testProc` -> `beginHandleProc` -> `endHandleProc`
    /// 4. `endProcessDocument`
    class LORETLCAPI_EXPORT Pass {
    public:
        enum Phase {
            Builder,
            Guard,
            Misc,
            NumPhases,
        };

        virtual ~Pass() = default;

        inline Phase phase() const {
            return m_phase;
        }

        inline int id() const {
            return m_id;
        }

        virtual std::string name() const = 0;

        /// Called while handling translation unit AST.
        virtual void handleTranslationUnit(DocumentContext &doc) {
            (void) doc;
        }

        /// Called before processing procs in the current document.
        virtual void beginProcessDocument(DocumentContext &doc) {
            (void) doc;
        }

        /// Called after processing procs in the current document.
        virtual void endProcessDocument(DocumentContext &doc) {
            (void) doc;
        }

        /// Determines whether this pass should run on `proc`.
        virtual bool testProc(ProcSnippet &proc, std::unique_ptr<PassMessage> &msg);

        /// Pre-body pass hook for one proc.
        virtual llvm::Error beginHandleProc(ProcSnippet &proc,
                                            std::unique_ptr<PassMessage> &msg) = 0;

        /// Post-body pass hook for one proc.
        virtual llvm::Error endHandleProc(ProcSnippet &proc, std::unique_ptr<PassMessage> &msg) = 0;

    protected:
        inline Pass(Phase phase, int id) : m_phase(phase), m_id(id) {
        }

        Phase m_phase;
        int m_id;
    };

}

#endif // LORE_TOOLS_TLCAPI_PASS_H
