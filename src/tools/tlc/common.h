#ifndef COMMON_H
#define COMMON_H

#include <clang/AST/AST.h>
#include <llvm/ADT/StringExtras.h>
#include <llvm/ADT/StringRef.h>

namespace TLC {

    std::string replace(std::string str, const std::string &from, const std::string &to);

    std::string getTypeString(const clang::QualType &type);

    std::string getCallArgsString(int n, bool PrefixComma = false);

    inline bool isCharPointerType(const clang::QualType &type) {
        return type->isPointerType() && type->getPointeeType()->isCharType();
    }

}

#endif // COMMON_H