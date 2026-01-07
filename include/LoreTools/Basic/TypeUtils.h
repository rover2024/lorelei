// SPDX-License-Identifier: MIT

#ifndef LORE_TOOLS_BASIC_TYPEUTILS_H
#define LORE_TOOLS_BASIC_TYPEUTILS_H

#include <llvm/ADT/SmallVector.h>
#include <clang/AST/Type.h>
#include <clang/AST/Expr.h>
#include <clang/AST/ASTContext.h>

#include <LoreTools/Basic/Global.h>

namespace HLR {

    LORETOOLBASIC_EXPORT clang::QualType realCalleeType(const clang::CallExpr *E,
                                                        clang::ASTContext &AST);

    LORETOOLBASIC_EXPORT std::string getTypeString(const clang::QualType &T);

}

#endif // LORE_TOOLS_BASIC_TYPEUTILS_H
