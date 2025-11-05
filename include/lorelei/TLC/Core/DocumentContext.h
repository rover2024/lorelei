#ifndef TLCCONTEXT_H
#define TLCCONTEXT_H

#include <set>
#include <optional>

#include <lorelei/TLC/ASTMetaItem/MetaConfigItem.h>
#include <lorelei/TLC/ASTMetaItem/MetaProcDescItem.h>
#include <lorelei/TLC/ASTMetaItem/MetaProcItem.h>

#include <lorelei/Utils/Text/ConfigFile.h>
#include <lorelei/TLC/AST/ASTConsumerAddOn.h>
#include <lorelei/TLC/Core/ProcContext.h>
#include <lorelei/TLC/Basic/SourceLineList.h>

namespace clang {
    class CompilerInstance;
}

namespace TLC {

    class LORELIBTLC_EXPORT DocumentContext {
    public:
        DocumentContext() = default;
        ~DocumentContext() = default;

    public:
        /// Called by \c ASTConsumer
        void initialize(clang::ASTContext &ast, const lore::ConfigFile &inputConfig);
        void handleTranslationUnit(clang::ASTContext &ast);

        /// Called by \c ASTFrontendAction
        bool beginSourceFileAction(clang::CompilerInstance &CI);
        void endSourceFileAction();

        /// Called by \c ASTFrontendAction::EndSourceFileAction
        void generateOutput(llvm::raw_ostream &os);

    public:
        struct DocumentSource {
            std::map<std::string, std::string> properties;
            SourceLineList<> head;
            SourceLineList<> tail;
        };

        /// Input data
        const lore::ConfigFile &inputConfig() const {
            return _inputConfig;
        }

        clang::ASTContext *ast() const {
            return _ast;
        }

        /// Result data in source file, valid after \c endSourceFileAction is called.
        llvm::ArrayRef<std::optional<MetaConfigItem>> configs() const {
            return _metaConfigs;
        }
        const std::map<std::string, MetaProcDescItem> &procDescs() const {
            return _metaProcDescs;
        }
        const std::map<std::string, MetaProcCBDescItem> &procCBDescs() const {
            return _metaProcCBDescs;
        }
        const std::map<std::string, MetaProcItem> &procs(CProcKind kind,
                                                         CProcThunkPhase phase) const {
            return _metaProcs[kind][phase];
        }
        const std::map<std::string, MetaProcCBItem> &procCBs(CProcKind kind,
                                                             CProcThunkPhase phase) const {
            return _metaProcCBs[kind][phase];
        }
        const std::map<std::string, const clang::FunctionDecl *> &functions() const {
            return _functions;
        }
        const std::map<std::string, const clang::VarDecl *> &vars() const {
            return _vars;
        }
        const std::map<std::string, const clang::FunctionDecl *> &guestFunctions() const {
            return _guestFunctions;
        }
        const std::map<std::string, clang::QualType> &callbacks() const {
            return _callbacks;
        }

        /// Result metadata
        bool isHost() const {
            return _isHost;
        }

        /// Source code of the document
        const DocumentSource &source() const {
            return _source;
        }
        DocumentSource &source() {
            return _source;
        }
        std::map<std::string, std::unique_ptr<ProcContext>> &procContexts(CProcKind kind) {
            return _procContexts[kind];
        }

    protected:
        // Input data
        lore::ConfigFile _inputConfig;

        // AST data
        clang::ASTContext *_ast = nullptr;

        // Result data
        std::array<std::optional<MetaConfigItem>, MetaConfigItem::User + 1> _metaConfigs;
        std::map<std::string, MetaProcDescItem> _metaProcDescs;
        std::map<std::string, MetaProcCBDescItem> _metaProcCBDescs;
        std::array<std::array<std::map<std::string, MetaProcItem>, CProcThunkPhase_NumThunkPhase>,
                   CProcKind_NumProcKind>
            _metaProcs;
        std::array<std::array<std::map<std::string, MetaProcCBItem>, CProcThunkPhase_NumThunkPhase>,
                   CProcKind_NumProcKind>
            _metaProcCBs;

        std::map<std::string, const clang::FunctionDecl *> _functions;
        std::map<std::string, const clang::VarDecl *> _vars;
        std::map<std::string, const clang::FunctionDecl *> _guestFunctions;
        std::map<std::string, clang::QualType> _callbacks;

        std::set<std::string> _missingFunctions;
        std::set<std::string> _missingVars;

        // Result metadata
        bool _isHost = false;
        std::string _moduleName = "unknown";

        // Source
        DocumentSource _source;
        std::array<std::map<std::string, std::unique_ptr<ProcContext>>, CProcKind_NumProcKind>
            _procContexts;
    };

}

#endif // TLCCONTEXT_H
