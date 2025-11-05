#include "MetaPassItem.h"

#include <clang/AST/DeclCXX.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/AST/ExprCXX.h>

using namespace clang;

namespace TLC {

    void MetaPassItem::tryAssign(const clang::QualType &type) {
        // Check if it's a specialization of MetaPass_xxx which derives from MetaPass
        auto canonicalType = type.getCanonicalType();
        auto decl = canonicalType.getTypePtr()->getAsCXXRecordDecl();
        if (!decl) {
            return;
        }

        if (decl->hasDefinition()) {
            // The decl has a definition, use it
            decl = decl->getDefinition();
        } else {
            // The decl doesn't have a definition, retrieve it from the template specialization
            auto templateSpecDecl = dyn_cast<ClassTemplateSpecializationDecl>(decl);
            if (!templateSpecDecl) {
                return;
            }
            auto templatedClassDecl = templateSpecDecl->getSpecializedTemplate();
            if (!templatedClassDecl) {
                return;
            }
            auto templatedDecl = templatedClassDecl->getTemplatedDecl();
            if (!templatedDecl || !templatedDecl->hasDefinition()) {
                return;
            }
            decl = templatedDecl->getDefinition();
        }

        if (decl->getNumBases() != 1) {
            return;
        }
        auto base = *decl->bases_begin();
        auto baseDecl = base.getType()->getAsCXXRecordDecl();
        if (!baseDecl) {
            return;
        }
        if (baseDecl->getName() != "MetaPass") {
            return;
        }

        // check BuiltinID
        for (auto &member : decl->decls()) {
            switch (member->getKind()) {
                case Decl::Var:
                    if (auto VD = dyn_cast<VarDecl>(member)) {
                        if (!VD->isStaticDataMember()) {
                            break;
                        }
                        if (VD->getName() == "ID") {
                            auto init = VD->getInit();
                            Expr::EvalResult result;
                            if (init->EvaluateAsInt(result, VD->getASTContext())) {
                                _id = result.Val.getInt().getSExtValue();
                            }
                            break;
                        }
                        if (VD->getName() == "isBuilder") {
                            auto init = VD->getInit();
                            if (bool result;
                                init->EvaluateAsBooleanCondition(result, VD->getASTContext())) {
                                _isBuilder = result;
                            }
                            break;
                        }
                    }
                    break;
                default:
                    break;
            }
        }

        _type = type;
    }

}