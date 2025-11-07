#include "MetaProcDescItem.h"

using namespace clang;

namespace TLC {

    void MetaProcDescItemBase::postAssign() {
        for (auto &subDecl : _decl->decls()) {
            switch (subDecl->getKind()) {
                case Decl::Var:
                    if (auto VD = dyn_cast<VarDecl>(subDecl)) {
                        if (!VD->isStaticDataMember()) {
                            break;
                        }
                        if (VD->getName() == "name") {
                            auto init = VD->getInit();
                            if (auto SL = dyn_cast<StringLiteral>(init)) {
                                _nameHint = SL->getString().str();
                            }
                            break;
                        }
                        if (VD->getName() == "builder_pass") {
                            auto type = VD->getType();
                            MetaPassItem passItem(type);
                            if (!passItem.isValid()) {
                                break;
                            }
                            _builderPass = passItem;
                            break;
                        }
                        if (VD->getName() == "passes") {
                            auto type = VD->getType();
                            auto typeDecl = type->getAsCXXRecordDecl();
                            if (!typeDecl) {
                                break;
                            }
                            if (typeDecl->getName() != "MetaPassList") {
                                break;
                            }
                            auto templateSpecDecl =
                                dyn_cast<ClassTemplateSpecializationDecl>(typeDecl);
                            if (!templateSpecDecl) {
                                break;
                            }
                            auto &tList = templateSpecDecl->getTemplateArgs();
                            if (tList.size() != 1) {
                                break;
                            }
                            auto &tArg = tList[0];
                            if (tArg.getKind() != TemplateArgument::ArgKind::Pack) {
                                break;
                            }
                            for (auto &packArg : tArg.getPackAsArray()) {
                                if (packArg.getKind() != TemplateArgument::ArgKind::Type) {
                                    continue;
                                }
                                auto passType = packArg.getAsType().getCanonicalType();
                                MetaPassItem passItem(passType);
                                if (!passItem.isValid()) {
                                    continue;
                                }
                                _passes.push_back(passItem);
                            }
                            break;
                        }
                    }
                    break;

                case Decl::Typedef:
                case Decl::TypeAlias:
                    if (auto TD = dyn_cast<TypeAliasDecl>(subDecl)) {
                        if (TD->getName() == "overlay_type") {
                            auto type = TD->getUnderlyingType().getCanonicalType();
                            if (!type.getCanonicalType()->isFunctionPointerType()) {
                                break;
                            }
                            _overlayType = type;
                            break;
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }

    void MetaProcDescItem::tryAssign(const ClassTemplateSpecializationDecl *decl) {
        if (decl->getTemplateSpecializationKind() != TSK_ExplicitSpecialization) {
            return;
        }
        auto &tArgs = decl->getTemplateArgs();
        if (tArgs.size() != 1) {
            return;
        }
        auto &tArg = tArgs[0];
        if (tArg.getKind() != TemplateArgument::ArgKind::Declaration) {
            return;
        }
        auto tArgDecl = tArg.getAsDecl();
        if (tArgDecl->getKind() != Decl::Kind::Function) {
            return;
        }
        _decl = decl;
        _procDecl = tArgDecl->getAsFunction();

        postAssign();
    }

    void MetaProcCBDescItem::tryAssign(const ClassTemplateSpecializationDecl *decl) {
        if (decl->getTemplateSpecializationKind() != TSK_ExplicitSpecialization) {
            return;
        }

        auto &tArgs = decl->getTemplateArgs();
        if (tArgs.size() != 1) {
            return;
        }
        auto &tArg = tArgs[0];
        if (tArg.getKind() != TemplateArgument::ArgKind::Type) {
            return;
        }
        auto tArgType = tArg.getAsType();
        if (!tArgType.getCanonicalType()->isFunctionPointerType()) {
            return;
        }
        _decl = decl;
        _procType = tArgType;

        postAssign();
    }


}