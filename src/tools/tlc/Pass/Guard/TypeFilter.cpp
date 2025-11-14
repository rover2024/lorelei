#include "Pass.h"

#include <stdcorelib/str.h>

#include <lorelei/TLCMeta/MetaPass.h>

#include "ProcContext.h"
#include "DocumentContext.h"
#include "PassCommon.h"
#include "BuilderPassCommon.h"

#include "CommonMatchFinder.h"

using namespace clang;
using namespace clang::ast_matchers;

namespace TLC {

    class TypeFilterMessage : public ProcMessage {
    public:
        TypeFilterMessage(llvm::SmallVector<size_t> filteredArgIndexes, bool filterRet)
            : filteredArgIndexes(std::move(filteredArgIndexes)), filterRet(filterRet) {
        }

        llvm::SmallVector<size_t> filteredArgIndexes;
        bool filterRet = false;
    };

    class TypeFilterPass : public Pass {
    public:
        TypeFilterPass() : Pass(Guard, lorethunk::MetaPass::TypeFilter) {
        }

        std::string name() const override {
            return "TypeFilter";
        }

        void handleTranslationUnit(DocumentContext &doc) override;

        bool testProc(ProcContext &proc, std::unique_ptr<ProcMessage> &msg) override;
        llvm::Error beginHandleProc(ProcContext &proc, std::unique_ptr<ProcMessage> &msg) override;
        llvm::Error endHandleProc(ProcContext &proc, std::unique_ptr<ProcMessage> &msg) override;

    protected:
        std::map<std::string, const ClassTemplateSpecializationDecl *> _metaProcArgFilters;
        std::map<std::string, const ClassTemplateSpecializationDecl *> _metaProcRetFilters;
    };

    static inline QualType getFilterType(const ClassTemplateSpecializationDecl *decl) {
        auto &tArgs = decl->getTemplateArgs();
        if (tArgs.size() != 1) {
            return {};
        }
        return tArgs[0].getAsType().getCanonicalType();
    }

    void TypeFilterPass::handleTranslationUnit(DocumentContext &doc) {
        const auto &matchCallback = [this](const MatchFinder::MatchResult &Result) {
            if (auto metaProcArgFilter =
                    Result.Nodes.getNodeAs<ClassTemplateSpecializationDecl>("metaProcArgFilter")) {
                auto filterType = getFilterType(metaProcArgFilter);
                if (filterType.isNull()) {
                    return;
                }
                _metaProcArgFilters[getTypeString(filterType)] = metaProcArgFilter;
            }
            if (auto metaProcRetFilter = Result.Nodes.getNodeAs<ClassTemplateSpecializationDecl>(
                    "metaProcReturnFilter")) {
                auto filterType = getFilterType(metaProcRetFilter);
                if (filterType.isNull()) {
                    return;
                }
                _metaProcRetFilters[getTypeString(filterType)] = metaProcRetFilter;
            }
        };

        CommonMatchFinder matchHandler(matchCallback);
        MatchFinder finder;

        /// STEP: Match \c MetaProcArgFilter
        finder.addMatcher(                   //
            classTemplateSpecializationDecl( //
                hasName("MetaProcArgFilter") //
                )
                .bind("metaProcArgFilter"),
            &matchHandler);

        /// STEP: Match \c MetaProcReturnFilter
        finder.addMatcher(                      //
            classTemplateSpecializationDecl(    //
                hasName("MetaProcReturnFilter") //
                )
                .bind("metaProcReturnFilter"),
            &matchHandler);

        finder.matchAST(*doc.ast());
    }

    bool TypeFilterPass::testProc(ProcContext &proc, std::unique_ptr<ProcMessage> &msg) {
        auto view = proc.realFunctionTypeView();

        llvm::SmallVector<size_t> filteredArgIndexes;
        bool filterRet = false;

        for (size_t i = 0; i < view.argTypes().size(); ++i) {
            const auto &argType = view.argTypes()[i];
            const auto &argTypeStr = getTypeString(argType.getCanonicalType());
            if (_metaProcArgFilters.find(argTypeStr) != _metaProcArgFilters.end()) {
                filteredArgIndexes.push_back(i);
            }
        }

        auto it = _metaProcRetFilters.find(getTypeString(view.returnType().getCanonicalType()));
        if (it != _metaProcRetFilters.end()) {
            filterRet = true;
        }

        if (filteredArgIndexes.empty() && !filterRet) {
            return false;
        }
        msg = std::make_unique<TypeFilterMessage>(std::move(filteredArgIndexes), filterRet);
        return true;
    }

    llvm::Error TypeFilterPass::beginHandleProc(ProcContext &proc,
                                                std::unique_ptr<ProcMessage> &msg) {
        auto &message = static_cast<TypeFilterMessage &>(*msg);

        auto &doc = proc.documentContext();
        auto &ast = *doc.ast();
        const auto &real = proc.realFunctionTypeView();

        std::string key = name();
        FunctionInfo FI = real;

        auto &GTP = proc.source(CProcThunkPhase_GTP);
        auto &GTP_IMPL = proc.source(CProcThunkPhase_GTP_IMPL);
        auto &HTP = proc.source(CProcThunkPhase_HTP);
        auto &HTP_IMPL = proc.source(CProcThunkPhase_HTP_IMPL);

        bool isCallback =
            proc.procKind() == CProcKind_HostCallback || proc.procKind() == CProcKind_GuestCallback;
        const char *procKind_str = CProcKindNames[proc.procKind()];

        const auto &getArgFilterStatement = [&](size_t idx) {
            return stdc::formatN(
                "MetaProcArgFilter<%2>::filter<%3<%4>, %5, CProcKind_%6>"
                "(%1, MetaProcArgContext(%7));",
                FI.argumentName(idx), getTypeString(real.argTypes()[idx].getCanonicalType()),
                isCallback ? "MetaProcCBDesc" : "MetaProcDesc",
                (isCallback ? "" : "::") + proc.name(), idx, procKind_str, SRC_callList(FI));
        };

        const auto &getRetFilterStatement = [&]() {
            return stdc::formatN("MetaProcReturnFilter<%1>::filter<%2<%3>, CProcKind_%4>"
                                 "(ret, MetaProcArgContext(%5));",
                                 getTypeString(real.returnType().getCanonicalType()),
                                 isCallback ? "MetaProcCBDesc" : "MetaProcDesc",
                                 (isCallback ? "" : "::") + proc.name(), procKind_str,
                                 SRC_callList(FI));
        };

        {
            bool isHost = CProcKind_isHost(proc.procKind());

            auto &XTP = isHost ? GTP : HTP;
            auto &XTP_IMPL = isHost ? GTP_IMPL : HTP_IMPL;
            auto &YTP = isHost ? HTP : GTP;
            auto &YTP_IMPL = isHost ? HTP_IMPL : GTP_IMPL;

            for (const auto &idx : message.filteredArgIndexes) {
                XTP.body.forward.push_back(key, SRC_asIs(getArgFilterStatement(idx)));
                YTP_IMPL.body.forward.push_back(key, SRC_asIs(getArgFilterStatement(idx)));
            }

            if (message.filterRet) {
                XTP.body.backward.push_back(key, SRC_asIs(getRetFilterStatement()));
                YTP_IMPL.body.backward.push_back(key, SRC_asIs(getRetFilterStatement()));
            }
        }

        return llvm::Error::success();
    }

    llvm::Error TypeFilterPass::endHandleProc(ProcContext &proc,
                                              std::unique_ptr<ProcMessage> &msg) {
        return llvm::Error::success();
    }

    static PassRegistration<TypeFilterPass> PR_TypeFilter;

}