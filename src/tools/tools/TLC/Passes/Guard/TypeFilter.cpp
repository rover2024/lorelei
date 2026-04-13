#include <lorelei/Base/Support/StringExtras.h>
#include <lorelei/Tools/TLCApi/Core/Pass.h>
#include <lorelei/Tools/TLCApi/Core/ProcSnippet.h>
#include <lorelei/Tools/TLCApi/Core/DocumentContext.h>
#include <lorelei/Tools/ThunkInterface/PassTags.h>
#include <lorelei/Tools/ToolUtils/CommonMatchFinder.h>
#include <lorelei/Tools/ToolUtils/TypeUtils.h>

#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>

#include "Utils/PassCodeTemplates.h"

using namespace clang;
using namespace clang::ast_matchers;

namespace lore::tool::TLC {

    class TypeFilterMessage : public PassMessage {
    public:
        TypeFilterMessage(llvm::SmallVector<size_t> filteredArgIndexes, bool filterRet)
            : filteredArgIndexes(std::move(filteredArgIndexes)), filterRet(filterRet) {
        }

        llvm::SmallVector<size_t> filteredArgIndexes;
        bool filterRet = false;
    };

    class TypeFilterPass : public Pass {
    public:
        TypeFilterPass() : Pass(Guard, lore::thunk::pass::ID_TypeFilter) {
        }

        std::string name() const override {
            return "TypeFilter";
        }

        void handleTranslationUnit(DocumentContext &doc) override;

        bool testProc(ProcSnippet &proc, std::unique_ptr<PassMessage> &msg) override;
        llvm::Error beginHandleProc(ProcSnippet &proc, std::unique_ptr<PassMessage> &msg) override;
        llvm::Error endHandleProc(ProcSnippet &proc, std::unique_ptr<PassMessage> &msg) override;

    protected:
        std::map<std::string, const ClassTemplateSpecializationDecl *> m_procArgFilters;
        std::map<std::string, const ClassTemplateSpecializationDecl *> m_procRetFilters;
    };

    static inline QualType getFilterType(const ClassTemplateSpecializationDecl *decl) {
        auto &tArgs = decl->getTemplateArgs();
        if (tArgs.size() != 1 || tArgs[0].getKind() != TemplateArgument::Type) {
            return {};
        }
        return tArgs[0].getAsType().getCanonicalType();
    }

    void TypeFilterPass::handleTranslationUnit(DocumentContext &doc) {
        const auto &matchCallback = [this](const MatchFinder::MatchResult &result) {
            if (auto procArgFilter =
                    result.Nodes.getNodeAs<ClassTemplateSpecializationDecl>("procArgFilter")) {
                auto filterType = getFilterType(procArgFilter);
                if (filterType.isNull()) {
                    return;
                }
                m_procArgFilters[getTypeString(filterType)] = procArgFilter;
            }
            if (auto procReturnFilter =
                    result.Nodes.getNodeAs<ClassTemplateSpecializationDecl>("procReturnFilter")) {
                auto filterType = getFilterType(procReturnFilter);
                if (filterType.isNull()) {
                    return;
                }
                m_procRetFilters[getTypeString(filterType)] = procReturnFilter;
            }
        };

        CommonMatchFinder matchHandler(matchCallback);
        MatchFinder finder;

        finder.addMatcher(
            classTemplateSpecializationDecl(hasName("ProcArgFilter")).bind("procArgFilter"),
            &matchHandler);
        finder.addMatcher(
            classTemplateSpecializationDecl(hasName("ProcReturnFilter")).bind("procReturnFilter"),
            &matchHandler);

        finder.matchAST(doc.ast());
    }

