#include <filesystem>
#include <map>
#include <cstdlib>
#include <sstream>
#include <set>

#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Rewrite/Core/Rewriter.h>

#include <llvm/Support/CommandLine.h>
#include <llvm/Support/SHA256.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/Error.h>

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;

namespace cl = llvm::cl;
namespace fs = std::filesystem;

class InputFile {
public:
    InputFile() = default;
    ~InputFile() = default;

public:
    llvm::Error load(const std::filesystem::path &path) {
        auto buffer = llvm::MemoryBuffer::getFile(path.string());
        if (std::error_code ec = buffer.getError()) {
            return llvm::createStringError(ec, "Failed to open file: %s", ec.message().c_str());
        }

        decltype(_symbols) symbols;
        {
            std::istringstream iss(buffer->get()->getBuffer().str());
            std::string line;
            int i = 0;
            while (std::getline(iss, line)) {
                line = StringRef(line).trim().str();
                if (line.empty()) {
                    continue;
                }
                symbols.insert(line);
            }
        }
        _symbols = std::move(symbols);
        return llvm::Error::success();
    }

    const std::set<std::string> &symbols() const {
        return _symbols;
    }

protected:
    std::set<std::string> _symbols;
};

class MyGlobalContext {
public:
    std::filesystem::path WorkingDirectory;

    class InputFile InputFile;
    std::string OutputPath;
};

static MyGlobalContext GlobalContext;

// 1. Match handler
class MyMatchHandler : public MatchFinder::MatchCallback {
public:
    MyMatchHandler(SmallVectorImpl<const CallExpr *> &Expressions,
                   SmallVectorImpl<const FunctionDecl *> &FDs)
        : Expressions(Expressions), FDs(FDs) {
    }

    virtual void run(const MatchFinder::MatchResult &Result) override {
        const CallExpr *call = Result.Nodes.getNodeAs<CallExpr>("call");
        if (call) {
            const Expr *calleeExpr = call->getCallee()->IgnoreImpCasts();
            if (!calleeExpr)
                return;

            QualType type = calleeExpr->getType().getCanonicalType();
            if (!type->isFunctionPointerType() && !type->isBlockPointerType() &&
                !type->isMemberFunctionPointerType())
                return;

            Expressions.push_back(call);
            return;
        }


        const FunctionDecl *func = Result.Nodes.getNodeAs<FunctionDecl>("functionDecl");
        if (func) {
            FDs.push_back(func);
            return;
        }
    }

private:
    SmallVectorImpl<const CallExpr *> &Expressions;
    SmallVectorImpl<const FunctionDecl *> &FDs;
};

// 2. AST consumer
class MyASTConsumer : public ASTConsumer {
public:
    MyASTConsumer(SmallVectorImpl<const CallExpr *> &Expressions,
                  SmallVectorImpl<const FunctionDecl *> &FDs)
        : Handler(Expressions, FDs) {
        Matcher.addMatcher(functionDecl().bind("functionDecl"), &Handler);
        Matcher.addMatcher(callExpr().bind("call"), &Handler);
    }

    void HandleTranslationUnit(ASTContext &Context) override {
        Matcher.matchAST(Context); //
    }

private:
    MatchFinder Matcher;
    MyMatchHandler Handler;
};

// 3. AST frontend action
class MyFrontendAction : public ASTFrontendAction {
public:
    MyFrontendAction() {
    }

    void EndSourceFileAction() override {
        fs::current_path(GlobalContext.WorkingDirectory);

        SourceManager &SM = Rewrite.getSourceMgr();
        LangOptions LangOpts = Rewrite.getLangOpts();
        ASTContext &Context = getCompilerInstance().getASTContext();

        // for (const CallExpr *call : std::as_const(Expressions)) {
        //     auto Loc = call->getBeginLoc();

        //     // Ignore expressions in header files
        //     if (!SM.isInMainFile(Loc)) {
        //         continue;
        //     }

        //     auto calleeExpr = call->getCallee()->IgnoreImpCasts();
        //     auto type = calleeExpr->getType().getCanonicalType()->getPointeeType();
        //     llvm::outs() << Loc.printToString(SM) << ": " << type.getAsString() << "\n";
        // }

        for (const FunctionDecl *FD : std::as_const(FDs)) {
            auto Name = FD->getNameAsString();
            if (!GlobalContext.InputFile.symbols().count(Name)) {
                continue;
            }
            FD->print(llvm::outs());
            llvm::outs() << ";\n";
        }
    }

    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                   StringRef InFile) override {
        Rewrite.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
        return std::make_unique<MyASTConsumer>(Expressions, FDs);
    }

private:
    SmallVector<const CallExpr *, 20> Expressions;
    SmallVector<const FunctionDecl *, 20> FDs;
    Rewriter Rewrite;
};


static cl::OptionCategory MyCategory("my tooling");

static cl::opt<std::string> InputOption("i", cl::desc("Specify input file"),
                                        cl::value_desc("input file"), cl::cat(MyCategory));

static cl::opt<std::string> OutputOption("o", cl::desc("Specify output file"),
                                         cl::value_desc("output file"), cl::cat(MyCategory));

static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

int main(int argc, const char *argv[]) {
    auto ExpectedParser = CommonOptionsParser::create(argc, argv, MyCategory);
    if (!ExpectedParser) {
        llvm::errs() << ExpectedParser.takeError();
        return 0;
    }

    GlobalContext.WorkingDirectory = fs::current_path();

    if (auto InputPath = InputOption.getValue(); !InputPath.empty()) {
        std::ignore = GlobalContext.InputFile.load(InputPath);
    }
    GlobalContext.OutputPath = OutputOption.getValue();

    auto &OptionsParser = ExpectedParser.get();
    ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());
    return Tool.run(newFrontendActionFactory<MyFrontendAction>().get());
}
