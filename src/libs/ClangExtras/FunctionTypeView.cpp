#include "FunctionTypeView.h"

#include <clang/AST/ASTContext.h>

namespace lore::tool {

    clang::QualType FunctionTypeView::buildQualType(clang::ASTContext &ctx) const {
        clang::FunctionProtoType::ExtProtoInfo epi;
        epi.Variadic = m_variadic;
        return ctx.getFunctionType(m_returnType, m_argTypes, epi);
    }

}
