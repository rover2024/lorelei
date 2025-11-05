#ifndef METAPROCITEM_H
#define METAPROCITEM_H

#include <clang/AST/Decl.h>
#include <clang/AST/DeclTemplate.h>

#include <lorelei-c/Core/ProcInfo_c.h>
#include <lorelei/TLC/Global.h>

namespace TLC {

    class LORELIBTLC_EXPORT MetaProcItemBase {
    public:
        virtual ~MetaProcItemBase() = default;

        bool isValid() const {
            return _decl != nullptr;
        }

        CProcKind procKind() const {
            return _procKind;
        }

        CProcThunkPhase thunkPhase() const {
            return _thunkPhase;
        }

        const clang::ClassTemplateSpecializationDecl *decl() const {
            return _decl;
        }

    protected:
        MetaProcItemBase() = default;

        void postAssign();

        const clang::ClassTemplateSpecializationDecl *_decl = nullptr;
        CProcKind _procKind = CProcKind_HostFunction;
        CProcThunkPhase _thunkPhase = CProcThunkPhase_GTP;
    };

    class LORELIBTLC_EXPORT MetaProcItem : public MetaProcItemBase {
    public:
        MetaProcItem() = default;

        explicit MetaProcItem(const clang::ClassTemplateSpecializationDecl *decl) {
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

    class LORELIBTLC_EXPORT MetaProcCBItem : public MetaProcItemBase {
    public:
        MetaProcCBItem() = default;

        explicit MetaProcCBItem(const clang::ClassTemplateSpecializationDecl *decl) {
            tryAssign(decl);
        }

    public:
        const clang::QualType &procType() const {
            return _procType;
        }

    protected:
        void tryAssign(const clang::ClassTemplateSpecializationDecl *decl);

        clang::QualType _procType = {};
    };

}

#endif // METAPROCITEM_H
