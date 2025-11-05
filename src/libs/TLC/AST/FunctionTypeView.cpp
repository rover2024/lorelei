#include "FunctionTypeView.h"

#include <clang/AST/ASTContext.h>

namespace TLC {

    clang::QualType FunctionTypeView::buildQualType(clang::ASTContext &ctx) const {
        clang::FunctionProtoType::ExtProtoInfo epi;
        epi.Variadic = _variadic;
        return ctx.getFunctionType(_returnType, _argTypes, epi);
    }

}