#include "TypeUtils.h"

#include <lorelei/Base/Support/StringExtras.h>

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
        // x86_64 va_list
        res = str::replace(res, "struct __va_list_tag[1]", "va_list");
        res = str::replace(res, "struct __va_list_tag *", "va_list");
        // arm64 va_list
        res = str::replace(res, "struct std::__va_list", "va_list");
        res = str::replace(res, "struct __va_list", "va_list");
        return res;
    }

    static inline bool isCompound(clang::QualType type) {
        while (type->isPointerType()) {
            type = type->getPointeeType();
        }
        return type->isArrayType() || type->isFunctionType();
    }

    std::string getTypeStringDecompound(const clang::QualType &type) {
        std::string ret;
        if (isCompound(type)) {
            ret = "__typeof__(" + getTypeString(type) + ")";
        } else {
            ret = getTypeString(type);
        }
        return ret;
    }

    std::string getTypeStringWithName(const clang::QualType &type, const std::string &name) {
        auto typeStr = getTypeStringDecompound(type);
        if (!llvm::StringRef(typeStr).ends_with("*"))
            typeStr += " ";
        return typeStr + name;
    }

}
