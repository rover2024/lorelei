#include <filesystem>
#include <map>
#include <cstdlib>
#include <sstream>
#include <utility>

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

// 1. Match handler
class FuncDeclHandler : public MatchFinder::MatchCallback {
public:
    FuncDeclHandler(SmallVectorImpl<const FunctionDecl *> &FunctionDecls) : FunctionDecls(FunctionDecls) {
    }

    virtual void run(const MatchFinder::MatchResult &Result) override {
        const FunctionDecl *func = Result.Nodes.getNodeAs<FunctionDecl>("functionDecl");
        if (!func)
            return;
        FunctionDecls.push_back(func);
    }

private:
    SmallVectorImpl<const FunctionDecl *> &FunctionDecls;
};

// 2. AST consumer
class FuncDeclConsumer : public ASTConsumer {
public:
    FuncDeclConsumer(SmallVectorImpl<const FunctionDecl *> &FunctionDecls)
        : FunctionDecls(FunctionDecls), Handler(FunctionDecls) {
        Matcher.addMatcher(functionDecl().bind("functionDecl"), &Handler);
    }

    void HandleTranslationUnit(ASTContext &Context) override {
        Matcher.matchAST(Context); //
    }

private:
    SmallVectorImpl<const FunctionDecl *> &FunctionDecls;
    MatchFinder Matcher;
    FuncDeclHandler Handler;
};

// 3. AST frontend action
class TestAction : public ASTFrontendAction {
public:
    TestAction() {
    }

    void EndSourceFileAction() override {
        for (const auto &FD : std::as_const(FunctionDecls)) {
            auto FPT = FD->getType()->getAs<clang::FunctionProtoType>();
            if (FPT->getNumParams() > 1) {
                auto MaybeVaListParam = FPT->getParamType(FPT->getNumParams() - 1);
                if (StringRef(MaybeVaListParam.getAsString()).starts_with("struct __va_list_tag")) {
                    auto MaybeFmtParam = FPT->getParamType(FPT->getNumParams() - 2);
                    if (MaybeFmtParam->isPointerType()) {
                        auto MaybeFmtParamPointee = MaybeFmtParam->getPointeeType();
                        if (MaybeFmtParamPointee->isCharType()) {
                            llvm::outs() << FD->getNameAsString()
                                         << " has variadic arguments and a format string parameter.\n";
                        }
                    }
                }
            }
        }
    }

    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef InFile) override {
        this->CI = &CI;
        Rewrite.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
        return std::make_unique<FuncDeclConsumer>(FunctionDecls);
    }

private:
    SmallVector<const FunctionDecl *, 20> FunctionDecls;
    Rewriter Rewrite;
    CompilerInstance *CI = nullptr;
};

static cl::OptionCategory FPExprCategory("test");

static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

int main(int argc, const char *argv[]) {
    auto ExpectedParser = CommonOptionsParser::create(argc, argv, FPExprCategory);
    if (!ExpectedParser) {
        llvm::errs() << ExpectedParser.takeError();
        return 0;
    }

    auto &OptionsParser = ExpectedParser.get();
    ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());
    return Tool.run(newFrontendActionFactory<TestAction>().get());
}