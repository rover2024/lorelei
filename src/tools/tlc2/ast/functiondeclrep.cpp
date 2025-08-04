#include "functiondeclrep.h"

using namespace clang;
using namespace llvm;

namespace TLC {

    Expected<FunctionDeclRep> FunctionDeclRep::fromQualType(const clang::QualType &QT,
                                                            std::string name,
                                                            SmallVector<std::string> argNames) {
        clang::QualType realType;
        if (QT->isPointerType()) {
            realType = QT->getPointeeType();
        } else {
            realType = QT;
        }

        if (realType->isFunctionProtoType()) {
            const clang::FunctionProtoType *FPT = realType->getAs<clang::FunctionProtoType>();
            FunctionDeclRep result;
            result._qt = realType;
            result._typeRep = FPT;
            result._name = std::move(name);
            result._argNames = std::move(argNames);
            return result;
        } else if (realType->isFunctionNoProtoType()) {
            const clang::FunctionNoProtoType *FNT = realType->getAs<clang::FunctionNoProtoType>();
            FunctionDeclRep result;
            result._qt = realType;
            result._typeRep = FNT;
            result._name = std::move(name);
            result._argNames = std::move(argNames);
            return result;
        }
        return createStringError(std::errc::invalid_argument, "Not a function pointer type");
    }

}