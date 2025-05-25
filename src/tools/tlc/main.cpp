#include <filesystem>
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

#include "core/passcontext.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;

namespace cl = llvm::cl;
namespace fs = std::filesystem;

class TLCConfig {
public:
    std::set<std::string> Functions;

    static TLCConfig parse(const std::string &path) {
        auto buffer = llvm::MemoryBuffer::getFile(path);
        if (std::error_code EC = buffer.getError()) {
            llvm::errs() << path << ": failed to open file: " << EC.message() << "\n";
            std::abort();
        }

        TLCConfig config;
        {
            std::istringstream iss(buffer->get()->getBuffer().str());
            std::string line;
            int i = 0;
            while (std::getline(iss, line)) {
                line = llvm::StringRef(line).trim().str();
                if (line.empty()) {
                    continue;
                }
                config.Functions.insert(line);
            }
        }
        return config;
    }
};

class TLCCGlobalContext {
public:
    TLCConfig Config;

    std::filesystem::path WorkingDirectory;

    std::string InputFilePath;
    std::string OutputHeaderPath;
    std::string OutputSourcePath;
    std::string OutputCallbacksPath;
};

static TLCCGlobalContext GlobalContext;

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
class TLCAction : public ASTFrontendAction {
public:
    TLCAction() {
    }

    void EndSourceFileAction() override {
        fs::current_path(GlobalContext.WorkingDirectory);

        auto InputFilePath = GlobalContext.InputFilePath.empty() ? "lorelib_manifest.c" : GlobalContext.InputFilePath;
        auto OutHeaderPath = GlobalContext.OutputHeaderPath.empty() ? "lorelib_defs.h" : GlobalContext.OutputHeaderPath;
        auto OutSourcePath = GlobalContext.OutputSourcePath.empty() ? "lorelib_impl.c" : GlobalContext.OutputSourcePath;
        auto OutCallbacksPath =
            GlobalContext.OutputCallbacksPath.empty() ? "lorelib_callbacks.txt" : GlobalContext.OutputCallbacksPath;

        CompilerInstance &CI = getCompilerInstance();
        SourceManager &SM = CI.getSourceManager();
        LangOptions LangOpts = CI.getLangOpts();
        ASTContext &Context = CI.getASTContext();

        TLC::PassContext PassCtx(GlobalContext.Config.Functions, FunctionDecls, &SM, LangOpts, &Context,
                                 fs::path(InputFilePath).filename().string(),
                                 fs::path(OutHeaderPath).filename().string(),
                                 fs::path(OutSourcePath).filename().string());
        auto PassResult = PassCtx.Run();

        std::error_code EC;
        llvm::raw_fd_ostream OutHeader(OutHeaderPath, EC);
        if (EC) {
            llvm::errs() << "Error occurs opening output header file: " << EC.message() << "\n";
            std::abort();
        }
        llvm::raw_fd_ostream OutSource(OutSourcePath, EC);
        if (EC) {
            llvm::errs() << "Error occurs opening output source file: " << EC.message() << "\n";
            std::abort();
        }
        llvm::raw_fd_ostream OutCallbacks(OutCallbacksPath, EC);
        if (EC) {
            llvm::errs() << "Error occurs opening output callbacks file: " << EC.message() << "\n";
            std::abort();
        }

        OutHeader << PassResult.Defs << "\n";
        OutSource << PassResult.Impl << "\n";

        for (auto Name : PassCtx.getApiNameSet(TLC::ApiSource::GuestCallback)) {
            auto &Source = *PassCtx.getApiSource(TLC::ApiSource::GuestCallback, Name);
            OutCallbacks << Source.getType().getCanonicalType().getAsString() << "\n";
        }
    }

    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef InFile) override {
        return std::make_unique<FuncDeclConsumer>(FunctionDecls);
    }

private:
    SmallVector<const FunctionDecl *, 20> FunctionDecls;
};

static cl::OptionCategory FuncDeclCategory("Thunk library compiler");

static cl::opt<std::string> ConfigOption("c", cl::desc("Specify configuration file"), cl::value_desc("config"),
                                         cl::cat(FuncDeclCategory));

static cl::opt<std::string> OutputHeaderOption("output-header", cl::desc("Specify output header file"),
                                               cl::value_desc("path"), cl::cat(FuncDeclCategory));

static cl::opt<std::string> OutputSourceOption("output-source", cl::desc("Specify output source file"),
                                               cl::value_desc("path"), cl::cat(FuncDeclCategory));

static cl::opt<std::string> OutputCallbacksOption("output-callbacks", cl::desc("Specify output callbacks file"),
                                                  cl::value_desc("path"), cl::cat(FuncDeclCategory));

static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

int main(int argc, const char *argv[]) {
    auto ExpectedParser = CommonOptionsParser::create(argc, argv, FuncDeclCategory);
    if (!ExpectedParser) {
        llvm::errs() << ExpectedParser.takeError();
        return 0;
    }

    GlobalContext.WorkingDirectory = fs::current_path();

    if (auto ConfigPath = ConfigOption.getValue(); !ConfigPath.empty()) {
        GlobalContext.Config = TLCConfig::parse(ConfigPath);
    }
    GlobalContext.OutputHeaderPath = OutputHeaderOption.getValue();
    GlobalContext.OutputSourcePath = OutputSourceOption.getValue();
    GlobalContext.OutputCallbacksPath = OutputCallbacksOption.getValue();

    auto &OptionsParser = ExpectedParser.get();
    auto &SourcePathList = OptionsParser.getSourcePathList();
    ClangTool Tool(OptionsParser.getCompilations(), SourcePathList);

    GlobalContext.InputFilePath = SourcePathList.empty() ? std::string() : SourcePathList[0];

    return Tool.run(newFrontendActionFactory<TLCAction>().get());
}