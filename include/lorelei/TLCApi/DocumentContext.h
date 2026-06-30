// SPDX-License-Identifier: MIT

#ifndef LORE_TLCAPI_DOCUMENTCONTEXT_H
#define LORE_TLCAPI_DOCUMENTCONTEXT_H

#include <map>
#include <string>
#include <cassert>
#include <set>
#include <array>
#include <vector>
#include <memory>

#include <lorelei/ClangExtras/SourceLineList.h>
#include <lorelei/TLCApi/ProcSnippet.h>
#include <lorelei/TLCApi/Pass.h>
#include <lorelei/TLCApi/Global.h>

namespace clang {
    class CompilerInstance;
}

namespace lore::tool::TLC {

    class ManifestSummary;
    class Pass;
    class ProcAliasMaker;

    /// DocumentContext - The mutable state and pipeline driver for one TLC `generate` invocation.
    ///
    /// It owns everything that lives for the span of a single document: the input metadata (mode,
    /// source paths, and the set of requested procs), the AST data collected from the translation
    /// unit, the ProcSnippet models the passes operate on, and the document-level source fragments
    /// the pipeline emits. The begin/handle/end hooks carry one translation unit through collection
    /// and the pass pipeline; generateOutput() then serializes the result.
    class LORETLCAPI_EXPORT DocumentContext {
    public:
        /// The side a manifest is generated for.
        enum Mode {
            Guest,
            Host,
        };

        /// The procs the caller asked TLC to generate; used to filter AST collection.
        struct RequestedProcData {
            std::array<std::set<std::string>, ProcSnippet::NumProcKind> functions;
            std::map<std::string /* signature */, std::string /* name */> callbacks;
        };

        /// Document-level generated source: free-form properties plus head/tail line buffers.
        struct DocumentSource {
            std::map<std::string, std::string> properties;
            SourceLineList<> head;
            SourceLineList<> tail;
        };

        /// A resolved callback type together with its preferred alias name.
        struct FunctionPointerTypeInfo {
            clang::QualType type;
            std::string name;
        };

        DocumentContext();
        ~DocumentContext();

        /// Stores the per-invocation inputs before the pipeline runs.
        inline void initialize(Mode mode, std::string preIncludeFileName, std::string mainFileName,
                               RequestedProcData interestedProcData) {
            m_mode = mode;
            m_preIncludeFileName = std::move(preIncludeFileName);
            m_mainFileName = std::move(mainFileName);
            m_requestedProcData = std::move(interestedProcData);
        }

        /// Instantiates the pass pipeline for this source file; a returned error aborts the action.
        void beginSourceFileAction(clang::CompilerInstance &CI);

        /// Collects the requested procs and their metadata from the parsed AST.
        void handleTranslationUnit(clang::ASTContext &ast);

        /// Runs the Builder/Guard/Misc passes over the collected procs.
        void endSourceFileAction();

        /// Serializes the generated thunk translation unit into \c os.
        void generateOutput(llvm::raw_ostream &os);

    public:
        inline Mode mode() const {
            return m_mode;
        }
        inline std::string preIncludeFileName() const {
            return m_preIncludeFileName;
        }
        inline std::string mainFileName() const {
            return m_mainFileName;
        }
        inline clang::ASTContext &ast() const {
            assert(m_ast != nullptr);
            return *m_ast;
        }

        /// Metadata collected from the translation unit.
        inline const std::map<std::string, const clang::FunctionDecl *> &
            functionDecls(ProcSnippet::Direction direction) const {
            return m_functionDecls[direction];
        }
        inline const std::map<std::string, FunctionPointerTypeInfo> &callbackTypes() const {
            return m_callbackTypes;
        }

        /// The document-level generation buffer.
        inline DocumentSource &source() {
            return m_source;
        }
        inline const DocumentSource &source() const {
            return m_source;
        }

        /// The collected procs, indexed by kind and direction.
        inline std::map<std::string, ProcSnippet> &procs(ProcSnippet::Kind kind,
                                                         ProcSnippet::Direction direction) {
            return m_procs[kind][direction];
        }
        inline const std::map<std::string, ProcSnippet> &
            procs(ProcSnippet::Kind kind, ProcSnippet::Direction direction) const {
            return m_procs[kind][direction];
        }

    protected:
        // generateOutput helpers; each appends one section of the generated TU to `os`.
        void emitManifestPrologue(llvm::raw_ostream &os) const;
        void emitExportedAliases(llvm::raw_ostream &os) const;
        void emitForeachMacros(llvm::raw_ostream &os) const;
        void emitProcTexts(llvm::raw_ostream &os, bool asDeclaration) const;
        void emitMissingComments(llvm::raw_ostream &os) const;

        Mode m_mode = Guest;
        std::string m_preIncludeFileName;
        std::string m_mainFileName;
        RequestedProcData m_requestedProcData;

        // AST data
        clang::ASTContext *m_ast = nullptr;
        std::array<std::map<std::string, const clang::FunctionDecl *>, ProcSnippet::NumProcDirection>
            m_functionDecls;
        std::map<std::string, FunctionPointerTypeInfo> m_callbackTypes;

        // Produced by the passes.
        DocumentSource m_source;
        std::array<std::array<std::map<std::string, ProcSnippet>, ProcSnippet::NumProcDirection>,
                   ProcSnippet::NumProcKind>
            m_procs;

        // Helpers
        std::unique_ptr<ProcAliasMaker> m_procAliasMaker;

        // Cached pass instances
        std::array<std::map<int, Pass *>, Pass::NumPhases> m_passMaps;
        std::vector<std::unique_ptr<Pass>> m_passInstances;
    };

}

#endif // LORE_TLCAPI_DOCUMENTCONTEXT_H
