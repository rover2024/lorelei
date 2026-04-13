#include "DocumentContext.h"

#include <filesystem>
#include <array>
#include <vector>

#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/Type.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/AST/Expr.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/Error.h>

#include <lorelei/Tools/TLCApi/Core/Pass.h>
#include <lorelei/Tools/ToolUtils/CommonMatchFinder.h>
#include <lorelei/Tools/ToolUtils/TypeUtils.h>

using namespace clang;
using namespace clang::ast_matchers;

namespace lore::tool::TLC {

    DocumentContext::~DocumentContext() = default;

    template <class T>
    static bool isCDecl(const T *decl) {
        const DeclContext *dc = decl->getDeclContext();
        if (const auto *lsd = dyn_cast<LinkageSpecDecl>(dc)) {
            return lsd->getLanguage() == LinkageSpecLanguageIDs::C;
        }

        const LangOptions &langOpts = decl->getASTContext().getLangOpts();
        if (!langOpts.CPlusPlus) {
            return true;
        }
        return false;
    }

    template <class T>
    static bool isCLinkage(const T *decl) {
        if (decl->getLanguageLinkage() == CLanguageLinkage) {
            return true;
        }
        return isCDecl(decl);
    }

    static bool isBridgeNamespaceDecl(const Decl *decl) {
        if (!decl) {
            return false;
        }

        const DeclContext *dc = decl->getDeclContext();
        if (!dc) {
            return false;
        }

        // Require exact namespace suffix: lore::thunk::__bridge__
        std::array<llvm::StringRef, 3> expected = {"__bridge__", "thunk", "lore"};
        size_t matched = 0;
        while (dc && matched < expected.size()) {
            const auto *ns = dyn_cast<NamespaceDecl>(dc);
            if (!ns) {
                dc = dc->getParent();
                continue;
            }
            if (ns->isAnonymousNamespace()) {
                return false;
            }
            if (ns->getName() == expected[matched]) {
                matched++;
            } else {
                return false;
            }
            dc = dc->getParent();
        }
        return matched == expected.size();
    }

    static const FunctionDecl *extractFunctionDeclFromTemplateArg(const TemplateArgument &arg) {
        if (arg.getKind() == TemplateArgument::Declaration) {
            return dyn_cast<FunctionDecl>(arg.getAsDecl());
        }

        if (arg.getKind() != TemplateArgument::Expression) {
            return nullptr;
        }

        const Expr *expr = arg.getAsExpr();
        if (!expr) {
            return nullptr;
        }
        expr = expr->IgnoreParenImpCasts();

        if (const auto *dre = dyn_cast<DeclRefExpr>(expr)) {
            return dyn_cast<FunctionDecl>(dre->getDecl());
        }
        if (const auto *uo = dyn_cast<UnaryOperator>(expr)) {
            if (uo->getOpcode() == UO_AddrOf) {
                if (const auto *inner =
                        dyn_cast<DeclRefExpr>(uo->getSubExpr()->IgnoreParenImpCasts())) {
                    return dyn_cast<FunctionDecl>(inner->getDecl());
                }
            }
        }
        return nullptr;
    }

    static std::optional<int> evalStaticPassID(const VarDecl &vd, ASTContext &ast) {
        const Expr *initExpr = vd.getInit();
        if (!initExpr) {
            return std::nullopt;
        }

        Expr::EvalResult result;
        if (!initExpr->EvaluateAsInt(result, ast)) {
            return std::nullopt;
        }
        return static_cast<int>(result.Val.getInt().getSExtValue());
    }

    static std::optional<int> extractPassIDFromType(QualType type, ASTContext &ast) {
        (void) ast;
        type = type.getCanonicalType();
        const auto *decl = type->getAsCXXRecordDecl();
        if (!decl) {
            return std::nullopt;
        }

        for (const auto *subDecl : decl->decls()) {
            const auto *vd = dyn_cast<VarDecl>(subDecl);
            if (!vd || !vd->isStaticDataMember() || vd->getName() != "ID") {
                continue;
            }
            if (auto passID = evalStaticPassID(*vd, vd->getASTContext())) {
                return *passID;
            }
        }
        return std::nullopt;
    }

