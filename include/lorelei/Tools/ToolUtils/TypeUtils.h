#ifndef LORE_TOOLS_TOOLUTILS_TYPEUTILS_H
#define LORE_TOOLS_TOOLUTILS_TYPEUTILS_H

#include <llvm/ADT/SmallVector.h>
#include <clang/AST/Type.h>
#include <clang/AST/Expr.h>
#include <clang/AST/ASTContext.h>

namespace lore::tool {

    /// Returns the real function pointer type when the type of \c E does not have a prototype but
    /// called with arguments.
    clang::QualType realCalleeType(const clang::CallExpr *E, clang::ASTContext &AST);

    /// When \c T is a \a va_list type, \c T.getAsString() returns \a "struct __va_list_tag[1]" or
    /// \a "struct __va_list_tag *", this function returns \a "va_list". Otherwise, it is equivalent
    /// to \c T.getAsString().
    std::string getTypeString(const clang::QualType &T);

    /// Returns the type string.
    /// \example
    ///     int -> "int"
    ///     int [] -> “__typeof__(int[])"
    ///     int (*a)(int) -> "__typeof__(int (*)(int))"
    std::string getTypeStringDecompound(const clang::QualType &type);

    /// Returns the type string with a name.
    /// \example
    ///     int -> "int a"
    ///     int * -> "int *a"
    std::string getTypeStringWithName(const clang::QualType &type, const std::string &name);

}

#endif // LORE_TOOLS_TOOLUTILS_TYPEUTILS_H
