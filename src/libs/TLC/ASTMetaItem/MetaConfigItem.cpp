#include "MetaConfigItem.h"

#include <lorelei/TLCMeta/MetaConfig.h>

using namespace clang;

namespace TLC {

    void MetaConfigItem::tryAssign(const clang::ClassTemplateSpecializationDecl *decl) {
        if (decl->getTemplateSpecializationKind() != TSK_ExplicitSpecialization) {
            return;
        }
        auto &tArgs = decl->getTemplateArgs();
        if (tArgs.size() != 1) {
            return;
        }
        auto &tArg = tArgs[0];
        if (tArg.getKind() != TemplateArgument::ArgKind::Integral) {
            return;
        }

        switch (tArg.getAsIntegral().getExtValue()) {
            case lorethunk::MCS_Builtin:
                _scope = Scope::Builtin;
                break;
            case lorethunk::MCS_User:
                _scope = Scope::User;
                break;
            default:
                return;
        }
        _decl = decl;

        // Get values
        for (auto &subDecl : decl->decls()) {
            switch (subDecl->getKind()) {
                case Decl::Var:
                    if (auto VD = dyn_cast<VarDecl>(subDecl)) {
                        if (!VD->isStaticDataMember()) {
                            break;
                        }
                        if (VD->getName() == "isHost") {
                            auto init = VD->getInit();
                            if (bool result;
                                init->EvaluateAsBooleanCondition(result, VD->getASTContext())) {
                                _isHost = result;
                            }
                            break;
                        }
                        if (VD->getName() == "moduleName") {
                            auto init = VD->getInit();
                            if (auto SL = dyn_cast<StringLiteral>(init)) {
                                _moduleName = SL->getString();
                            }
                            break;
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }

}