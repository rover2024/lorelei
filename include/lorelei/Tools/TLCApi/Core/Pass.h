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
        };

        virtual ~Pass() = default;

        /// Returns all registered pass instances for a given phase, indexed by pass ID.
        static std::map<int, Pass *> &passMap(Phase phase);

        Phase phase() const {
            return m_phase;
        }

        int id() const {
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
        virtual bool testProc(ProcSnippet &proc, std::unique_ptr<ProcMessage> &msg) {
            (void) proc;
            (void) msg;
            return false;
        }

        /// Pre-body pass hook for one proc.
        virtual llvm::Error beginHandleProc(ProcSnippet &proc,
                                            std::unique_ptr<ProcMessage> &msg) = 0;

        /// Post-body pass hook for one proc.
        virtual llvm::Error endHandleProc(ProcSnippet &proc, std::unique_ptr<ProcMessage> &msg) = 0;

    protected:
        Pass(Phase phase, int id) : m_phase(phase), m_id(id) {
        }

        Phase m_phase;
        int m_id;
    };

    using PassRegistry = llvm::Registry<Pass>;

}

#define LORE_TLC_DETAIL_CONCAT_INNER(a, b) a##b
#define LORE_TLC_DETAIL_CONCAT(a, b)       LORE_TLC_DETAIL_CONCAT_INNER(a, b)

/// Registers a TLC pass class into the global LLVM registry.
///
/// Requirements:
/// 1. `PASS_TYPE` derives from `lore::tool::TLC::Pass`.
/// 2. `PASS_TYPE` has a default constructor.
#define LORE_TLC_REGISTER_PASS(PASS_TYPE, NAME, DESC)                                              \
    static ::llvm::Registry<::lore::tool::TLC::Pass>::Add<PASS_TYPE> LORE_TLC_DETAIL_CONCAT(       \
        g_loreTlcPassReg_, __COUNTER__)(NAME, DESC)

#endif // LORE_TOOLS_TLCAPI_PASS_H
