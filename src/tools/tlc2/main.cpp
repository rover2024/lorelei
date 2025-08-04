#include <filesystem>
#include <cstdlib>

#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>

#include <llvm/Support/CommandLine.h>

#include "configfile.h"
#include "analyzer.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;

namespace cl = llvm::cl;
namespace fs = std::filesystem;

namespace TLC {

    class GlobalContext {
    public:
        ConfigFile Config;

        std::filesystem::path WorkingDirectory;

        std::string InputFilePath;
        std::string OutputHeaderPath;
        std::string OutputSourcePath;
        std::string OutputCallbacksPath;
    };

}

static TLC::GlobalContext GlobalContext;

/// \class DeclHandler
/// \brief Match callback for function and variable declarations
class DeclHandler : public MatchFinder::MatchCallback {
public:
    DeclHandler(SmallVectorImpl<const FunctionDecl *> &FDs, SmallVectorImpl<const VarDecl *> &VDs)
        : FDs(FDs), VDs(VDs) {
    }

    virtual void run(const MatchFinder::MatchResult &Result) override {
        const FunctionDecl *func = Result.Nodes.getNodeAs<FunctionDecl>("functionDecl");
        if (func) {
            FDs.push_back(func);
            return;
        }

        const VarDecl *var = Result.Nodes.getNodeAs<VarDecl>("varDecl");
        if (var) {
            if (var->hasGlobalStorage() && var->getLinkageInternal() == Linkage::External &&
                var->getStorageClass() != SC_Static) {
                VDs.push_back(var);
            }
            return;
        }
    }

private:
    SmallVectorImpl<const FunctionDecl *> &FDs;
    SmallVectorImpl<const VarDecl *> &VDs;
};

/// \class DeclConsumer
/// \brief AST consumer for function and variable declarations.
class DeclConsumer : public ASTConsumer {
public:
    DeclConsumer(SmallVectorImpl<const FunctionDecl *> &FDs, SmallVectorImpl<const VarDecl *> &VDs)
        : Handler(FDs, VDs) {
        Matcher.addMatcher(functionDecl().bind("functionDecl"), &Handler);
        Matcher.addMatcher(varDecl().bind("varDecl"), &Handler);
    }

    void HandleTranslationUnit(ASTContext &Context) override {
        Matcher.matchAST(Context); //
    }

private:
    MatchFinder Matcher;
    DeclHandler Handler;
};

/// \class TLCAction
/// \brief Frontend action for generating thunk library code.
class TLCAction : public ASTFrontendAction {
public:
    TLCAction() {
    }

    void EndSourceFileAction() override {
        fs::current_path(GlobalContext.WorkingDirectory);

        // Fix file paths
        auto InputFilePath = GlobalContext.InputFilePath.empty() ? "lorelib_manifest.c"
                                                                 : GlobalContext.InputFilePath;
        auto OutHeaderPath = GlobalContext.OutputHeaderPath.empty()
                                 ? "lorelib_defs.h"
                                 : GlobalContext.OutputHeaderPath;
        auto OutSourcePath = GlobalContext.OutputSourcePath.empty()
                                 ? "lorelib_impl.c"
                                 : GlobalContext.OutputSourcePath;
        auto OutCallbacksPath = GlobalContext.OutputCallbacksPath.empty()
                                    ? "lorelib_callbacks.txt"
                                    : GlobalContext.OutputCallbacksPath;

        TLC::Analyzer Analyzer(getCompilerInstance(), FDs, VDs, GlobalContext.Config);
        Analyzer.analyze();

        // Open output files
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

        // Generate codes
        Analyzer.generateHeader(OutHeader);
        Analyzer.generateSource(OutSource);
        for (auto &pair : Analyzer.thunks(TLC::ThunkDefinition::GuestCallbackThunk)) {
            auto &Thunk = pair.second;
            OutCallbacks << Thunk.qualType().getCanonicalType().getAsString() << "\n";
        }
    }

    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                   StringRef InFile) override {
        return std::make_unique<DeclConsumer>(FDs, VDs);
    }

private:
    SmallVector<const FunctionDecl *, 100> FDs;
    SmallVector<const VarDecl *, 100> VDs;
};

static cl::OptionCategory FuncDeclCategory("Lorelei thunk library compiler");

static cl::opt<std::string> ConfigOption("c", cl::desc("Specify configuration file"),
                                         cl::value_desc("config"), cl::cat(FuncDeclCategory));
static cl::opt<std::string> OutputHeaderOption("output-header",
                                               cl::desc("Specify output header file"),
                                               cl::value_desc("path"), cl::cat(FuncDeclCategory));
static cl::opt<std::string> OutputSourceOption("output-source",
                                               cl::desc("Specify output source file"),
                                               cl::value_desc("path"), cl::cat(FuncDeclCategory));
static cl::opt<std::string> OutputCallbacksOption("output-callbacks",
                                                  cl::desc("Specify output callbacks file"),
                                                  cl::value_desc("path"),
                                                  cl::cat(FuncDeclCategory));

static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

int main(int argc, const char *argv[]) {
    auto ExpectedParser = CommonOptionsParser::create(argc, argv, FuncDeclCategory);
    if (!ExpectedParser) {
        llvm::errs() << ExpectedParser.takeError();
        return 0;
    }

    GlobalContext.WorkingDirectory = fs::current_path();

    if (auto ConfigPath = ConfigOption.getValue(); !ConfigPath.empty()) {
        std::ignore = GlobalContext.Config.load(ConfigPath);
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