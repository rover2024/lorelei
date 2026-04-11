// SPDX-License-Identifier: MIT

#include <string>
#include <cstdlib>
#include <map>
#include <set>
#include <optional>

#include <clang/AST/ASTContext.h>
#include <clang/AST/Type.h>
#include <clang/AST/Expr.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/Program.h>
#include <llvm/Support/raw_ostream.h>

#include <lorelei/Base/Support/ConfigFile.h>
#include <lorelei/Tools/ToolUtils/CommonMatchFinder.h>
#include <lorelei/Tools/ToolUtils/TypeUtils.h>
#include <lorelei/Tools/TLCApi/Utils/ManifestStatistics.h>

using namespace clang;
using namespace clang::tooling;
using namespace clang::ast_matchers;

namespace cl = llvm::cl;

namespace lore::tool::command::stat {

    struct GlobalContext {
        std::string outputPath;
        std::string configPath;
        lore::ConfigFile config;
        TLC::ManifestStatistics stat;
    };

    static GlobalContext &g_ctx() {
        static GlobalContext instance;
        return instance;
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
        if (const auto *DRE = dyn_cast<DeclRefExpr>(expr)) {
            return dyn_cast<FunctionDecl>(DRE->getDecl());
        }
        if (const auto *UO = dyn_cast<UnaryOperator>(expr)) {
            if (UO->getOpcode() == UO_AddrOf) {
                if (const auto *DRE =
                        dyn_cast<DeclRefExpr>(UO->getSubExpr()->IgnoreParenImpCasts())) {
                    return dyn_cast<FunctionDecl>(DRE->getDecl());
                }
            }
        }
        return nullptr;
    }

    static std::string sourceLocationText(SourceManager &sm, SourceLocation loc) {
        loc = sm.getSpellingLoc(loc);
        return loc.printToString(sm);
    }

