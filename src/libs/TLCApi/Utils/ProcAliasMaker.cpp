#include "ProcAliasMaker.h"

#include <cstdlib>
#include <cassert>

#include <clang/AST/Decl.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/DeclTemplate.h>
#include <llvm/Support/raw_ostream.h>

namespace lore::tool::TLC {

    void ProcAliasMaker::initialize(clang::ClassTemplateDecl *procDecl) {
        assert(procDecl);

        m_procDecl = procDecl;

        auto &ast = procDecl->getASTContext();
        auto templateParams = procDecl->getTemplateParameters()->asArray();

        if (templateParams.size() != 3) {
            llvm::errs() << "error: invalid template parameter count of " << procDecl->getName()
                         << "\n";
            std::exit(1);
        }

        /// STEP: Materialize enum template arguments (`ProcDirection` + `ProcPhase`).
        clang::EnumDecl *procDirectionEnumDecl = nullptr;
        clang::EnumDecl *procPhaseEnumDecl = nullptr;

        if (auto nonTypeArg = llvm::dyn_cast<clang::NonTypeTemplateParmDecl>(templateParams[1])) {
            auto argType = nonTypeArg->getType();
            if (!argType->isEnumeralType()) {
                llvm::errs() << "error: invalid template parameter type of "
                             << nonTypeArg->getName() << "\n";
                std::exit(1);
            }
            auto enumDecl = argType->getAs<clang::EnumType>()->getDecl();
            if (enumDecl->getName() != "ProcDirection") {
                llvm::errs() << "error: invalid template parameter type of "
                             << nonTypeArg->getName() << "\n";
                std::exit(1);
            }
            procDirectionEnumDecl = enumDecl;
        } else {
            llvm::errs() << "error: failed to find template parameter ProcDirection\n";
            std::exit(1);
        }

        if (auto nonTypeArg = llvm::dyn_cast<clang::NonTypeTemplateParmDecl>(templateParams[2])) {
            auto argType = nonTypeArg->getType();
            if (!argType->isEnumeralType()) {
                llvm::errs() << "error: invalid template parameter type of "
                             << nonTypeArg->getName() << "\n";
                return;
            }
            auto enumDecl = argType->getAs<clang::EnumType>()->getDecl();
            if (enumDecl->getName() != "ProcPhase") {
                llvm::errs() << "error: invalid template parameter type of "
                             << nonTypeArg->getName() << "\n";
                return;
            }
            procPhaseEnumDecl = enumDecl;
        } else {
            llvm::errs() << "error: failed to find template parameter ProcPhase\n";
            std::exit(1);
        }

        m_procDirectionEnumDecl = procDirectionEnumDecl;
        m_procPhaseEnumDecl = procPhaseEnumDecl;

        /// STEP: Create mangle context.
        m_mangleContext.reset(clang::ItaniumMangleContext::create(ast, ast.getDiagnostics()));
    }

