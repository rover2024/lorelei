#include <filesystem>
#include <map>
#include <vector>
#include <cstdlib>

#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Rewrite/Core/Rewriter.h>

#include <llvm/Support/CommandLine.h>
#include <llvm/Support/SHA256.h>
#include <llvm/Support/MemoryBuffer.h>

#include <json11/json11.hpp>

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;

namespace cl = llvm::cl;
namespace fs = std::filesystem;

class FPExprConfig {
public:
    std::set<std::string> Functions;

    static FPExprConfig parse(const std::string &path) {
        auto buffer = llvm::MemoryBuffer::getFile(path);
        if (std::error_code EC = buffer.getError()) {
            llvm::errs() << path << ": failed to open file: " << EC.message() << "\n";
            std::abort();
        }

        std::string jsonErr;
        auto json = json11::Json::parse(buffer->get()->getBuffer().str(), jsonErr);
        if (!jsonErr.empty()) {
            llvm::errs() << path << ": failed to parse file: " << jsonErr << "\n";
            std::abort();
        }
        if (!json.is_object()) {
            llvm::errs() << path << ": not an object\n";
            std::abort();
        }
        const auto &docObj = json.object_items();

        FPExprConfig config;
        if (auto it = docObj.find("functions"); it != docObj.end()) {
            const auto &functionsVal = it->second;
            if (functionsVal.is_array()) {
                const auto &functionsArr = functionsVal.array_items();
                for (const auto &item : functionsArr) {
                    if (!item.is_string()) {
                        continue;
                    }
                    const auto &name = item.string_value();
                    if (name.empty()) {
                        continue;
                    }
                    config.Functions.insert(name);
                }
            }
        }
        return config;
    }
};

class FPExprGlobalContext {
public:
    FPExprConfig Config;
    std::string OutputPath;
};

static FPExprGlobalContext GlobalContext;

// 1. Match handler
class FPExprHandler : public MatchFinder::MatchCallback {
public:
    FPExprHandler(std::vector<const CallExpr *> &Expressions) : Expressions(Expressions) {}

    virtual void run(const MatchFinder::MatchResult &Result) override {
        const CallExpr *call = Result.Nodes.getNodeAs<CallExpr>("call");
        if (!call)
            return;

        const Expr *calleeExpr = call->getCallee()->IgnoreImpCasts();
        if (!calleeExpr)
            return;

        QualType type = calleeExpr->getType();
        if (!type->isFunctionPointerType() && !type->isBlockPointerType() && !type->isMemberFunctionPointerType())
            return;

        Expressions.push_back(call);
    }

private:
    std::vector<const CallExpr *> &Expressions;
};

// 2. AST consumer
class FPExprConsumer : public ASTConsumer {
public:
    FPExprConsumer(std::vector<const CallExpr *> &Expressions) : Expressions(Expressions), Handler(Expressions) {
        Matcher.addMatcher(callExpr().bind("call"), &Handler);
    }

    void HandleTranslationUnit(ASTContext &Context) override {
        Matcher.matchAST(Context); //
    }

private:
    std::vector<const CallExpr *> &Expressions;
    MatchFinder Matcher;
    FPExprHandler Handler;
};

// 3. AST frontend action
class FPExprAction : public ASTFrontendAction {
public:
    FPExprAction() {}

