// SPDX-License-Identifier: MIT

#include <string>
#include <optional>
#include <map>
#include <vector>
#include <algorithm>

#include <llvm/Support/CommandLine.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/AST/Expr.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>

#include <lorelei/Tools/TLCApi/Core/Pass.h>
#include <lorelei/Tools/TLCApi/Core/DocumentContext.h>
#include <lorelei/Tools/TLCApi/Core/ProcContext.h>
#include <lorelei/Tools/TLCApi/Utils/ManifestStatistics.h>
#include <lorelei/Tools/ToolUtils/CommonMatchFinder.h>
#include <lorelei/Tools/ToolUtils/TypeUtils.h>

using namespace clang;
using namespace clang::tooling;
using namespace clang::ast_matchers;

namespace cl = llvm::cl;

namespace lore::tool::command::generate {

    const char *name = "generate";

    const char *help = "Generate thunk library sources for input files";

    enum GenerateMode {
        GenerateMode_Guest,
        GenerateMode_Host,
    };

    struct PassConfig {
        int builderPassID = -1;
        std::vector<int> passIDs;
    };

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
                if (const auto *dre = dyn_cast<DeclRefExpr>(uo->getSubExpr()->IgnoreParenImpCasts())) {
                    return dyn_cast<FunctionDecl>(dre->getDecl());
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

    static void appendPassIDsFromPassListType(QualType listType, ASTContext &ast,
                                              std::vector<int> &out) {
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
            if (auto passID = extractPassIDFromType(arg.getAsType(), ast)) {
                if (std::find(out.begin(), out.end(), *passID) == out.end()) {
                    out.push_back(*passID);
                }
            }
        }
    }

    static PassConfig extractPassConfigFromProcDesc(const ClassTemplateSpecializationDecl &decl) {
        PassConfig config;
        for (const auto *subDecl : decl.decls()) {
            const auto *vd = dyn_cast<VarDecl>(subDecl);
            if (!vd || !vd->isStaticDataMember()) {
                continue;
            }

            auto memberName = vd->getName();
            if (memberName == "builder_pass" || memberName == "BUILDER_PASS") {
                if (auto passID = extractPassIDFromType(vd->getType(), decl.getASTContext())) {
                    config.builderPassID = *passID;
                }
                continue;
            }
            if (memberName == "passes" || memberName == "MISC_PASSES") {
                appendPassIDsFromPassListType(vd->getType(), decl.getASTContext(), config.passIDs);
                continue;
            }
        }
        return config;
    }

    class ProcDescCollectorASTConsumer : public ASTConsumer {
    public:
        ProcDescCollectorASTConsumer(std::map<std::string, PassConfig> &functionPassConfigByName,
                                     std::map<std::string, PassConfig> &callbackPassConfigBySignature)
            : m_functionPassConfigByName(functionPassConfigByName),
              m_callbackPassConfigBySignature(callbackPassConfigBySignature) {
        }

        void HandleTranslationUnit(ASTContext &ast) override {
            const auto matchCallback = [this, &ast](const MatchFinder::MatchResult &result) {
                if (const auto *decl =
                        result.Nodes.getNodeAs<ClassTemplateSpecializationDecl>("procFnDescDecl")) {
                    auto tArgs = decl->getTemplateArgs().asArray();
                    if (tArgs.size() != 1) {
                        return;
                    }
                    const auto *fd = extractFunctionDeclFromTemplateArg(tArgs[0]);
                    if (!fd) {
                        return;
                    }
                    m_functionPassConfigByName[fd->getNameAsString()] =
                        extractPassConfigFromProcDesc(*decl);
                    return;
                }

                if (const auto *decl =
                        result.Nodes.getNodeAs<ClassTemplateSpecializationDecl>("procCbDescDecl")) {
                    auto tArgs = decl->getTemplateArgs().asArray();
                    if (tArgs.size() != 1 || tArgs[0].getKind() != TemplateArgument::Type) {
                        return;
                    }

                    auto type = tArgs[0].getAsType();
                    if (type->isFunctionType()) {
                        type = ast.getPointerType(type);
                    }
                    type = type.getCanonicalType();
                    if (!type->isFunctionPointerType()) {
                        return;
                    }

                    m_callbackPassConfigBySignature[getTypeString(type)] =
                        extractPassConfigFromProcDesc(*decl);
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
            finder.matchAST(ast);
        }

    private:
        std::map<std::string, PassConfig> &m_functionPassConfigByName;
        std::map<std::string, PassConfig> &m_callbackPassConfigBySignature;
    };

    class ProcDescCollectorFrontendAction : public ASTFrontendAction {
    public:
        ProcDescCollectorFrontendAction(std::map<std::string, PassConfig> &functionPassConfigByName,
                                        std::map<std::string, PassConfig> &callbackPassConfigBySignature)
            : m_functionPassConfigByName(functionPassConfigByName),
              m_callbackPassConfigBySignature(callbackPassConfigBySignature) {
        }

        std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &, StringRef) override {
            return std::make_unique<ProcDescCollectorASTConsumer>(m_functionPassConfigByName,
                                                                  m_callbackPassConfigBySignature);
        }

    private:
        std::map<std::string, PassConfig> &m_functionPassConfigByName;
        std::map<std::string, PassConfig> &m_callbackPassConfigBySignature;
    };

