// void *QEMU_ADDRESS;

// struct BB {
//     int (*fp)(int);
// };

// struct AA {
//     struct BB *bb;
// };

// static __thread __typeof__(int (*)(int)) BB_fp_thunk_callback;
// static int BB_fp_thunk(int a) {
//     return BB_fp_thunk_callback(a);
// }

// void func(AA *aa) {
//     if (aa->bb) {
//         __auto_type TMP = &aa->bb->fp;
//         if (__builtin_expect((unsigned long) *TMP < (unsigned long) QEMU_ADDRESS, 1)) {
//             BB_fp_thunk_callback = *TMP;
//             *TMP = BB_fp_thunk;
//         }
//     }
// }

// int main(int argc, char *argv[]) {
//     printf("hello world\n");
// }

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

// 1. Match handler
class FPExprHandler : public MatchFinder::MatchCallback {
public:
    FPExprHandler(std::vector<const FunctionDecl *> &FunctionDecls) : FunctionDecls(FunctionDecls) {}

    virtual void run(const MatchFinder::MatchResult &Result) override {
        const FunctionDecl *func = Result.Nodes.getNodeAs<FunctionDecl>("functionDecl");
        if (!func)
            return;

        FunctionDecls.push_back(func);
    }

private:
    std::vector<const FunctionDecl *> &FunctionDecls;
};

// 2. AST consumer
class FPExprConsumer : public ASTConsumer {
public:
    FPExprConsumer(std::vector<const FunctionDecl *> &FunctionDecls)
        : FunctionDecls(FunctionDecls), Handler(FunctionDecls) {
        Matcher.addMatcher(callExpr().bind("functionDecl"), &Handler);
    }

    void HandleTranslationUnit(ASTContext &Context) override {
        Matcher.matchAST(Context); //
    }

private:
    std::vector<const FunctionDecl *> &FunctionDecls;
    MatchFinder Matcher;
    FPExprHandler Handler;
};

// 3. AST frontend action
class FPExprAction : public ASTFrontendAction {
public:
    FPExprAction() {}

    void EndSourceFileAction() override {}

    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef InFile) override {
        this->CI = &CI;
        Rewrite.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
        return std::make_unique<FPExprConsumer>(FunctionDecls);
    }

private:
    std::vector<const FunctionDecl *> FunctionDecls;
    Rewriter Rewrite;
    CompilerInstance *CI = nullptr;
};

static cl::OptionCategory FPExprCategory("thunk library compiler");

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