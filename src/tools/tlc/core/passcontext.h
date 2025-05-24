#ifndef PASSCONTEXT_H
#define PASSCONTEXT_H

#include <set>

#include <clang/AST/AST.h>

#include "core/apisource.h"

namespace TLC {

    class PassContext {
    public:
        PassContext(const std::set<std::string> &FunctionNames,                       //
                    const llvm::ArrayRef<const clang::FunctionDecl *> &FunctionDecls, //
                    clang::SourceManager *SourceMgr,                                  //
                    const clang::LangOptions &LangOpts,                               //
                    clang::ASTContext *Context,                                       //
                    llvm::StringRef InputFileName,                                    //
                    llvm::StringRef HeaderFileName,                                   //
                    llvm::StringRef SourceFileName                                    //
        );
        ~PassContext();

        /// AST
        clang::SourceManager &getSourceManager() const;
        const clang::LangOptions &getLangOpts() const;
        clang::ASTContext &getASTContext() const;

        /// Children
        std::set<std::string> getApiNameSet(ApiSource::Type type) const;
        ApiSource *getApiSource(ApiSource::Type type, const std::string &name);

    public:
        struct ResultText {
            llvm::StringRef Defs;
            llvm::StringRef Impl;
        };
        ResultText Run();

    protected:
        class Impl;
        std::unique_ptr<Impl> _impl;
    };

}

#endif // PASSCONTEXT_H
