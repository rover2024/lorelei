#ifndef METACONFIGITEM_H
#define METACONFIGITEM_H

#include <clang/AST/Decl.h>
#include <clang/AST/DeclTemplate.h>

#include <lorelei/TLC/Global.h>

namespace TLC {

    class LORELIBTLC_EXPORT MetaConfigItem {
    public:
        MetaConfigItem() = default;

        explicit MetaConfigItem(const clang::ClassTemplateSpecializationDecl *decl) {
            tryAssign(decl);
        }

    public:
        enum Scope {
            Builtin,
            User,
        };

        bool isValid() const {
            return _decl != nullptr;
        }

        Scope scope() const {
            return _scope;
        }

        std::optional<bool> isHost() const {
            return _isHost;
        }

        std::optional<std::string> moduleName() const {
            return _moduleName;
        }

        const clang::ClassTemplateSpecializationDecl *decl() const {
            return _decl;
        }

    protected:
        void tryAssign(const clang::ClassTemplateSpecializationDecl *decl);

        const clang::ClassTemplateSpecializationDecl *_decl = nullptr;
        Scope _scope = Builtin;
        std::optional<bool> _isHost;
        std::optional<std::string> _moduleName;
    };

}

#endif // METACONFIGITEM_H
