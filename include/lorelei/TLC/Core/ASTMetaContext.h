#ifndef ASTMETACONTEXT_H
#define ASTMETACONTEXT_H

#include <lorelei/TLC/ASTMetaItem/MetaConfigItem.h>
#include <lorelei/TLC/ASTMetaItem/MetaProcDescItem.h>
#include <lorelei/TLC/ASTMetaItem/MetaProcItem.h>

#include <lorelei/TLC/Core/ProcContext.h>

namespace TLC {

    class LORELIBTLC_EXPORT ASTMetaContext {
    public:
        ASTMetaContext() = default;
        ~ASTMetaContext() = default;

    public:
        virtual void handleTranslationUnit(clang::ASTContext &ast);

    public:
        /// AST context result, valid after \c handleTranslationUnit is called.
        clang::ASTContext *ast() const {
            return _ast;
        }
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
        const std::map<std::string, const clang::TypedefDecl *> &functionPointerTypedefs() const {
            return _functionPointerTypedefs;
        }

    protected:
        // AST data
        clang::ASTContext *_ast = nullptr;
        clang::ClassTemplateDecl *_metaProcDecl = nullptr;

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
        std::map<std::string, const clang::TypedefDecl *> _functionPointerTypedefs;
    };

}

#endif // ASTMETACONTEXT_H
