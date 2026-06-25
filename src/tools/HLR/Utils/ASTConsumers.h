#ifndef LORE_TOOLS_HLR_ASTCONSUMERS_H
#define LORE_TOOLS_HLR_ASTCONSUMERS_H

#include <clang/AST/ASTConsumer.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>

namespace lore::tool::HLR {

    /// CallbackInvokeExprMatcher - Walk through the AST and collect all callback invoke
    /// expressions.
    ///
    /// Example forms:
    /// - fp(args...)
    /// - (*fp)(args...)
    class CallbackInvokeExprMatcher : public clang::ast_matchers::MatchFinder::MatchCallback {
    public:
        inline CallbackInvokeExprMatcher(llvm::StringRef id,
                                         llvm::SmallVectorImpl<const clang::CallExpr *> &exprList)
            : m_id(id), m_exprList(exprList) {
        }

        void run(const clang::ast_matchers::MatchFinder::MatchResult &Result) override;

        inline clang::ast_matchers::StatementMatcher matcher() const {
            return clang::ast_matchers::callExpr().bind(m_id);
        }

    protected:
        llvm::StringRef m_id;
        llvm::SmallVectorImpl<const clang::CallExpr *> &m_exprList;
    };

    /// FunctionDecayExprMatcher - Walk through the AST and collect all expressions where a function
    /// degenerates into a function pointer.
    ///
    /// Example forms:
    /// - FP fp = func;
    /// - FP fp = (a > b) ? func1 : func2;
    class FunctionDecayExprMatcher : public clang::ast_matchers::MatchFinder::MatchCallback {
    public:
        inline FunctionDecayExprMatcher(llvm::StringRef id,
                                        llvm::SmallVectorImpl<const clang::DeclRefExpr *> &exprList)
            : m_id(id), m_exprList(exprList) {
        }

        void run(const clang::ast_matchers::MatchFinder::MatchResult &Result) override;

        inline clang::ast_matchers::StatementMatcher matcher() const {
            return clang::ast_matchers::declRefExpr(
                       clang::ast_matchers::to(clang::ast_matchers::functionDecl()))
                .bind(m_id);
        }

    protected:
        llvm::StringRef m_id;
        llvm::SmallVectorImpl<const clang::DeclRefExpr *> &m_exprList;
    };

    class FunctionExprConsumer : public clang::ASTConsumer {
    public:
        inline FunctionExprConsumer(
            llvm::SmallVectorImpl<const clang::CallExpr *> &callbackInvokeExprList,
            llvm::SmallVectorImpl<const clang::DeclRefExpr *> &functionDecayExprList)
            : m_callbackInvokeExprMatcher("callExpr", callbackInvokeExprList),
              m_functionDecayExprMatcher("functionRef", functionDecayExprList) {
            m_matcher.addMatcher(m_callbackInvokeExprMatcher.matcher(),
                                 &m_callbackInvokeExprMatcher);
            m_matcher.addMatcher(m_functionDecayExprMatcher.matcher(), &m_functionDecayExprMatcher);
        }

        void HandleTranslationUnit(clang::ASTContext &Context) override {
            m_matcher.matchAST(Context);
        }

    private:
        clang::ast_matchers::MatchFinder m_matcher;
        CallbackInvokeExprMatcher m_callbackInvokeExprMatcher;
        FunctionDecayExprMatcher m_functionDecayExprMatcher;
    };

}

#endif // LORE_TOOLS_HLR_ASTCONSUMERS_H