    class ProcDescCollectorFrontendActionFactory : public FrontendActionFactory {
    public:
        ProcDescCollectorFrontendActionFactory(
            std::map<std::string, PassConfig> &functionPassConfigByName,
            std::map<std::string, PassConfig> &callbackPassConfigBySignature)
            : m_functionPassConfigByName(functionPassConfigByName),
              m_callbackPassConfigBySignature(callbackPassConfigBySignature) {
        }

        std::unique_ptr<FrontendAction> create() override {
            return std::make_unique<ProcDescCollectorFrontendAction>(
                m_functionPassConfigByName, m_callbackPassConfigBySignature);
        }

    private:
        std::map<std::string, PassConfig> &m_functionPassConfigByName;
        std::map<std::string, PassConfig> &m_callbackPassConfigBySignature;
    };

    static bool collectProcDescPassConfigs(CommonOptionsParser &parser,
                                           std::map<std::string, PassConfig> &functionPassConfigByName,
                                           std::map<std::string, PassConfig> &callbackPassConfigBySignature) {
        ClangTool tool(parser.getCompilations(), parser.getSourcePathList());
        ProcDescCollectorFrontendActionFactory factory(functionPassConfigByName,
                                                       callbackPassConfigBySignature);
        return tool.run(&factory) == 0;
    }

    static void applyProcDescPassConfigs(TLC::DocumentContext &doc,
                                         const std::map<std::string, PassConfig> &functionPassConfigByName,
                                         const std::map<std::string, PassConfig> &callbackPassConfigBySignature) {
        for (auto &proc : doc.procs) {
            if (proc.kind == TLC::ProcContext::Function) {
                auto it = functionPassConfigByName.find(proc.name);
                if (it == functionPassConfigByName.end()) {
                    continue;
                }
                proc.builderPassID = it->second.builderPassID;
                proc.passIDs = it->second.passIDs;
                continue;
            }

            auto it = callbackPassConfigBySignature.find(proc.signature);
            if (it == callbackPassConfigBySignature.end()) {
                continue;
            }
            proc.builderPassID = it->second.builderPassID;
            proc.passIDs = it->second.passIDs;
        }
    }

    static void appendProcContextsFromStat(TLC::DocumentContext &doc,
                                           const TLC::ManifestStatistics &stat) {
        for (int i = TLC::ManifestStatistics::GuestToHost;
             i < TLC::ManifestStatistics::NumFunctionDirection; ++i) {
            auto direction = static_cast<TLC::ManifestStatistics::FunctionDirection>(i);
            const auto &functionMap = stat.functions[direction];
            for (const auto &[name, info] : functionMap) {
                TLC::ProcContext proc;
                proc.kind = TLC::ProcContext::Function;
                proc.direction = direction;
                proc.name = name;
                proc.signature = info.signature;
                proc.location = info.location;
                doc.procs.push_back(std::move(proc));
            }
        }

        const auto callbackDirection =
            doc.mode == TLC::DocumentContext::Guest ? TLC::ManifestStatistics::HostToGuest
                                                    : TLC::ManifestStatistics::GuestToHost;
        for (const auto &[signature, info] : stat.callbacks) {
            std::ignore = signature;
            TLC::ProcContext proc;
            proc.kind = TLC::ProcContext::Callback;
            proc.direction = callbackDirection;
            proc.name = info.alias;
            proc.signature = info.signature;
            proc.alias = info.alias;
            proc.location = info.origin;
            doc.procs.push_back(std::move(proc));
        }
    }

