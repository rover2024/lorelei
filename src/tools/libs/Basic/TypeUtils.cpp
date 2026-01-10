#include "TypeUtils.h"

#include <LoreBase/CoreLib/ADT/StringExtras.h>

namespace lore::tool {

    clang::QualType realCalleeType(const clang::CallExpr *E, clang::ASTContext &AST) {
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

    std::string getTypeString(const clang::QualType &T) {
        auto res = T.getAsString();
        res = str::replace(res, "struct __va_list_tag[1]", "va_list");
        res = str::replace(res, "struct __va_list_tag *", "va_list");
        return res;
    }

}