    static void appendPassTypesFromPassListType(QualType listType, ASTContext &ast,
                                                llvm::SmallVectorImpl<QualType> &out) {
        (void) ast;
        listType = listType.getCanonicalType();
        const auto *decl = listType->getAsCXXRecordDecl();
        if (!decl || decl->getName() != "PassTagList") {
            return;
        }

        const auto *specDecl = dyn_cast<ClassTemplateSpecializationDecl>(decl);
        if (!specDecl) {
            return;
        }

        for (const auto &arg : specDecl->getTemplateArgs().asArray()) {
            if (arg.getKind() != TemplateArgument::Type) {
                continue;
            }
            auto passType = arg.getAsType().getCanonicalType();
            bool exists = false;
            for (auto existing : out) {
                if (existing.getCanonicalType() == passType) {
                    exists = true;
                    break;
                }
            }
            if (!exists) {
                out.push_back(passType);
            }
        }
    }

    static std::optional<QualType> extractOverlayType(const ClassTemplateSpecializationDecl &decl,
                                                      ASTContext &ast) {
        for (const auto *subDecl : decl.decls()) {
            const auto *td = dyn_cast<TypedefNameDecl>(subDecl);
            if (!td || td->getName() != "overlay_type") {
                continue;
            }
            auto type = td->getUnderlyingType().getCanonicalType();
            if (type->isFunctionType()) {
                type = ast.getPointerType(type);
            }
            type = type.getCanonicalType();
            if (!type->isFunctionPointerType()) {
                return std::nullopt;
            }
            return type;
        }
        return std::nullopt;
    }

    static ProcSnippet::Desc extractProcDesc(const ClassTemplateSpecializationDecl &decl,
                                             ASTContext &ast) {
        ProcSnippet::Desc desc;
        desc.overlayType = extractOverlayType(decl, ast);

        for (const auto *subDecl : decl.decls()) {
            const auto *vd = dyn_cast<VarDecl>(subDecl);
            if (!vd || !vd->isStaticDataMember()) {
                continue;
            }

            auto memberName = vd->getName();
            if (memberName == "builder_pass" || memberName == "BUILDER_PASS") {
                auto builderType = vd->getType().getCanonicalType();
                desc.builderPass = builderType;
                if (auto passID = extractPassIDFromType(builderType, ast)) {
                    desc.builderID = *passID;
                }
                continue;
            }
            if (memberName == "passes" || memberName == "PASSES") {
                appendPassTypesFromPassListType(vd->getType(), ast, desc.passes);
                continue;
            }
        }
        return desc;
    }

    static std::optional<ProcSnippet::Direction> extractDirectionArg(const TemplateArgument &arg) {
        if (arg.getKind() != TemplateArgument::Integral) {
            return std::nullopt;
        }
        auto directionValue = arg.getAsIntegral().getExtValue();
        if (directionValue == ProcSnippet::GuestToHost) {
            return ProcSnippet::GuestToHost;
        }
        if (directionValue == ProcSnippet::HostToGuest) {
            return ProcSnippet::HostToGuest;
        }
        return std::nullopt;
    }

    static std::optional<ProcSnippet::Phase> extractPhaseArg(const TemplateArgument &arg) {
        if (arg.getKind() != TemplateArgument::Integral) {
            return std::nullopt;
        }
        auto phaseValue = arg.getAsIntegral().getExtValue();
        if (phaseValue == ProcSnippet::Entry) {
            return ProcSnippet::Entry;
        }
        if (phaseValue == ProcSnippet::Caller) {
            return ProcSnippet::Caller;
        }
        return std::nullopt;
    }

    bool DocumentContext::beginSourceFileAction(clang::CompilerInstance &CI) {
        (void) CI;
        for (auto &map : m_passMaps) {
            map.clear();
        }
        m_passInstances.clear();

        for (const auto &entry : llvm::Registry<Pass>::entries()) {
            auto pass = entry.instantiate();
            if (!pass) {
                continue;
            }

            auto phaseIndex = static_cast<size_t>(pass->phase());
            if (phaseIndex >= m_passMaps.size()) {
                continue;
            }
            m_passMaps[phaseIndex][pass->id()] = pass.get();
            m_passInstances.push_back(std::move(pass));
        }
        return true;
    }