    static llvm::Error runRegisteredPasses(TLC::DocumentContext &doc, size_t &executedPassCount) {
        executedPassCount = 0;
        constexpr TLC::Pass::Phase phases[] = {TLC::Pass::Builder, TLC::Pass::Guard,
                                               TLC::Pass::Misc};

        for (auto phase : phases) {
            auto &passMap = TLC::Pass::passMap(phase);
            for (auto &[id, pass] : passMap) {
                (void) id;
                if (!pass) {
                    continue;
                }
                ++executedPassCount;
                pass->beginProcessDocument(doc);

                for (auto &proc : doc.procs) {
                    if (!proc.hasPassID(pass->id())) {
                        continue;
                    }

                    std::unique_ptr<TLC::ProcMessage> msg;
                    if (!pass->testProc(proc, msg)) {
                        continue;
                    }
                    if (auto err = pass->beginHandleProc(proc, msg)) {
                        return err;
                    }
                    if (auto err = pass->endHandleProc(proc, msg)) {
                        return err;
                    }
                }

                pass->endProcessDocument(doc);
            }
        }

        return llvm::Error::success();
    }

    int main(int argc, char *argv[]) {
        static cl::OptionCategory myOptionCat("Lorelei TLC - Generate");
        static cl::opt<GenerateMode> modeOption(
            "m", cl::desc("Generate mode"),
            cl::values(
                clEnumValN(GenerateMode_Guest, "guest", "Generate guest thunk source"),
                clEnumValN(GenerateMode_Host, "host", "Generate host thunk source")),
            cl::cat(myOptionCat), cl::Required);
        static cl::opt<std::string> statOption(
            "s", cl::desc("Specify TLC stat JSON file"), cl::value_desc("stat json"),
            cl::cat(myOptionCat), cl::Required);
        static cl::opt<std::string> outputOption(
            "o", cl::desc("Specify output file"), cl::value_desc("output file"),
            cl::cat(myOptionCat));
        static cl::extrahelp commonHelp(CommonOptionsParser::HelpMessage);

        auto expectedParser = CommonOptionsParser::create(
            argc, const_cast<const char **>(argv), myOptionCat, cl::Required);
        if (!expectedParser) {
            llvm::errs() << expectedParser.takeError();
            return 1;
        }
        auto &parser = expectedParser.get();

        const auto &sourcePathList = parser.getSourcePathList();
        auto mode = modeOption.getValue();

        TLC::ManifestStatistics stat;
        if (std::string err; !stat.loadFromJson(statOption.getValue(), err)) {
            llvm::errs() << "error: failed to parse stat json: " << err << "\n";
            return 1;
        }

        TLC::DocumentContext doc;
        doc.fileName = sourcePathList.front();
        doc.mode = mode == GenerateMode_Guest ? TLC::DocumentContext::Guest
                                              : TLC::DocumentContext::Host;
        doc.manifestStat = &stat;
        appendProcContextsFromStat(doc, stat);
        std::map<std::string, PassConfig> functionPassConfigByName;
        std::map<std::string, PassConfig> callbackPassConfigBySignature;
        if (!collectProcDescPassConfigs(parser, functionPassConfigByName,
                                        callbackPassConfigBySignature)) {
            llvm::errs() << "error: failed to parse ProcDesc pass metadata from input file.\n";
            return 1;
        }
        applyProcDescPassConfigs(doc, functionPassConfigByName, callbackPassConfigBySignature);

        size_t executedPassCount = 0;
        if (auto err = runRegisteredPasses(doc, executedPassCount)) {
            llvm::errs() << "error: pass execution failed: " << llvm::toString(std::move(err))
                         << "\n";
            return 1;
        }

        std::error_code ec;
        auto outputPath = outputOption.getValue().empty() ? std::string("-")
                                                           : outputOption.getValue();
        llvm::raw_fd_ostream out(outputPath, ec, llvm::sys::fs::OF_Text);
        if (ec) {
            llvm::errs() << "error: failed to open output file: " << ec.message() << "\n";
            return 1;
        }

        out << "// SPDX-License-Identifier: MIT\n";
        out << "// Auto-generated by LoreTLC generate\n";
        out << "// mode: "
            << (mode == GenerateMode_Guest ? "guest (Thunk_manifest_guest.cpp)"
                                           : "host (Thunk_manifest_host.cpp)")
            << "\n";
        out << "// stat: " << statOption.getValue() << "\n";
        out << "// stat.fileName: " << stat.fileName << "\n";
        out << "// input.fileName: " << doc.fileName << "\n";
        out << "// functions.GuestToHost: " << stat.functions[TLC::ManifestStatistics::GuestToHost].size()
            << "\n";
        out << "// functions.HostToGuest: " << stat.functions[TLC::ManifestStatistics::HostToGuest].size()
            << "\n";
        out << "// callbacks: " << stat.callbacks.size() << "\n\n";
        out << "// pass.executedCount: " << executedPassCount << "\n";
        out << "// proc.count: " << doc.procs.size() << "\n\n";
        out << "/* TODO: implement TLC generate */\n";

        if (out.has_error()) {
            llvm::errs() << "error: failed while writing output file.\n";
            return 1;
        }
        return 0;
    }

}
