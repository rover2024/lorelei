#include "ASTConsumers.h"

using namespace clang;
using namespace clang::ast_matchers;

namespace lore::tool::HLR {

    static bool isFunctionPointerCallee(const Expr *E, ASTContext &AST) {
        std::ignore = AST;

        /// Check function pointer callee
        ///
        /// \code
        ///     typedef int (*Add)(int, int);
        ///     Add add;
        ///     add(1, 2);
        /// \endcode
        if (QualType type = E->getType().getCanonicalType(); type->isFunctionPointerType()) {
            return true;
        }

        /// Check function pointer callee dereferenced
        ///
        /// \code
        ///     typedef int (*Add)(int, int);
        ///     Add add;
        ///     (*add)(1, 2);
        /// \endcode
        if (auto UO = dyn_cast<UnaryOperator>(E->IgnoreParenImpCasts())) {
            if (UO->getOpcode() == clang::UO_Deref) {
                return true;
            }
        }
        return false;
    }

    static bool isDeclRefForCall(const DeclRefExpr *DRE, ASTContext &AST) {
        const Expr *E = DRE;
        while (E) {
            auto parents = AST.getParents(*E);
            if (parents.size() > 0) {
                auto parent = parents[0];
                if (auto CE = parent.get<CastExpr>()) {
                    E = CE;
                    continue;
                }
                if (auto PE = parent.get<ParenExpr>()) {
                    E = PE;
                    continue;
                }
                if (auto UO = parent.get<UnaryOperator>()) {
                    if (UO->getOpcode() == clang::UO_Deref || UO->getOpcode() == clang::UO_AddrOf) {
                        E = UO;
                        continue;
                    }
                }
                if (auto CE = parent.get<CallExpr>()) {
                    if (E == CE->getCallee()) {
                        return true;
                    }
                    return false;
                }
            }
            break;
        }
        return false;
    }

    void CallbackInvokeExprMatcher::run(const MatchFinder::MatchResult &Result) {
        auto &AST = *Result.Context;
        if (auto CE = Result.Nodes.getNodeAs<CallExpr>(_id)) {
            const Expr *calleeExpr = CE->getCallee()->IgnoreImpCasts()->IgnoreParens();
            if (!calleeExpr)
                return;

            if (isFunctionPointerCallee(calleeExpr, AST)) {
                _exprList.push_back(CE);
            }
        }
    }

    void FunctionDecayExprMatcher::run(const MatchFinder::MatchResult &Result) {
        auto &AST = *Result.Context;
        if (auto DRE = Result.Nodes.getNodeAs<DeclRefExpr>(_id)) {
            if (!isDeclRefForCall(DRE, AST)) {
                _exprList.push_back(DRE);
            } 
        }
    }

}
