#ifndef LORE_TOOLS_TLCAPI_PROCALIASMAKER_H
#define LORE_TOOLS_TLCAPI_PROCALIASMAKER_H

#include <memory>
#include <optional>
#include <string>

#include <clang/AST/Mangle.h>

#include <lorelei/Tools/TLCApi/Global.h>
#include <lorelei/Tools/ToolUtils/FunctionTypeView.h>

namespace lore::tool::TLC {

    /// ProcAliasMaker - Utility that resolves the mangled name of a thunk proc `invoke` symbol
    /// for a specific direction / phase pair.
    class LORETLCAPI_EXPORT ProcAliasMaker {
    public:
        ProcAliasMaker() = default;
        ~ProcAliasMaker() = default;

    public:
        /// Initialize the alias maker with the \c ProcFn template declaration.
        void initialize(clang::ClassTemplateDecl *procDecl);

        /// Return the mangled name of the \a invoke member function symbol of a \c ProcFn
        /// declaration.
        std::string getInvokeAlias(const char *direction, const char *phase,
                                   clang::FunctionDecl *fd,
                                   const std::optional<clang::QualType> &overlayType = {}) const;

    private:
        clang::ClassTemplateDecl *m_procDecl = nullptr;
        clang::EnumDecl *m_procDirectionEnumDecl = nullptr;
        clang::EnumDecl *m_procPhaseEnumDecl = nullptr;
        std::unique_ptr<clang::MangleContext> m_mangleContext;
    };

}

#endif // LORE_TOOLS_TLCAPI_PROCALIASMAKER_H
