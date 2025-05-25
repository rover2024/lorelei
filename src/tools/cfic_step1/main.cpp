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

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;

namespace cl = llvm::cl;
namespace fs = std::filesystem;

class CFICCGlobalContext {
public:
    std::filesystem::path WorkingDirectory;

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

        class Insertion {
        public:
            SourceLocation Location;
            std::string Text;
            bool InsertAfter = false;
        };
        std::vector<Insertion> InsertionList;

        struct LineAndColumn {
            unsigned Line;
            unsigned Column;

            bool operator<(const LineAndColumn &RHS) const {
                if (Line != RHS.Line) {
                    return Line < RHS.Line;
                }
                return Column < RHS.Column;
            }
        };

        std::set<LineAndColumn> InsertedLocations;
        for (const CallExpr *call : std::as_const(Expressions)) {
            auto Loc = call->getBeginLoc();

            // Ignore expressions in header files
            if (!SM.isInMainFile(Loc)) {
                continue;
            }
            if (!SM.isMacroBodyExpansion(Loc) && !SM.isMacroArgExpansion(Loc)) {
                continue;
            }

            auto expendRange = SM.getExpansionRange(Loc);
            {
                FullSourceLoc RealLoc(expendRange.getBegin(), SM);
                LineAndColumn LC = {SM.getSpellingLineNumber(RealLoc), SM.getSpellingColumnNumber(RealLoc)};
                if (InsertedLocations.count(LC)) {
                    continue;
                }
                InsertedLocations.insert(LC);

                // llvm::outs() << LC.Line << ":" << LC.Column << "\n";
            }

            // Fix range
            auto fixedRange = expendRange;
            fixedRange.setEnd(Lexer::getLocForEndOfToken(fixedRange.getEnd(), 0, SM, LangOpts));

            // llvm::outs() << Loc.printToString(SM) << ": " << Lexer::getSourceText(fixedRange, SM, LangOpts) << "\n";

            InsertionList.push_back({fixedRange.getBegin(), "/*__LORELEI_MACRO_BEGIN__*/"});
            InsertionList.push_back({fixedRange.getEnd(), "/*__LORELEI_MACRO_END__*/", true});
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

        // Generate rewrited code
        std::sort(InsertionList.begin(), InsertionList.end(), [](const Insertion &a, const Insertion &b) {
            return a.Location > b.Location; //
        });
        for (const auto &ins : std::as_const(InsertionList)) {
            Rewrite.InsertText(ins.Location, ins.Text, ins.InsertAfter);
        }
        Rewrite.getEditBuffer(SM.getMainFileID()).write(out);
    }

    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef InFile) override {
        Rewrite.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
        return std::make_unique<FPExprConsumer>(Expressions);
    }

private:
    SmallVector<const CallExpr *, 20> Expressions;
    Rewriter Rewrite;
};


static cl::OptionCategory FPExprCategory("cfi compiler step1");

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

    GlobalContext.OutputPath = OutputOption.getValue();

    auto &OptionsParser = ExpectedParser.get();
    ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());
    return Tool.run(newFrontendActionFactory<CFICAction>().get());
}
