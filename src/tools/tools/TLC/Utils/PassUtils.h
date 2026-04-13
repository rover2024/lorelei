#ifndef PASSCOMMON_H
#define PASSCOMMON_H

#include <llvm/ADT/StringExtras.h>
#include <clang/AST/ASTContext.h>

#include <lorelei/Tools/TLCApi/Core/ProcSnippet.h>

namespace lore::tool::TLC {

    template <class T, size_t N>
    static inline bool PASS_getIntegralArgumentsInPass(T (&outList)[N], ProcSnippet &proc,
                                                       bool isBuilder, int id) {
        clang::QualType passType;
        if (isBuilder) {
            if (!proc.desc()) {
                return false;
            }
            auto passInfo = proc.desc()->builderPass;
            if (!passInfo.has_value()) {
                return false;
            }
            if (passInfo->id != id) {
                return false;
            }
            passType = passInfo.value().type;
        } else {
            if (!proc.desc()) {
                return false;
            }
            auto it =
                std::find_if(proc.desc()->passes.begin(), proc.desc()->passes.end(),
                             [id](const ProcSnippet::PassInfo &pass) { return pass.id == id; });
            if (it == proc.desc()->passes.end()) {
                return false;
            }
            passType = it->type;
        }

        auto typeDecl = passType->getAsCXXRecordDecl();
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