    void EndSourceFileAction() override {
        SourceManager &SM = Rewrite.getSourceMgr();
        LangOptions LangOpts = Rewrite.getLangOpts();
        ASTContext &Context = CI->getASTContext();

        class CFIInfo {
        public:
            QualType Type;
            std::string Hash;
            std::string Signature;
        };

        class Insertion {
        public:
            SourceLocation Location;
            std::string Text;
            bool InsertAfter = false;
        };
        std::map<std::string, CFIInfo> CFIInfoMap; // signature -> info
        std::vector<Insertion> InsertionList;

        int hashCnt = 0;
        for (const CallExpr *call : std::as_const(Expressions)) {
            // Ignore expressions in header files
            if (!SM.isInMainFile(call->getBeginLoc())) {
                continue;
            }

            // Get callee
            auto calleeExpr = call->getCallee()->IgnoreImpCasts();
            SourceRange calleeRange = calleeExpr->getSourceRange();
            if (calleeRange.isInvalid()) {
                continue;
            }

            // Fix range
            auto fixedRange = calleeRange;
            fixedRange.setEnd(Lexer::getLocForEndOfToken(fixedRange.getEnd(), 0, SM, LangOpts));

            // Consider case of passing arguments to a function with no prototype
            auto type = calleeExpr->getType().getCanonicalType()->getPointeeType();
            if (type->isFunctionNoProtoType() && call->getNumArgs() > 0) {
                auto funcType = type->getAs<clang::FunctionNoProtoType>();
                SmallVector<QualType, 4> argTypes;
                for (unsigned i = 0; i < call->getNumArgs(); ++i) {
                    argTypes.push_back(call->getArg(i)->getType());
                }

                QualType newFuncType =
                    Context.getFunctionType(funcType->getReturnType(), llvm::ArrayRef<QualType>(argTypes), {})
                        .getCanonicalType();

                // Replace with the real function signature
                type = newFuncType;
            }

            std::string signature = type.getAsString();
            if (!GlobalContext.Config.Functions.empty() && !GlobalContext.Config.Functions.count(signature)) {
                continue;
            }

            std::string hashStr;
            if (auto it = CFIInfoMap.find(signature); it == CFIInfoMap.end()) {
                hashStr = std::to_string(++hashCnt);
                CFIInfoMap.insert(std::make_pair(signature, CFIInfo{type, hashStr, signature}));
            } else {
                hashStr = it->second.Hash;
            }
            InsertionList.push_back({fixedRange.getBegin(), "LORELIB_CFI_" + hashStr + "(", false});
            InsertionList.push_back({fixedRange.getEnd(), ")", true});
        }

        std::error_code EC;
        llvm::raw_fd_ostream out(GlobalContext.OutputPath.empty() ? "-" : GlobalContext.OutputPath, EC);
        if (EC) {
            llvm::errs() << "Error occurs opening output file: " << EC.message() << "\n";
            std::abort();
        }

        auto fileName = fs::path(SM.getFileEntryRefForID(SM.getMainFileID())->getName().str()).filename();

        // 1. Generate header
        out << llvm::format(R"(/****************************************************************************
** Lifted code from reading C file '%s'
**
** Created by: Lorelei CFI compiler
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

//
// CFI declarations begin
//
static void (*LoreLib_ECB)(void *, void *, void *[], void *); // guest entry
static __thread void *LoreLib_Callback;
#define LORELIB_CFI_BASE(FP, CFI_FP)                                            \
    ({                                                                          \
        __auto_type _lorelib_cfi_ret = (FP);                                    \
        if ((unsigned long) _lorelib_cfi_ret < (unsigned long) LoreLib_ECB) {   \
            LoreLib_Callback = _lorelib_cfi_ret;                                \
            _lorelib_cfi_ret = (__typeof__(_lorelib_cfi_ret)) CFI_FP;           \
        }                                                                       \
        _lorelib_cfi_ret;                                                       \
    })
)",
                            fileName.string().c_str());
        out << "\n";

        // 2. Generate forward declarations
        for (const auto &pair : std::as_const(CFIInfoMap)) {
            auto &info = pair.second;
            out << "// decl: " << info.Signature << "\n";
            out << llvm::format(R"(static const void *const LoreLib_CFI_%s_ptr;
#define LORELIB_CFI_%s(FP) \
    LORELIB_CFI_BASE(FP, LoreLib_CFI_%s_ptr)
)",
                                info.Hash.c_str(), info.Hash.c_str(), info.Hash.c_str());
            out << "\n";
        }

        out << R"(//
// CFI declarations end
//
)";
        out << "\n\n";

        // 3. Generate rewrited code
        std::sort(InsertionList.begin(), InsertionList.end(), [](const Insertion &a, const Insertion &b) {
            return a.Location > b.Location; //
        });
        for (const auto &ins : std::as_const(InsertionList)) {
            Rewrite.InsertText(ins.Location, ins.Text, ins.InsertAfter);
        }

        out << R"(//
// Original code begin
//
)";
        Rewrite.getEditBuffer(SM.getMainFileID()).write(out);
        out << "\n";
        out << R"(//
// Original code end
//
)";
        out << "\n\n";

        // 4. Generate CFI implementations
        out << R"(//
// CFI implementation begin
//
)";
        out << "\n";

        out << R"(#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