    bool TypeFilterPass::testProc(ProcSnippet &proc, std::unique_ptr<PassMessage> &msg) {
        auto view = proc.realFunctionTypeView();

        llvm::SmallVector<size_t> filteredArgIndexes;
        bool filterRet = false;

        for (size_t i = 0; i < view.argTypes().size(); ++i) {
            const auto &argTypeStr = getTypeString(view.argTypes()[i].getCanonicalType());
            if (m_procArgFilters.find(argTypeStr) != m_procArgFilters.end()) {
                filteredArgIndexes.push_back(i);
            }
        }

        const auto &retTypeStr = getTypeString(view.returnType().getCanonicalType());
        if (m_procRetFilters.find(retTypeStr) != m_procRetFilters.end()) {
            filterRet = true;
        }

        if (filteredArgIndexes.empty() && !filterRet) {
            return false;
        }
        msg = std::make_unique<TypeFilterMessage>(std::move(filteredArgIndexes), filterRet);
        return true;
    }

    llvm::Error TypeFilterPass::beginHandleProc(ProcSnippet &proc,
                                                std::unique_ptr<PassMessage> &msg) {
        auto &message = static_cast<TypeFilterMessage &>(*msg);

        const auto &real = proc.realFunctionTypeView();
        std::string key = name();
        FunctionInfo FI = real;

        auto &doc = proc.document();
        bool isHost = doc.mode() == DocumentContext::Host;
        bool isG2H = proc.direction() == ProcSnippet::GuestToHost;

        auto &ENT = proc.source(ProcSnippet::Entry);
        auto &CAL = proc.source(ProcSnippet::Caller);
        ProcSnippet::ProcSource emptyENT;
        ProcSnippet::ProcSource emptyCAL;

        auto &GENT = isHost ? emptyENT : ENT;
        auto &GCAL = isHost ? emptyCAL : CAL;
        auto &HENT = isHost ? ENT : emptyENT;
        auto &HCAL = isHost ? CAL : emptyCAL;

        auto &XENT = isG2H ? GENT : HENT;
        auto &YCAL = isG2H ? HCAL : GCAL;

        const auto &getProcDescType = [&]() {
            if (proc.isCallback()) {
                return formatN("ProcCbDesc<%1>", proc.name());
            }
            return formatN("ProcFnDesc<%1>", proc.name());
        };
        const auto &getProcKind = [&]() { return proc.isCallback() ? "Callback" : "Function"; };
        const auto &getProcDirection = [&]() {
            return proc.direction() == ProcSnippet::GuestToHost ? "GuestToHost" : "HostToGuest";
        };
        const auto &getArgFilterStatement = [&](size_t idx) {
            return formatN("ProcArgFilter<%1>::filter<%2, %3, %4, %5>(%6, ProcArgContext(%7));",
                           getTypeString(real.argTypes()[idx].getCanonicalType()),
                           getProcDescType(), idx, getProcKind(), getProcDirection(),
                           FI.argumentName(idx), SRC_callList(FI));
        };
        const auto &getRetFilterStatement = [&]() {
            return formatN("ProcReturnFilter<%1>::filter<%2, %3, %4>(ret, ProcArgContext(%5));",
                           getTypeString(real.returnType().getCanonicalType()), getProcDescType(),
                           getProcKind(), getProcDirection(), SRC_callList(FI));
        };

        for (const auto &idx : message.filteredArgIndexes) {
            XENT.body.forward.push_back(key, SRC_asIs(getArgFilterStatement(idx)));
            YCAL.body.forward.push_back(key, SRC_asIs(getArgFilterStatement(idx)));
        }

        if (message.filterRet) {
            XENT.body.backward.push_back(key, SRC_asIs(getRetFilterStatement()));
            YCAL.body.backward.push_back(key, SRC_asIs(getRetFilterStatement()));
        }

        return llvm::Error::success();
    }

    llvm::Error TypeFilterPass::endHandleProc(ProcSnippet &proc,
                                              std::unique_ptr<PassMessage> &msg) {
        return llvm::Error::success();
    }

    static llvm::Registry<Pass>::Add<TypeFilterPass> PR_TypeFilter("TypeFilter", {});

}
