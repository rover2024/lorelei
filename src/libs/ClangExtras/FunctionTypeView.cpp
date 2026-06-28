// SPDX-License-Identifier: MIT

#include "FunctionTypeView.h"

#include <clang/AST/ASTContext.h>

namespace lore::tool {

    clang::QualType FunctionTypeView::buildQualType(clang::ASTContext &ctx) const {
        // Variadic-ness is not part of the return/arg types, so it is carried separately via the
        // ExtProtoInfo when reassembling the function type.
        clang::FunctionProtoType::ExtProtoInfo epi;
        epi.Variadic = m_variadic;
        return ctx.getFunctionType(m_returnType, m_argTypes, epi);
    }

}
