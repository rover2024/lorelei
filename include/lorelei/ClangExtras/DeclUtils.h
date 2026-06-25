#ifndef LORE_CLANGEXTRAS_DECLUTILS_H
#define LORE_CLANGEXTRAS_DECLUTILS_H

#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclCXX.h>
#include <clang/Basic/Linkage.h>

namespace lore::tool {

    /// True if \a decl is lexically C: it lives in an `extern "C"` block, or in a C
    /// (non-C++) translation unit.
    template <class T>
    inline bool isCDecl(const T *decl) {
        const clang::DeclContext *dc = decl->getDeclContext();
        if (const auto *lsd = clang::dyn_cast<clang::LinkageSpecDecl>(dc)) {
            return lsd->getLanguage() == clang::LinkageSpecLanguageIDs::C;
        }
        return !decl->getASTContext().getLangOpts().CPlusPlus;
    }

    /// True if \a decl effectively has C linkage, either by its computed language linkage
    /// or by \c isCDecl.
    template <class T>
    inline bool isCLinkage(const T *decl) {
        if (decl->getLanguageLinkage() == clang::CLanguageLinkage) {
            return true;
        }
        return isCDecl(decl);
    }

}

#endif // LORE_CLANGEXTRAS_DECLUTILS_H
