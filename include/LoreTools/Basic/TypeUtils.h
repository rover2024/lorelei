// SPDX-License-Identifier: MIT

#ifndef LORE_TOOLS_BASIC_TYPEUTILS_H
#define LORE_TOOLS_BASIC_TYPEUTILS_H

#include <llvm/ADT/SmallVector.h>
#include <clang/AST/Type.h>
#include <clang/AST/Expr.h>
#include <clang/AST/ASTContext.h>

#include <LoreBase/CoreLib/ADT/StringExtras.h>

namespace HLR {

    inline clang::QualType realCalleeType(const clang::CallExpr *E, clang::ASTContext &AST) {
        auto T = E->getCallee()->getType().getCanonicalType()->getPointeeType();
        if (T->isFunctionNoProtoType() && E->getNumArgs() > 0) {
            auto funcType = T->getAs<clang::FunctionNoProtoType>();
            llvm::SmallVector<clang::QualType, 4> argTypes;
            for (unsigned i = 0; i < E->getNumArgs(); ++i) {
                argTypes.push_back(E->getArg(i)->getType());
            }
            clang::QualType FT = AST.getFunctionType(funcType->getReturnType(),
                                                     llvm::ArrayRef<clang::QualType>(argTypes), {})
                                     .getCanonicalType();
            T = FT;
        }
        return AST.getPointerType(T);
    }

    inline std::string getTypeString(const clang::QualType &T) {
        auto res = T.getAsString();
        res = lore::str::replace(res, "struct __va_list_tag[1]", "va_list");
        res = lore::str::replace(res, "struct __va_list_tag *", "va_list");
        return res;
    }

}

#endif // LORE_TOOLS_BASIC_TYPEUTILS_H
