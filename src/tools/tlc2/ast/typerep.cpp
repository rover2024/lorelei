#include "typerep.h"

#include "common.h"

namespace TLC {

    std::string TypeRep::getAsString() const {
        if (_type == String) {
            return toString();
        }
        return getTypeString(toQualType());
    }

    FunctionTypeRep::FunctionTypeRep(const clang::FunctionNoProtoType *FNT) {
        _returnType = FNT->getReturnType();
        _variadic = false;
    }

    FunctionTypeRep::FunctionTypeRep(const clang::FunctionProtoType *FTP) {
        _returnType = FTP->getReturnType();
        _variadic = FTP->isVariadic();
        for (const auto &param : FTP->param_types()) {
            _argTypes.push_back(param);
        }
    }

    FunctionTypeRep::FunctionTypeRep(const clang::FunctionDecl *FD) {
        _returnType = FD->getReturnType();
        _variadic = FD->isVariadic();
        for (const auto &param : FD->parameters()) {
            _argTypes.push_back(param->getType());
        }
    }

}