extern bool Lore_RevealLibraryPath(char *buffer, const void *addr);
extern void *Lore_GetFPExecuteCallback();
extern void *Lore_GetCallbackThunk(const char *sign);
)";
        out << "\n";

        for (const auto &pair : std::as_const(CFIInfoMap)) {
            auto &info = pair.second;
            out << "// thunk: " << info.Signature << "\n";
            out << llvm::format("static void *LoreLib_GTK_%s;\n", info.Hash.c_str());
        }
        out << "\n";

        out << R"(static inline void LoreLib_GetThunkHelper(
    void **thunk, const char *sign, const char *path) {
    if (!(*thunk = Lore_GetCallbackThunk(sign))) {
        fprintf(stderr, "%s: guest thunk lookup error: %s\n", path, sign);
        abort();
    }
}
static void __attribute__((constructor)) LoreLib_Init() {
    char path[PATH_MAX];
    if (!Lore_RevealLibraryPath(path, LoreLib_Init)) {
        fprintf(stderr, "Unknown host library: failed to get library path\n");
        abort();
    }
    LoreLib_ECB = (__typeof__(LoreLib_ECB)) Lore_GetFPExecuteCallback();
)";
        out << "\n";

        for (const auto &pair : std::as_const(CFIInfoMap)) {
            auto &info = pair.second;
            out << llvm::format("    LoreLib_GetThunkHelper(&LoreLib_GTK_%s, \"%s\", path);\n", info.Hash.c_str(),
                                info.Signature.c_str());
        }

        out << "}\n\n";

        for (const auto &pair : std::as_const(CFIInfoMap)) {
            auto &info = pair.second;
            auto &type = info.Type;

            QualType retType;
            SmallVector<QualType, 4> argTypes;
            if (type->isFunctionNoProtoType()) {
                auto funcType = type->getAs<clang::FunctionNoProtoType>();
                retType = funcType->getReturnType();
            } else {
                auto funcType = type->getAs<clang::FunctionProtoType>();
                retType = funcType->getReturnType();
                argTypes = {funcType->param_type_begin(), funcType->param_type_end()};
            }

            out << "// impl: " << info.Signature << "\n";
            out << llvm::format("__typeof__(%s) LoreLib_CFI_%s(", retType.getAsString().c_str(), info.Hash.c_str());
            for (int i = 0; i < argTypes.size(); ++i) {
                out << llvm::format("__typeof__(%s) arg%d", argTypes[i].getAsString().c_str(), i + 1);
                if (i != argTypes.size() - 1) {
                    out << ", ";
                }
            }
            out << ") {\n";
            if (!retType->isVoidType()) {
                out << llvm::format("    __typeof__(%s) ret;\n", retType.getAsString().c_str());
            }
            if (!argTypes.empty()) {
                out << "    void *args[] = {\n        ";
                for (int i = 0; i < argTypes.size(); ++i) {
                    out << "&arg" << i + 1 << ", ";
                }
                out << "\n    };\n";
            } else {
                out << "    void *args[] = {};\n";
            }
            if (!retType->isVoidType()) {
                out << llvm::format("    LoreLib_ECB(LoreLib_GTK_%s, LoreLib_Callback, args, &ret);\n",
                                    info.Hash.c_str());
                out << "    return ret;\n";
            } else {
                out << llvm::format("    LoreLib_ECB(LoreLib_GTK_%s, LoreLib_Callback, args, NULL);\n",
                                    info.Hash.c_str());
            }
            out << "}\n";
            out << llvm::format("static const void *const LoreLib_CFI_%s_ptr = LoreLib_CFI_%s;\n\n", info.Hash.c_str(),
                                info.Hash.c_str());
        }

        out << R"(//
// CFI implementation end
//)";
    }

    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef InFile) override {
        this->CI = &CI;
        Rewrite.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
        return std::make_unique<FPExprConsumer>(Expressions);
    }

private:
    std::vector<const CallExpr *> Expressions;
    Rewriter Rewrite;
    CompilerInstance *CI = nullptr;
};

static cl::OptionCategory FPExprCategory("cfi compiler");

static cl::opt<std::string> ConfigOption("c", cl::desc("Specify configuration file"), cl::desc("<config>"),
                                         cl::cat(FPExprCategory));

static cl::opt<std::string> OutputOption("o", cl::desc("Specify output file"), cl::value_desc("output file"),
                                         cl::cat(FPExprCategory));

static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

int main(int argc, const char *argv[]) {
    auto ExpectedParser = CommonOptionsParser::create(argc, argv, FPExprCategory);
    if (!ExpectedParser) {
        llvm::errs() << ExpectedParser.takeError();
        return 0;
    }

    // Parse configure
    if (auto ConfigPath = ConfigOption.getValue(); !ConfigPath.empty()) {
        GlobalContext.Config = FPExprConfig::parse(ConfigPath);
    }
    GlobalContext.OutputPath = OutputOption.getValue();

    auto &OptionsParser = ExpectedParser.get();
    ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());
    return Tool.run(newFrontendActionFactory<FPExprAction>().get());
}