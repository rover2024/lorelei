// SPDX-License-Identifier: MIT

#ifndef LORE_TOOLS_HLRUTILS_ASTCONSUMERS_H
#define LORE_TOOLS_HLRUTILS_ASTCONSUMERS_H

#include <clang/AST/ASTConsumer.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>

#include <LoreTools/HLRUtils/Global.h>

namespace HLR {

    class LOREHLRUTILS_EXPORT CallbackInvokeExprMatcher
        : public clang::ast_matchers::MatchFinder::MatchCallback {
    public:
        inline CallbackInvokeExprMatcher(llvm::StringRef id,
                                         llvm::SmallVectorImpl<const clang::CallExpr *> &exprList)
            : _id(id), _exprList(exprList) {
        }

        void run(const clang::ast_matchers::MatchFinder::MatchResult &Result) override;

        inline clang::ast_matchers::StatementMatcher matcher() const {
            return clang::ast_matchers::callExpr().bind(_id);
        }

    protected:
        llvm::StringRef _id;
        llvm::SmallVectorImpl<const clang::CallExpr *> &_exprList;
    };

    class LOREHLRUTILS_EXPORT FunctionDecayExprMatcher
        : public clang::ast_matchers::MatchFinder::MatchCallback {
    public:
        inline FunctionDecayExprMatcher(llvm::StringRef id,
                                        llvm::SmallVectorImpl<const clang::DeclRefExpr *> &exprList)
            : _id(id), _exprList(exprList) {
        }

        void run(const clang::ast_matchers::MatchFinder::MatchResult &Result) override;

        inline clang::ast_matchers::StatementMatcher matcher() const {
            return clang::ast_matchers::declRefExpr(
                       clang::ast_matchers::to(clang::ast_matchers::functionDecl()))
                .bind(_id);
        }

    protected:
        llvm::StringRef _id;
        llvm::SmallVectorImpl<const clang::DeclRefExpr *> &_exprList;
    };

    /// Simple ASTConsumer to collect callback invoke expressions and function decay expressions.
    class FunctionExprConsumer : public clang::ASTConsumer {
    public:
        inline FunctionExprConsumer(
            llvm::SmallVectorImpl<const clang::CallExpr *> &callbackInvokeExprList,
            llvm::SmallVectorImpl<const clang::DeclRefExpr *> &functionDecayExprList)
            : _callbackInvokeExprMatcher("callExpr", callbackInvokeExprList),
              _functionDecayExprMatcher("functionRef", functionDecayExprList) {
            _matcher.addMatcher(_callbackInvokeExprMatcher.matcher(), &_callbackInvokeExprMatcher);
            _matcher.addMatcher(_functionDecayExprMatcher.matcher(), &_functionDecayExprMatcher);
        }

        void HandleTranslationUnit(clang::ASTContext &Context) override {
            _matcher.matchAST(Context);
        }

    private:
        clang::ast_matchers::MatchFinder _matcher;
        CallbackInvokeExprMatcher _callbackInvokeExprMatcher;
        FunctionDecayExprMatcher _functionDecayExprMatcher;
    };

}

#endif // LORE_TOOLS_HLRUTILS_ASTCONSUMERS_H