    void DocumentContext::handleTranslationUnit(clang::ASTContext &ast) {
        m_ast = &ast;

        ClassTemplateDecl *procFnTemplateDecl = nullptr;

        std::map<std::string, std::pair<QualType, std::string>> namedCallbackBySignature;
        std::map<std::string, const FunctionDecl *> functionByName;
        std::map<std::string, const VarDecl *> varByName;
        std::map<std::string, std::pair<const FunctionDecl *, ProcSnippet::Desc>>
            functionDescByName;
        std::map<std::string, std::pair<QualType, ProcSnippet::Desc>> callbackDescBySignature;
        std::array<
            std::array<std::map<std::string, std::pair<const FunctionDecl *,
                                                       const ClassTemplateSpecializationDecl *>>,
                       ProcSnippet::NumPhases>,
            ProcSnippet::NumDirections>
            procFnDeclByName;
        std::array<
            std::array<
                std::map<std::string, std::pair<QualType, const ClassTemplateSpecializationDecl *>>,
                ProcSnippet::NumPhases>,
            ProcSnippet::NumDirections>
            procCbDeclBySignature;

        const auto matchCallback = [&](const MatchFinder::MatchResult &result) {
            if (const auto *decl = result.Nodes.getNodeAs<FunctionDecl>("functionDecl")) {
                if (!isCLinkage(decl)) {
                    return;
                }
                functionByName[decl->getNameAsString()] = decl;
                return;
            }

            if (const auto *decl = result.Nodes.getNodeAs<VarDecl>("varDecl")) {
                if (!isCLinkage(decl)) {
                    return;
                }
                varByName[decl->getNameAsString()] = decl;
                return;
            }

            if (const auto *decl =
                    result.Nodes.getNodeAs<ClassTemplateDecl>("procFnTemplateDecl")) {
                if (!procFnTemplateDecl) {
                    procFnTemplateDecl = const_cast<ClassTemplateDecl *>(decl);
                }
                return;
            }

            if (const auto *decl = result.Nodes.getNodeAs<TypedefDecl>("typedefDecl")) {
                if (!isBridgeNamespaceDecl(decl)) {
                    return;
                }
                auto type = decl->getUnderlyingType().getCanonicalType();
                if (!type->isFunctionPointerType()) {
                    return;
                }
                auto name = decl->getNameAsString();
                namedCallbackBySignature[getTypeString(type)] = {type, name};
                return;
            }

            if (const auto *decl = result.Nodes.getNodeAs<TypeAliasDecl>("typeAliasDecl")) {
                if (!isBridgeNamespaceDecl(decl)) {
                    return;
                }
                auto type = decl->getUnderlyingType().getCanonicalType();
                if (!type->isFunctionPointerType()) {
                    return;
                }
                auto name = decl->getNameAsString();
                namedCallbackBySignature[getTypeString(type)] = {type, name};
                return;
            }

            if (const auto *decl =
                    result.Nodes.getNodeAs<ClassTemplateSpecializationDecl>("procFnDescDecl")) {
                if (decl->getTemplateSpecializationKind() != TSK_ExplicitSpecialization) {
                    return;
                }
                auto tArgs = decl->getTemplateArgs().asArray();
                if (tArgs.size() != 1) {
                    return;
                }
                const auto *fd = extractFunctionDeclFromTemplateArg(tArgs[0]);
                if (!fd) {
                    return;
                }
                functionDescByName[fd->getNameAsString()] = {fd, extractProcDesc(*decl, ast)};
                return;
            }

            if (const auto *decl =
                    result.Nodes.getNodeAs<ClassTemplateSpecializationDecl>("procCbDescDecl")) {
                if (decl->getTemplateSpecializationKind() != TSK_ExplicitSpecialization) {
                    return;
                }
                auto tArgs = decl->getTemplateArgs().asArray();
                if (tArgs.size() != 1 || tArgs[0].getKind() != TemplateArgument::Type) {
                    return;
                }
                auto type = tArgs[0].getAsType().getCanonicalType();
                if (type->isFunctionType()) {
                    type = ast.getPointerType(type);
                }
                if (!type->isFunctionPointerType()) {
                    return;
                }
                callbackDescBySignature[getTypeString(type)] = {type, extractProcDesc(*decl, ast)};
                return;
            }

            if (const auto *decl =
                    result.Nodes.getNodeAs<ClassTemplateSpecializationDecl>("procFnDecl")) {
                if (decl->getTemplateSpecializationKind() != TSK_ExplicitSpecialization) {
                    return;
                }
                auto tArgs = decl->getTemplateArgs().asArray();
                if (tArgs.size() != 3) {
                    return;
                }

                const auto *fd = extractFunctionDeclFromTemplateArg(tArgs[0]);
                if (!fd) {
                    return;
                }
                auto direction = extractDirectionArg(tArgs[1]);
                auto phase = extractPhaseArg(tArgs[2]);
                if (!direction || !phase) {
                    return;
                }
                procFnDeclByName[*direction][*phase][fd->getNameAsString()] = {fd, decl};
                return;
            }

            if (const auto *decl =
                    result.Nodes.getNodeAs<ClassTemplateSpecializationDecl>("procCbDefl")) {
                if (decl->getTemplateSpecializationKind() != TSK_ExplicitSpecialization) {
                    return;
                }
                auto tArgs = decl->getTemplateArgs().asArray();
                if (tArgs.size() != 3 || tArgs[0].getKind() != TemplateArgument::Type) {
                    return;
                }

                auto type = tArgs[0].getAsType().getCanonicalType();
                if (type->isFunctionType()) {
                    type = ast.getPointerType(type);
                }
                if (!type->isFunctionPointerType()) {
                    return;
                }
                auto direction = extractDirectionArg(tArgs[1]);
                auto phase = extractPhaseArg(tArgs[2]);
                if (!direction || !phase) {
                    return;
                }
                procCbDeclBySignature[*direction][*phase][getTypeString(type)] = {type, decl};
                return;
            }
        };

        /// STEP: Collect requested AST items.
        tool::CommonMatchFinder matchHandler(matchCallback);
        MatchFinder finder;

        // Matches "lore::thunk::ProcFn" for the initialization of ProcAliasMaker.
        finder.addMatcher(
            classTemplateDecl(hasName("lore::thunk::ProcFn")).bind("procFnTemplateDecl"),
            &matchHandler);

        // Matches requested functions.
        finder.addMatcher(functionDecl().bind("functionDecl"), &matchHandler);

        // Matches requested variables.
        finder.addMatcher(varDecl().bind("varDecl"), &matchHandler);

        // Matches requested typedefs and type aliases.
        finder.addMatcher(typedefDecl().bind("typedefDecl"), &matchHandler);
        finder.addMatcher(typeAliasDecl().bind("typeAliasDecl"), &matchHandler);

        // Matches requested descriptions.
        finder.addMatcher(classTemplateSpecializationDecl(hasName("lore::thunk::ProcFnDesc"))
                              .bind("procFnDescDecl"),
                          &matchHandler);
        finder.addMatcher(classTemplateSpecializationDecl(hasName("lore::thunk::ProcCbDesc"))
                              .bind("procCbDescDecl"),
                          &matchHandler);

        // Matches requested definitions.
        finder.addMatcher(
            classTemplateSpecializationDecl(hasName("lore::thunk::ProcFn")).bind("procFnDecl"),
            &matchHandler);
        finder.addMatcher(
            classTemplateSpecializationDecl(hasName("lore::thunk::ProcCb")).bind("procCbDefl"),
            &matchHandler);

        finder.matchAST(ast);

        if (!procFnTemplateDecl) {
            llvm::errs() << "ProcFn template declaration not found.\n";
            std::exit(1);
        }

        m_procAliasMaker.initialize(procFnTemplateDecl);

        /// STEP: Filter requested items.
        for (const auto &[name, fd] : std::as_const(functionByName)) {
            for (int i = ProcSnippet::GuestToHost; i < ProcSnippet::NumDirections; ++i) {
                auto &srcMap = m_requestedProcData.functions[i];
                auto &dstMap = m_functionDecls[i];
                if (auto it = srcMap.find(name); it != srcMap.end()) {
                    dstMap[name] = fd;
                }
            }
        }
        for (const auto &[signature, info] : std::as_const(namedCallbackBySignature)) {
            auto it = m_requestedProcData.callbacks.find(signature);
            if (it == m_requestedProcData.callbacks.end()) {
                continue;
            }
            m_callbackTypes[signature] = {info.first, info.second};
        }

        for (const auto &[name, vd] : std::as_const(varByName)) {
            // TODO: Filter variables (Future work, not implemented now)
        }

        /// STEP: Add requested function ProcSnippet items.
        for (int i = ProcSnippet::GuestToHost; i < ProcSnippet::NumDirections; ++i) {
            auto &functionDecls = m_functionDecls[i];
            auto direction = static_cast<ProcSnippet::Direction>(i);
            for (const auto &[name, fd] : functionDecls) {
                std::optional<ProcSnippet::Desc> desc;
                if (auto it = functionDescByName.find(name); it != functionDescByName.end()) {
                    desc = it->second.second;
                }

                const auto &entryMap = procFnDeclByName[direction][ProcSnippet::Entry];
                const auto &callerMap = procFnDeclByName[direction][ProcSnippet::Caller];
                const auto entryIt = entryMap.find(name);
                const auto callerIt = callerMap.find(name);
                std::array<const ClassTemplateSpecializationDecl *, ProcSnippet::NumPhases>
                    definitions = {
                        entryIt != entryMap.end() ? entryIt->second.second : nullptr,
                        callerIt != callerMap.end() ? callerIt->second.second : nullptr,
                    };

                m_procs[ProcSnippet::Function][direction].emplace(
                    name, ProcSnippet(ProcSnippet::Function, direction, fd, name, std::move(desc),
                                      definitions, *this));
            }
        }

        /// STEP: Add requested callback ProcSnippet items.
        for (const auto &[signature, callbackInfo] : m_callbackTypes) {
            std::optional<ProcSnippet::Desc> desc;
            if (auto it = callbackDescBySignature.find(signature);
                it != callbackDescBySignature.end()) {
                desc = it->second.second;
            }

            auto key = callbackInfo.name.empty() ? signature : callbackInfo.name;
            for (int i = ProcSnippet::GuestToHost; i < ProcSnippet::NumDirections; ++i) {
                auto direction = static_cast<ProcSnippet::Direction>(i);
                const auto &entryMap = procCbDeclBySignature[direction][ProcSnippet::Entry];
                const auto &callerMap = procCbDeclBySignature[direction][ProcSnippet::Caller];
                const auto entryIt = entryMap.find(signature);
                const auto callerIt = callerMap.find(signature);
                std::array<const ClassTemplateSpecializationDecl *, ProcSnippet::NumPhases>
                    definitions = {
                        entryIt != entryMap.end() ? entryIt->second.second : nullptr,
                        callerIt != callerMap.end() ? callerIt->second.second : nullptr,
                    };

                m_procs[ProcSnippet::Callback][direction].emplace(
                    key, ProcSnippet(ProcSnippet::Callback, direction, callbackInfo.type, desc,
                                     definitions, *this));
            }
        }

        for (int i = Pass::Builder; i < Pass::NumPhases; ++i) {
            auto phase = static_cast<Pass::Phase>(i);
            for (auto &[_, pass] : m_passMaps[phase]) {
                if (pass) {
                    pass->handleTranslationUnit(*this);
                }
            }
        }
    }

