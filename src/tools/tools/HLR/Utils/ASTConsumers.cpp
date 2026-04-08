#include "ASTConsumers.h"

using namespace clang;
using namespace clang::ast_matchers;

namespace lore::tool::HLR {

    /// Return true when `E` represents a call through a function-pointer-like callee expression.
    ///
    /// Supported shapes:
    /// 1. `fp(...)` where `fp` has function pointer type.
    /// 2. `(*fp)(...)` where the callee is an explicit dereference expression.
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

    /// Return true when the given `DeclRefExpr` is used as the direct callee of a call.
    ///
    /// The check walks parent nodes and skips transparent wrappers such as casts, parentheses and
    /// unary `*`/`&`, then verifies whether the final parent call uses this expression as callee.
    ///
    /// If this returns false, the reference is treated as a function-decay candidate (for example
    /// assigning/storing/passing a function pointer value).
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
        if (auto CE = Result.Nodes.getNodeAs<CallExpr>(m_id)) {
            // Keep only call expressions whose callee is function-pointer based.
            const Expr *calleeExpr = CE->getCallee()->IgnoreImpCasts()->IgnoreParens();
            if (!calleeExpr)
                return;

            if (isFunctionPointerCallee(calleeExpr, AST)) {
                m_exprList.push_back(CE);
            }
        }
    }

    void FunctionDecayExprMatcher::run(const MatchFinder::MatchResult &Result) {
        auto &AST = *Result.Context;
        if (auto DRE = Result.Nodes.getNodeAs<DeclRefExpr>(m_id)) {
            // Exclude direct call-site callees; keep non-call function references as decay sites.
            if (!isDeclRefForCall(DRE, AST)) {
                m_exprList.push_back(DRE);
            } 
        }
    }

}