    static std::optional<QualType> extractOverlayType(const ClassTemplateSpecializationDecl &decl,
                                                      ASTContext &ast) {
        for (const auto *subDecl : decl.decls()) {
            const TypedefNameDecl *td = dyn_cast<TypedefNameDecl>(subDecl);
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

    static std::optional<std::string> extractNameHint(const ClassTemplateSpecializationDecl &decl) {
        for (const auto *subDecl : decl.decls()) {
            const auto *vd = dyn_cast<VarDecl>(subDecl);
            if (!vd || !vd->isStaticDataMember() || vd->getName() != "name") {
                continue;
            }

            const Expr *initExpr = vd->getInit();
            if (!initExpr) {
                continue;
            }

            initExpr = initExpr->IgnoreParenImpCasts();
            if (const auto *constExpr = dyn_cast<ConstantExpr>(initExpr)) {
                initExpr = constExpr->getSubExpr()->IgnoreParenImpCasts();
            }
            if (const auto *str = dyn_cast<StringLiteral>(initExpr)) {
                return str->getString().str();
            }
        }
        return std::nullopt;
    }

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

    class MyASTConsumer : public ASTConsumer {
    public:
        void HandleTranslationUnit(ASTContext &ast) override {
            if (ast.getLangOpts().CPlusPlus) {
                llvm::errs() << "error: TLC stat only supports C input. "
                                "Use C language flags (e.g. -xc -std=c11).\n";
                std::exit(1);
            }

            m_ast = &ast;
            auto &sm = ast.getSourceManager();

            const auto matchCallback = [this, &sm](const MatchFinder::MatchResult &result) {
                if (const auto *decl =
                        result.Nodes.getNodeAs<ClassTemplateSpecializationDecl>("procFnDescDecl")) {
                    handleProcFnDesc(*decl, sm);
                    return;
                }

                if (const auto *decl =
                        result.Nodes.getNodeAs<ClassTemplateSpecializationDecl>("procCbDescDecl")) {
                    handleProcCbDesc(*decl, sm);
                    return;
                }

                if (const auto *decl = result.Nodes.getNodeAs<TypedefDecl>("typedefDecl")) {
                    if (!isCDecl(decl)) {
                        return;
                    }

                    auto type = decl->getUnderlyingType().getCanonicalType();
                    if (!type->isFunctionPointerType()) {
                        return;
                    }
                    m_functionPointerTypedefTypeMap[decl->getNameAsString()] = type;
                    m_namedCallbackAliasMap[getTypeString(type)] = decl->getNameAsString();
                    return;
                }

                if (const auto *decl = result.Nodes.getNodeAs<TypeAliasDecl>("typeAliasDecl")) {
                    if (!isCDecl(decl)) {
                        return;
                    }

                    auto type = decl->getUnderlyingType().getCanonicalType();
                    if (!type->isFunctionPointerType()) {
                        return;
                    }
                    m_functionPointerTypedefTypeMap[decl->getNameAsString()] = type;
                    m_namedCallbackAliasMap[getTypeString(type)] = decl->getNameAsString();
                    return;
                }

                if (const auto *decl = result.Nodes.getNodeAs<FunctionDecl>("functionDecl")) {
                    if (!isCLinkage(decl)) {
                        return;
                    }
                    m_functionMap[decl->getNameAsString()] = decl;
                    return;
                }
            };

            tool::CommonMatchFinder matchHandler(matchCallback);
            MatchFinder finder;
            finder.addMatcher(classTemplateSpecializationDecl(hasName("lore::thunk::ProcFnDesc"))
                                  .bind("procFnDescDecl"),
                              &matchHandler);
            finder.addMatcher(classTemplateSpecializationDecl(hasName("lore::thunk::ProcCbDesc"))
                                  .bind("procCbDescDecl"),
                              &matchHandler);
            finder.addMatcher(typedefDecl().bind("typedefDecl"), &matchHandler);
            finder.addMatcher(typeAliasDecl().bind("typeAliasDecl"), &matchHandler);
            finder.addMatcher(functionDecl().bind("functionDecl"), &matchHandler);
            finder.matchAST(ast);

            collectConfigSeeds();
        }

    private:
        struct ProcFnDescData {
            const FunctionDecl *funcDecl = nullptr;
            std::optional<QualType> overlayType;
            std::string declLocation;
        };

        void handleProcFnDesc(const ClassTemplateSpecializationDecl &decl, SourceManager &sm) {
            if (decl.getTemplateSpecializationKind() != TSK_ExplicitSpecialization) {
                return;
            }

            auto tArgs = decl.getTemplateArgs().asArray();
            if (tArgs.size() != 1) {
                return;
            }

            const auto *funcDecl = extractFunctionDeclFromTemplateArg(tArgs[0]);
            if (!funcDecl) {
                return;
            }

            ProcFnDescData data;
            data.funcDecl = funcDecl;
            data.overlayType = extractOverlayType(decl, *m_ast);
            data.declLocation = sourceLocationText(sm, decl.getLocation());
            m_procFnDescDataMap[funcDecl->getNameAsString()] = std::move(data);
        }

        void handleProcCbDesc(const ClassTemplateSpecializationDecl &decl, SourceManager &sm) {
            if (decl.getTemplateSpecializationKind() != TSK_ExplicitSpecialization) {
                return;
            }

            auto tArgs = decl.getTemplateArgs().asArray();
            if (tArgs.size() != 1) {
                llvm::errs() << "warning: unexpected ProcCbDesc template arity at "
                             << sourceLocationText(sm, decl.getLocation()) << "\n";
                return;
            }
            const auto &arg = tArgs[0];
            if (arg.getKind() != TemplateArgument::Type) {
                llvm::errs() << "warning: invalid ProcCbDesc type argument at "
                             << sourceLocationText(sm, decl.getLocation()) << "\n";
                return;
            }

            auto type = arg.getAsType();
            std::string alias;
            if (auto nameHint = extractNameHint(decl)) {
                alias = *nameHint;
            } else if (const auto *typedefType = type->getAs<TypedefType>()) {
                alias = typedefType->getDecl()->getNameAsString();
            }

            if (type->isFunctionType()) {
                type = m_ast->getPointerType(type);
            }
            type = type.getCanonicalType();
            if (!type->isFunctionPointerType()) {
                llvm::errs() << "warning: ProcCbDesc argument is not a function pointer at "
                             << sourceLocationText(sm, decl.getLocation()) << "\n";
                return;
            }

            auto signature = getTypeString(type);
            collectCallbackSignatures(type, "ProcCbDesc<" + signature + ">", alias);

            if (auto overlayType = extractOverlayType(decl, *m_ast)) {
                collectCallbackSignatures(*overlayType,
                                          "ProcCbDesc<" + getTypeString(type) + ">@overlay");
            }
        }

        void collectCallbackSignatures(const QualType &seedType, const std::string &seedOrigin,
                                       const std::string &preferredAlias = {},
                                       bool includeSeed = true) {
            llvm::SmallVector<std::pair<QualType, std::string>, 32> stack;
            if (includeSeed) {
                stack.push_back(std::make_pair(seedType, seedOrigin));
            } else {
                auto type = seedType.getCanonicalType();
                if (type->isFunctionPointerType()) {
                    auto pointee = type->getPointeeType().getCanonicalType();
                    if (const auto *fpt = pointee->getAs<FunctionProtoType>()) {
                        if (!fpt->getReturnType()->isVoidType()) {
                            stack.push_back(
                                std::make_pair(fpt->getReturnType(), seedOrigin + "@ret"));
                        }
                        for (size_t i = 0; i < fpt->getNumParams(); ++i) {
                            stack.push_back(std::make_pair(
                                fpt->getParamType(i), seedOrigin + "@arg" + std::to_string(i + 1)));
                        }
                    } else if (const auto *fnt = pointee->getAs<FunctionNoProtoType>()) {
                        if (!fnt->getReturnType()->isVoidType()) {
                            stack.push_back(
                                std::make_pair(fnt->getReturnType(), seedOrigin + "@ret"));
                        }
                    }
                } else {
                    stack.push_back(std::make_pair(type, seedOrigin));
                }
            }

            std::set<std::string> visited;
            while (!stack.empty()) {
                auto [type, origin] = stack.pop_back_val();
                type = type.getCanonicalType();

                auto typeStr = getTypeString(type);
                if (!visited.insert(typeStr).second) {
                    continue;
                }

                if (type->isFunctionPointerType()) {
                    std::string alias;
                    if (!preferredAlias.empty() && origin == seedOrigin) {
                        alias = preferredAlias;
                    } else {
                        if (auto it = m_namedCallbackAliasMap.find(typeStr);
                            it != m_namedCallbackAliasMap.end()) {
                            alias = it->second;
                        }
                    }
                    g_ctx().stat.addCallbackSignature(typeStr, origin, alias);

                    auto pointee = type->getPointeeType().getCanonicalType();
                    if (const auto *fpt = pointee->getAs<FunctionProtoType>()) {
                        if (!fpt->getReturnType()->isVoidType()) {
                            stack.push_back(std::make_pair(fpt->getReturnType(), origin + "@ret"));
                        }
                        for (size_t i = 0; i < fpt->getNumParams(); ++i) {
                            stack.push_back(std::make_pair(
                                fpt->getParamType(i), origin + "@arg" + std::to_string(i + 1)));
                        }
                    } else if (const auto *fnt = pointee->getAs<FunctionNoProtoType>()) {
                        if (!fnt->getReturnType()->isVoidType()) {
                            stack.push_back(std::make_pair(fnt->getReturnType(), origin + "@ret"));
                        }
                    }
                    continue;
                }

                if (type->isPointerType()) {
                    stack.push_back(std::make_pair(type->getPointeeType(), origin + "@ptr"));
                    continue;
                }

                if (type->isArrayType()) {
                    stack.push_back(std::make_pair(type->getAsArrayTypeUnsafe()->getElementType(),
                                                   origin + "@array"));
                    continue;
                }

                if (const auto *recType = type->getAs<RecordType>()) {
                    for (const auto *field : recType->getDecl()->fields()) {
                        stack.push_back(std::make_pair(field->getType(),
                                                       origin + "@" + field->getNameAsString()));
                    }
                    continue;
                }
            }
        }

        void collectConfigFunctionSection(StringRef sectionName,
                                          TLC::ManifestStatistics::FunctionDirection direction) {
            auto sectionOpt = g_ctx().config.get(sectionName.str());
            if (!sectionOpt) {
                return;
            }
            auto &sm = m_ast->getSourceManager();

            for (const auto &[name, _] : sectionOpt->get()) {
                (void) _;
                auto it = m_functionMap.find(name);
                if (it == m_functionMap.end()) {
                    continue;
                }
                const FunctionDecl *fd = it->second;
                auto funcPtrType = m_ast->getPointerType(fd->getType().getCanonicalType());
                g_ctx().stat.addFunction(direction, fd->getNameAsString(),
                                         getTypeString(funcPtrType),
                                         sourceLocationText(sm, fd->getBeginLoc()));
                collectCallbackSignatures(funcPtrType, "Config[" + sectionName.str() + "]:" + name,
                                          {}, false);

                if (auto dataIt = m_procFnDescDataMap.find(name);
                    dataIt != m_procFnDescDataMap.end()) {
                    if (dataIt->second.overlayType) {
                        collectCallbackSignatures(
                            *dataIt->second.overlayType,
                            "Config[" + sectionName.str() + "]:" + name + "@overlay", {}, false);
                    }
                }
            }
        }

        void collectConfigCallbackSection() {
            auto sectionOpt = g_ctx().config.get("Callback");
            if (!sectionOpt) {
                return;
            }

            for (const auto &[token, _] : sectionOpt->get()) {
                (void) _;
                if (token.empty()) {
                    continue;
                }

                if (token.front() == '&') {
                    auto name = token.substr(1);
                    auto it = m_functionMap.find(name);
                    if (it == m_functionMap.end()) {
                        continue;
                    }
                    auto fpType = m_ast->getPointerType(it->second->getType().getCanonicalType());
                    std::string alias;
                    if (auto mapIt = m_namedCallbackAliasMap.find(getTypeString(fpType));
                        mapIt != m_namedCallbackAliasMap.end()) {
                        alias = mapIt->second;
                    } else {
                        alias = "PFN_AUTO_" + name;
                    }
                    collectCallbackSignatures(fpType, "Config[Callback]:" + token, alias);
                    continue;
                }

                auto it = m_functionPointerTypedefTypeMap.find(token);
                if (it == m_functionPointerTypedefTypeMap.end()) {
                    continue;
                }
                collectCallbackSignatures(it->second, "Config[Callback]:" + token, token);
            }
        }

        void collectConfigSeeds() {
            collectConfigFunctionSection("Function", TLC::ManifestStatistics::GuestToHost);
            collectConfigFunctionSection("Guest Function", TLC::ManifestStatistics::HostToGuest);
            collectConfigCallbackSection();
        }

    private:
        ASTContext *m_ast = nullptr;
        std::map<std::string, ProcFnDescData> m_procFnDescDataMap;
        std::map<std::string, const FunctionDecl *> m_functionMap;
        std::map<std::string, QualType> m_functionPointerTypedefTypeMap;
        std::map<std::string, std::string> m_namedCallbackAliasMap;
    };

    class MyASTFrontendAction : public ASTFrontendAction {
    public:
        std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &, StringRef) override {
            return std::make_unique<MyASTConsumer>();
        }
    };

    const char *name = "stat";

    const char *help = "Probe statistics of input files";

    int main(int argc, char *argv[]) {
        static cl::OptionCategory myOptionCat("Lorelei TLC - Statistics");
        static cl::opt<std::string> configOption("c", cl::desc("Specify thunk config file"),
                                                 cl::value_desc("config file"),
                                                 cl::cat(myOptionCat));
        static cl::opt<std::string> outputOption("o", cl::desc("Specify output file"),
                                                 cl::value_desc("output file"),
                                                 cl::cat(myOptionCat));
        static cl::extrahelp commonHelp(CommonOptionsParser::HelpMessage);

        auto expectedParser = CommonOptionsParser::create(argc, const_cast<const char **>(argv),
                                                          myOptionCat, cl::Required);
        if (!expectedParser) {
            llvm::errs() << expectedParser.takeError();
            return 0;
        }
        auto &parser = expectedParser.get();

        g_ctx().stat.clear();
        g_ctx().configPath = configOption.getValue();
        g_ctx().outputPath = outputOption.getValue();
        if (!parser.getSourcePathList().empty()) {
            g_ctx().stat.fileName = parser.getSourcePathList().front();
        }

        if (!g_ctx().configPath.empty()) {
            auto result = g_ctx().config.load(g_ctx().configPath);
            if (!result.success) {
                llvm::errs() << "Failed to load config file " << g_ctx().configPath << ": "
                             << result.errorMessage;
                if (!result.file.empty()) {
                    llvm::errs() << " (" << result.file.string() << ":" << result.line << ")";
                }
                llvm::errs() << "\n";
                return 1;
            }
        }

        ClangTool tool(parser.getCompilations(), parser.getSourcePathList());
        if (int ret = tool.run(newFrontendActionFactory<MyASTFrontendAction>().get()); ret != 0) {
            return ret;
        }

        if (std::string err;
            !g_ctx().stat.saveAsJson(g_ctx().outputPath.empty() ? "-" : g_ctx().outputPath, err)) {
            llvm::errs() << "Failed to write statistics file: " << err << "\n";
            return 1;
        }
        return 0;
    }

}
