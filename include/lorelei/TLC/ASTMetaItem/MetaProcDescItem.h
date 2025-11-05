#ifndef METAPROCDESCMESSAGE_H
#define METAPROCDESCMESSAGE_H

#include <clang/AST/Decl.h>
#include <clang/AST/DeclTemplate.h>
#include <llvm/ADT/SmallVector.h>

#include <lorelei/TLC/Global.h>
#include <lorelei/TLC/ASTMetaItem/MetaPassItem.h>

namespace TLC {

    class LORELIBTLC_EXPORT MetaProcDescItemBase {
    public:
        virtual ~MetaProcDescItemBase() = default;

        bool isValid() const {
            return _decl != nullptr;
        }

        /// Returns the name hint, available for callback desc items
        const std::string &nameHint() const {
            return _nameHint;
        }

        std::optional<std::reference_wrapper<const MetaPassItem>> builderPass() const {
            if (!_builderPass.isValid()) {
                return std::nullopt;
            }
            return _builderPass;
        }

        const llvm::SmallVector<MetaPassItem> &passes() const {
            return _passes;
        }

        /// Returns the overlay type (function pointer type)
        const std::optional<clang::QualType> &overlayType() const {
            return _overlayType;
        }

        const clang::ClassTemplateSpecializationDecl *decl() const {
            return _decl;
        }

    protected:
        MetaProcDescItemBase() = default;

        void postAssign();

        const clang::ClassTemplateSpecializationDecl *_decl = nullptr;
        std::string _nameHint;
        MetaPassItem _builderPass;
        llvm::SmallVector<MetaPassItem> _passes;
        std::optional<clang::QualType> _overlayType;
    };

    class LORELIBTLC_EXPORT MetaProcDescItem : public MetaProcDescItemBase {
    public:
        MetaProcDescItem() = default;

        explicit MetaProcDescItem(const clang::ClassTemplateSpecializationDecl *decl) {
            tryAssign(decl);
        }

    public:
        const clang::FunctionDecl *procDecl() const {
            return _procDecl;
        }

    protected:
        void tryAssign(const clang::ClassTemplateSpecializationDecl *decl);

        const clang::FunctionDecl *_procDecl = nullptr;
    };

    class LORELIBTLC_EXPORT MetaProcCBDescItem : public MetaProcDescItemBase {
    public:
        MetaProcCBDescItem() = default;

        explicit MetaProcCBDescItem(const clang::ClassTemplateSpecializationDecl *decl) {
            tryAssign(decl);
        }

    public:
        /// Returns the callback type (function pointer type)
        const clang::QualType &procType() const {
            return _procType;
        }

    protected:
        void tryAssign(const clang::ClassTemplateSpecializationDecl *decl);

        clang::QualType _procType = {};
    };

}

#endif // METAPROCDESCMESSAGE_H
