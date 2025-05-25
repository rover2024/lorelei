#include <filesystem>
#include <map>
#include <cstdlib>
#include <sstream>

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

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;

namespace cl = llvm::cl;
namespace fs = std::filesystem;

class CFICConfig {
public:
    std::map<std::string, int> Signatures;

    static CFICConfig parse(const std::string &path) {
        auto buffer = llvm::MemoryBuffer::getFile(path);
        if (std::error_code EC = buffer.getError()) {
            llvm::errs() << path << ": failed to open file: " << EC.message() << "\n";
            std::abort();
        }

        CFICConfig config;
        {
            std::istringstream iss(buffer->get()->getBuffer().str());
            std::string line;
            int i = 0;
            while (std::getline(iss, line)) {
                if (line.empty()) {
                    continue;
                }
                config.Signatures[line] = ++i;
            }
        }
        return config;
    }
};

class CFICCGlobalContext {
public:
    CFICConfig Config;

    std::filesystem::path WorkingDirectory;

    std::string Identifier = "unknown";
    std::string OutputPath;
};

static CFICCGlobalContext GlobalContext;

// 1. Match handler
class FPExprHandler : public MatchFinder::MatchCallback {
public:
    FPExprHandler(SmallVectorImpl<const CallExpr *> &Expressions) : Expressions(Expressions) {
    }

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
    SmallVectorImpl<const CallExpr *> &Expressions;
};

// 2. AST consumer
class FPExprConsumer : public ASTConsumer {
public:
    FPExprConsumer(SmallVectorImpl<const CallExpr *> &Expressions) : Expressions(Expressions), Handler(Expressions) {
        Matcher.addMatcher(callExpr().bind("call"), &Handler);
    }

    void HandleTranslationUnit(ASTContext &Context) override {
        Matcher.matchAST(Context); //
    }

private:
    SmallVectorImpl<const CallExpr *> &Expressions;
    MatchFinder Matcher;
    FPExprHandler Handler;
};

// 3. AST frontend action
class CFICAction : public ASTFrontendAction {
public:
    CFICAction() {
    }

    void EndSourceFileAction() override {
        fs::current_path(GlobalContext.WorkingDirectory);

        SourceManager &SM = Rewrite.getSourceMgr();
        LangOptions LangOpts = Rewrite.getLangOpts();
        ASTContext &Context = getCompilerInstance().getASTContext();

        class CFIInfo {
        public:
            QualType Type;
            int Index;
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
        auto &SignatureMap = GlobalContext.Config.Signatures;
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
            int hashIndex = 0;
            if (!SignatureMap.empty()) {
                auto it = SignatureMap.find(signature);
                if (it == SignatureMap.end()) {
                    continue;
                }
                hashIndex = it->second;
            }

            if (auto it = CFIInfoMap.find(signature); it == CFIInfoMap.end()) {
                if (hashIndex == 0) {
                    hashIndex = ++hashCnt;
                } else {
                    hashCnt = hashIndex;
                }
                CFIInfoMap.insert(std::make_pair(signature, CFIInfo{type, hashIndex, signature}));
            } else {
                hashIndex = it->second.Index;
            }
            InsertionList.push_back({fixedRange.getBegin(), "LORELIB_CFI_" + std::to_string(hashIndex) + "(", false});
            InsertionList.push_back({fixedRange.getEnd(), ")", true});
        }

        if (InsertionList.empty()) {
            return;
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
** CFI wrapped code from reading C file '%s'
**
** Created by: Lorelei CFI compiler
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

//
// CFI declarations begin
//
enum LoreLib_Constants {
    LoreLib_CFI_Count = %d,
};

struct LoreLib_HostLibraryContext {
    void *AddressBoundary;

    void (*HrtSetThreadCallback)(void *callback);
    void *HrtPThreadCreate;
    void *HrtPThreadExit;

    void *CFIs[LoreLib_CFI_Count];
};

__attribute__((visibility("default"))) struct LoreLib_HostLibraryContext LoreLib_HostLibCtx;

#define LORELIB_CFI(INDEX, FP)                                                                       \
    ({                                                                                               \
        typedef __typeof__(FP) _LORELIB_CFI_TYPE;                                                    \
        void *_lorelib_cfi_ret = (void *) (FP);                                                      \
        if ((unsigned long) _lorelib_cfi_ret < (unsigned long) LoreLib_HostLibCtx.AddressBoundary) { \
            LoreLib_HostLibCtx.HrtSetThreadCallback(_lorelib_cfi_ret);                               \
            _lorelib_cfi_ret = (void *) LoreLib_HostLibCtx.CFIs[INDEX - 1];                          \
        }                                                                                            \
        (_LORELIB_CFI_TYPE) _lorelib_cfi_ret;                                                        \
    })
)",
                            fileName.string().c_str(), int(GlobalContext.Config.Signatures.size()), GlobalContext.Identifier.c_str());
        out << "\n";

        // 2. Generate forward declarations
        for (const auto &pair : std::as_const(CFIInfoMap)) {
            auto &info = pair.second;
            out << "// decl: " << info.Signature << "\n";
            out << llvm::format("#define LORELIB_CFI_%d(FP) LORELIB_CFI(%d, FP)\n", info.Index, info.Index);
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
    }

    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef InFile) override {
        Rewrite.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
        return std::make_unique<FPExprConsumer>(Expressions);
    }

private:
    SmallVector<const CallExpr *, 20> Expressions;
    Rewriter Rewrite;
};

static cl::OptionCategory FPExprCategory("cfi compiler");

static cl::opt<bool> ExpandOption("e", cl::desc("Expand macros"), cl::cat(FPExprCategory));

static cl::opt<std::string> IdentifierOption("i", cl::desc("Specify identifier"), cl::value_desc("id"),
                                             cl::cat(FPExprCategory));

static cl::opt<std::string> ConfigOption("c", cl::desc("Specify configuration file"), cl::value_desc("config"),
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

    GlobalContext.WorkingDirectory = fs::current_path();

    if (auto Identifier = IdentifierOption.getValue(); !Identifier.empty()) {
        GlobalContext.Identifier = IdentifierOption.getValue();
    }
    if (auto ConfigPath = ConfigOption.getValue(); !ConfigPath.empty()) {
        GlobalContext.Config = CFICConfig::parse(ConfigPath);
    }
    GlobalContext.OutputPath = OutputOption.getValue();

    auto &OptionsParser = ExpectedParser.get();
    ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());
    return Tool.run(newFrontendActionFactory<CFICAction>().get());
}