    void DocumentContext::endSourceFileAction() {
        struct RunPassTask {
            ProcSnippet *proc = nullptr;
            Pass *pass = nullptr;
            std::unique_ptr<PassMessage> message;
        };
        llvm::SmallVector<RunPassTask, 256> runPassTasks;

        const auto appendPassTasks = [&](Pass::Phase phase) {
            auto &passMap = m_passMaps[phase];
            for (auto &[_, pass] : passMap) {
                if (!pass) {
                    continue;
                }

                for (int kind = ProcSnippet::Function; kind < ProcSnippet::NumKinds; ++kind) {
                    for (int direction = ProcSnippet::GuestToHost;
                         direction < ProcSnippet::NumDirections; ++direction) {
                        for (auto &[name, proc] : m_procs[kind][direction]) {
                            (void) name;
                            std::unique_ptr<PassMessage> msg;
                            if (pass->testProc(proc, msg)) {
                                runPassTasks.push_back(
                                    RunPassTask{&proc, pass, std::move(msg)});
                            }
                        }
                    }
                }
            }
        };

        // Begin document-level processing.
        for (int i = Pass::Builder; i < Pass::NumPhases; ++i) {
            auto phase = static_cast<Pass::Phase>(i);
            for (auto &[_, pass] : m_passMaps[phase]) {
                if (pass) {
                    pass->beginProcessDocument(*this);
                }
            }
        }

        // Select per-proc pass tasks.
        for (int i = Pass::Builder; i < Pass::NumPhases; ++i) {
            auto phase = static_cast<Pass::Phase>(i);
            appendPassTasks(phase);
        }

        // Run begin hooks in order.
        for (auto &task : runPassTasks) {
            if (auto err = task.pass->beginHandleProc(*task.proc, task.message)) {
                llvm::errs() << "error: in proc \"" << task.proc->name()
                             << "\", failed to begin pass @" << task.pass->name() << ": "
                             << llvm::toString(std::move(err)) << "\n";
                std::exit(1);
            }
        }

        // Run end hooks in reverse order.
        for (auto it = runPassTasks.rbegin(); it != runPassTasks.rend(); ++it) {
            if (auto err = it->pass->endHandleProc(*it->proc, it->message)) {
                llvm::errs() << "error: in proc \"" << it->proc->name()
                             << "\", failed to end pass @" << it->pass->name() << ": "
                             << llvm::toString(std::move(err)) << "\n";
                std::exit(1);
            }
        }

        // End document-level processing.
        for (int i = Pass::Builder; i < Pass::NumPhases; ++i) {
            auto phase = static_cast<Pass::Phase>(i);
            for (auto &[_, pass] : m_passMaps[phase]) {
                if (pass) {
                    pass->endProcessDocument(*this);
                }
            }
        }
    }

