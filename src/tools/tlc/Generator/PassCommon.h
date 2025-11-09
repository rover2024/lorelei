#ifndef PASSCOMMON_H
#define PASSCOMMON_H

#include <llvm/ADT/StringExtras.h>
#include <clang/AST/ASTContext.h>

#include <lorelei/TLC/Core/ProcContext.h>

namespace TLC {

    template <class T, size_t N>
    static inline bool PASS_getIntegralArgumentsInTemplate(T (&outList)[N], ProcContext &proc, int id) {
        if (!proc.builderID() || proc.builderID().value() != id) {
            return false;
        }

        auto metaPassOpt = proc.desc()->builderPass();
        auto typeDecl = metaPassOpt->get().type()->getAsCXXRecordDecl();

        if (!typeDecl) {
            return false;
        }
        auto templateSpecDecl = llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(typeDecl);
        if (!templateSpecDecl) {
            return false;
        }
        auto &tList = templateSpecDecl->getTemplateArgs();
        if (tList.size() != N) {
            return false;
        }
        for (size_t i = 0; i < N; ++i) {
            if (tList[i].getKind() != clang::TemplateArgument::ArgKind::Integral) {
                return false;
            }
        }
        for (size_t i = 0; i < N; ++i) {
            if constexpr (std::is_signed_v<T>) {
                outList[i] = tList[i].getAsIntegral().getSExtValue();
            } else {
                outList[i] = tList[i].getAsIntegral().getZExtValue();
            }
        }
        return true;
    }

}

#endif // PASSCOMMON_H
