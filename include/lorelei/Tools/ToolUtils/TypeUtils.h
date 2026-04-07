#ifndef LORE_TOOLS_TOOLUTILS_TYPEUTILS_H
#define LORE_TOOLS_TOOLUTILS_TYPEUTILS_H

#include <llvm/ADT/SmallVector.h>
#include <clang/AST/Type.h>
#include <clang/AST/Expr.h>
#include <clang/AST/ASTContext.h>

namespace lore::tool {

    clang::QualType realCalleeType(const clang::CallExpr *E, clang::ASTContext &AST);

    std::string getTypeString(const clang::QualType &T);

}

#endif // LORE_TOOLS_TOOLUTILS_TYPEUTILS_H