    std::string
        ProcAliasMaker::getInvokeAlias(const char *direction, const char *phase,
                                       clang::FunctionDecl *fd,
                                       const std::optional<clang::QualType> &overlayType) const {

        assert(m_mangleContext.get());

        auto &ast = m_procDecl->getASTContext();

        /// STEP: Materialize enum constants.
        clang::EnumConstantDecl *procDirectionECD = nullptr;
        clang::EnumConstantDecl *procPhaseECD = nullptr;
        for (auto *ecd : m_procDirectionEnumDecl->enumerators()) {
            if (ecd->getName() == direction) {
                procDirectionECD = ecd;
                break;
            }
        }
        for (auto *ecd : m_procPhaseEnumDecl->enumerators()) {
            if (ecd->getName() == phase) {
                procPhaseECD = ecd;
                break;
            }
        }

        if (!procDirectionECD || !procPhaseECD) {
            llvm::errs() << "error: failed to find enum constant " << direction << " or " << phase
                         << "\n";
            return {};
        }

        auto procDirectionTemplateArg = clang::TemplateArgument(
            ast, procDirectionECD->getInitVal(), ast.getTypeDeclType(m_procDirectionEnumDecl));
        auto procPhaseTemplateArg = clang::TemplateArgument(
            ast, procPhaseECD->getInitVal(), ast.getTypeDeclType(m_procPhaseEnumDecl));

        std::array<clang::TemplateArgument, 3> templateArgs = {
            clang::TemplateArgument(fd, ast.getPointerType(fd->getType())),
            procDirectionTemplateArg,
            procPhaseTemplateArg,
        };

        /// STEP: Find (or create) the target specialization.
        void *insertPos = nullptr;
        auto *specDecl = m_procDecl->findSpecialization(templateArgs, insertPos);
        if (!specDecl) {
            specDecl = clang::ClassTemplateSpecializationDecl::Create(
                ast, m_procDecl->getTemplatedDecl()->getTagKind(),
                m_procDecl->getTemplatedDecl()->getDeclContext(),
                m_procDecl->getTemplatedDecl()->getLocation(), m_procDecl->getLocation(),
                m_procDecl, templateArgs,
#if LLVM_VERSION_MAJOR > 18
                false,
#endif
                nullptr);
        }

        if (!specDecl) {
            llvm::errs() << "error: failed to find or create thunk proc specialization.\n";
            return {};
        }

        /// STEP: Find (or create) static method `invoke`.
        clang::CXXMethodDecl *invokeMethod = nullptr;
        for (auto *decl : specDecl->decls()) {
            if (auto *method = llvm::dyn_cast<clang::CXXMethodDecl>(decl)) {
                if (method->getName() == "invoke") {
                    invokeMethod = method;
                    break;
                }
            }
        }

        if (!invokeMethod) {
            clang::QualType invokeType =
                overlayType ? overlayType.value()->getPointeeType() : fd->getType();
            invokeMethod = clang::CXXMethodDecl::Create(
                ast, specDecl, specDecl->getLocation(),
                clang::DeclarationNameInfo(clang::DeclarationName(&ast.Idents.get("invoke")),
                                           specDecl->getLocation()),
                invokeType, ast.getTrivialTypeSourceInfo(invokeType),
                clang::StorageClass::SC_Static, false, false, clang::ConstexprSpecKind::Unspecified,
                specDecl->getLocation());
            invokeMethod->setAccess(clang::AccessSpecifier::AS_public);
            invokeMethod->setImplicit(true);

            if (!overlayType) {
                invokeMethod->setParams(fd->parameters());
            } else {
                FunctionTypeView view(*overlayType);
                llvm::SmallVector<clang::ParmVarDecl *, 4> params;
                params.reserve(view.argTypes().size());
                for (size_t i = 0; i < view.argTypes().size(); ++i) {
                    auto argType = view.argTypes()[i];
                    auto *param = clang::ParmVarDecl::Create(
                        ast, invokeMethod->getDeclContext(), clang::SourceLocation(),
                        clang::SourceLocation(), &ast.Idents.get("arg" + std::to_string(i + 1)),
                        argType, ast.getTrivialTypeSourceInfo(argType),
                        clang::StorageClass::SC_None, nullptr);
                    params.push_back(param);
                }
                invokeMethod->setParams(params);
            }
            specDecl->addDecl(invokeMethod);
        }

        if (!invokeMethod->isUserProvided()) {
            llvm::errs() << "error: failed to create user-provided `invoke` method.\n";
            return {};
        }

        /// STEP: Return the mangled function symbol for alias generation.
        std::string mangledName;
        llvm::raw_string_ostream stream(mangledName);
        if (m_mangleContext->shouldMangleDeclName(invokeMethod)) {
            m_mangleContext->mangleName(invokeMethod, stream);
        }
        stream.flush();
        return mangledName;
    }

}
