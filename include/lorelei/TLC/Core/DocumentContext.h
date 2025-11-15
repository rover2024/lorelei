#ifndef TLCCONTEXT_H
#define TLCCONTEXT_H

#include <set>
#include <optional>

#include <lorelei/TLC/Core/ASTMetaContext.h>
#include <lorelei/TLC/Core/ProcContext.h>
#include <lorelei/TLC/Basic/SourceLineList.h>
#include <lorelei/Utils/Text/ConfigFile.h>

namespace clang {
    class CompilerInstance;
}

namespace TLC {

    class LORELIBTLC_EXPORT DocumentContext : public ASTMetaContext {
    public:
        DocumentContext() = default;
        ~DocumentContext() = default;

    public:
        /// Called by \c ASTConsumer
        void initialize(const lore::ConfigFile &inputConfig);
        void handleTranslationUnit(clang::ASTContext &ast) override;

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

        const std::map<std::string, const clang::FunctionDecl *> &guestFunctions() const {
            return _guestFunctions;
        }
        const std::map<std::string, clang::QualType> &callbacks() const {
            return _callbacks;
        }
        const std::map<std::string, llvm::SmallVector<std::string, 10>> &callbackTraceInfos() const {
            return _callbackTraceInfos;
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

        std::map<std::string, const clang::FunctionDecl *> _guestFunctions;
        std::map<std::string, clang::QualType> _callbacks;
        std::map<std::string, llvm::SmallVector<std::string, 10>> _callbackTraceInfos;

        std::set<std::string> _missingFunctions;
        std::set<std::string> _missingVars;
        std::set<std::string> _missingCallbacks;

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