    void DocumentContext::generateOutput(llvm::raw_ostream &os) {
        const auto &legend = [](const std::string &name) {
            int eqCount = (75 - name.size()) / 2;
            return "// " + std::string(eqCount, '=') + " " + name + " " + std::string(eqCount, '=');
        };

        /// STEP: Add macro
        os << "#define LORE_THUNK_BUILD\n\n";

        /// STEP: Include pre-include file
        os << "#include \"" << std::filesystem::path(m_preIncludeFileName).filename().string()
           << "\"\n\n";

        /// STEP: Generate symbol declarations
        os << "extern \"C\" {\n";
        os << "#pragma GCC diagnostic push\n";
        os << "#pragma GCC diagnostic ignored \"-Wattribute-alias\"\n";
        if (m_mode == Host) {
            for (auto &[_, proc] : m_procs[ProcSnippet::Function][ProcSnippet::HostToGuest]) {
                os << "LORE_DECL_EXPORT "
                   << FunctionInfo(proc.functionDecl()).declText("GTL_" + proc.name(), *m_ast)
                   << "\n    __attribute__((alias(\""
                   << m_procAliasMaker.getInvokeAlias(
                          "HostToGuest", "Entry", const_cast<FunctionDecl *>(proc.functionDecl()),
                          proc.desc() ? proc.desc()->overlayType : std::nullopt)
                   << "\")));\n";
            }
        } else {
            for (auto &[_, proc] : m_procs[ProcSnippet::Function][ProcSnippet::GuestToHost]) {
                os << "LORETHUNK_EXPORT "
                   << FunctionInfo(proc.functionDecl()).declText(proc.name(), *m_ast)
                   << "\n    __attribute__((alias(\""
                   << m_procAliasMaker.getInvokeAlias(
                          "GuestToHost", "Entry", const_cast<FunctionDecl *>(proc.functionDecl()),
                          proc.desc() ? proc.desc()->overlayType : std::nullopt)
                   << "\")));\n";
            }
        }
        os << "#pragma GCC diagnostic pop\n";
        os << "}\n\n";

        /// STEP: Generate document head
        os << m_source.head.toRawText() << "\n";

        /// STEP: Generate macros
        os << "#define LORE_THUNK_FUNCTION_G2H_FOREACH(F)";
        for (const auto &[_, proc] : m_procs[ProcSnippet::Function][ProcSnippet::GuestToHost]) {
            os << " \\\n    F(" << proc.name() << ")";
        }
        os << "\n\n";
        os << "#define LORE_THUNK_FUNCTION_H2G_FOREACH(F)";
        for (const auto &[_, proc] : m_procs[ProcSnippet::Function][ProcSnippet::HostToGuest]) {
            os << " \\\n    F(" << proc.name() << ")";
        }
        os << "\n\n";

        os << "namespace lore::thunk {\n\n";
        for (const auto &[_, proc] : m_procs[ProcSnippet::Callback][ProcSnippet::GuestToHost]) {
            os << "using " << proc.name() << " = "
               << getTypeString(proc.functionPointerType().value()) << ";\n";
        }
        os << "\n";
        os << "}\n\n";

        os << "#define LORE_THUNK_CALLBACK_FOREACH(F)";
        for (const auto &[_, proc] : m_procs[ProcSnippet::Callback][ProcSnippet::GuestToHost]) {
            os << " \\\n    F(" << proc.name() << ", lore::thunk::" << proc.name() << ")";
        }
        os << "\n\n";

        /// STEP: Generate forward declarations
        os << "namespace lore::thunk {\n\n";

        for (int kind = ProcSnippet::Function; kind < ProcSnippet::NumKinds; ++kind) {
            for (int direction = ProcSnippet::GuestToHost; direction < ProcSnippet::NumDirections;
                 ++direction) {
                for (auto &[name, proc] : m_procs[kind][direction]) {
                    os << legend(proc.name()) << "\n";
                    if (!proc.hasDefinition(ProcSnippet::Caller)) {
                        os << proc.text(ProcSnippet::Caller, true) << "\n";
                    }
                    if (!proc.hasDefinition(ProcSnippet::Entry)) {
                        os << proc.text(ProcSnippet::Entry, true) << "\n";
                    }
                }
            }
        }

        os << "}\n\n";

        /// STEP: Include main source file
        os << "#include \"" << std::filesystem::path(m_mainFileName).filename().string()
           << "\"\n\n";

        /// STEP: Generate definitions
        os << "namespace lore::thunk {\n\n";

        for (int kind = ProcSnippet::Function; kind < ProcSnippet::NumKinds; ++kind) {
            for (int direction = ProcSnippet::GuestToHost; direction < ProcSnippet::NumDirections;
                 ++direction) {
                for (auto &[name, proc] : m_procs[kind][direction]) {
                    os << legend(proc.name()) << "\n";
                    if (!proc.hasDefinition(ProcSnippet::Caller)) {
                        os << proc.text(ProcSnippet::Caller, false) << "\n";
                    }
                    if (!proc.hasDefinition(ProcSnippet::Entry)) {
                        os << proc.text(ProcSnippet::Entry, false) << "\n";
                    }
                }
            }
        }

        os << "}\n\n";

        /// STEP: Generate document tail
        os << m_source.tail.toRawText() << "\n";

        /// STEP: Generate missing items in comments
        os << "//\n// Missing Function Declarations\n//\n";
        for (const auto &name :
             std::as_const(m_requestedProcData.functions[ProcSnippet::GuestToHost])) {
            auto &dstMap = m_functionDecls[ProcSnippet::GuestToHost];
            if (auto it = dstMap.find(name); it != dstMap.end()) {
                continue;
            }
            os << "// " << name << "\n";
        }
        os << "\n";

        os << "//\n// Missing Guest Function Declarations\n//\n";
        for (const auto &name :
             std::as_const(m_requestedProcData.functions[ProcSnippet::HostToGuest])) {
            auto &dstMap = m_functionDecls[ProcSnippet::HostToGuest];
            if (auto it = dstMap.find(name); it != dstMap.end()) {
                continue;
            }
            os << "// " << name << "\n";
        }
        os << "\n";

        os << "//\n// Missing Variable Declarations\n//\n";
        // TODO: future work
        os << "\n";

        /// STEP: Add manifest implementation
        if (m_mode == Host) {
            os << "#include <lorelei/Tools/ThunkInterface/Host/ManifestImpl.cpp.inc>\n";
        } else {
            os << "#include <lorelei/Tools/ThunkInterface/Guest/ManifestImpl.cpp.inc>\n";
        }

        os << "\n";
    }

}
