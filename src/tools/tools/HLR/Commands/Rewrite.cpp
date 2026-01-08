#include <string>
#include <filesystem>

#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <llvm/Support/MemoryBuffer.h>

#include <LoreTools/Basic/RewriteInsertion.h>
#include <LoreTools/HLRUtils/ASTConsumers.h>

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;

namespace cl = llvm::cl;
namespace fs = std::filesystem;

namespace lore::tool::command::rewrite {

    struct GlobalContext {
        /// Global states
        std::filesystem::path initialCwd;
        std::string outputPath;

        /// Runtime data
        std::string outBuffer;
    };

    static GlobalContext &g_ctx() {
        static GlobalContext instance;
        return instance;
    }

    class MyASTFrontendAction : public ASTFrontendAction {
    public:
        static inline bool isInMacro(SourceManager &SM, SourceLocation loc) {
            return SM.isMacroBodyExpansion(loc) || SM.isMacroArgExpansion(loc);
        }

        void EndSourceFileAction() override {
            SourceManager &SM = _rewriter.getSourceMgr();
            LangOptions LO = _rewriter.getLangOpts();
            ASTContext &AST = getCompilerInstance().getASTContext();

            const auto &getEndLoc = [&](SourceLocation loc) {
                return Lexer::getLocForEndOfToken(loc, 0, SM, LO);
            };

            const auto &fixRangeEnd = [&](CharSourceRange &range) {
                range.setEnd(getEndLoc(range.getEnd()));
                range.setTokenRange(true);
                return range;
            };

            const auto &getExprFullExpansionRange = [&](const Expr *E) {
                auto range = E->getSourceRange();
                auto beginLoc = range.getBegin();
                auto endLoc = range.getEnd();
                bool isBeginInMacro = isInMacro(SM, beginLoc);
                bool isEndInMacro = isInMacro(SM, endLoc);

                CharSourceRange c;
                if (isBeginInMacro) {
                    auto macroBeginExpansionRange = SM.getExpansionRange(beginLoc);
                    c.setBegin(macroBeginExpansionRange.getBegin());
                } else {
                    c.setBegin(beginLoc);
                }

                if (isEndInMacro) {
                    auto macroEndExpansionRange = SM.getExpansionRange(endLoc);
                    c.setEnd(macroEndExpansionRange.getEnd());
                } else {
                    c.setEnd(endLoc);
                }
                fixRangeEnd(c);

                return c;
            };

            RewriteRangePackerSet insertions;
            for (const CallExpr *CE : std::as_const(_fileData.callbackInvokeExprList)) {
            }

            for (const DeclRefExpr *DRE : std::as_const(_fileData.functionDecayExprList)) {
            }

            if (insertions.ranges().empty()) {
                return;
            }

            for (const auto &ins : std::as_const(insertions.ranges())) {
                _rewriter.InsertText(ins.range.getBegin(), ins.textBefore, false);
                _rewriter.InsertText(ins.range.getEnd(), ins.textAfter, true);
            }

            // Generate rewritten code
            llvm::raw_string_ostream out(g_ctx().outBuffer);
            _rewriter.getEditBuffer(SM.getMainFileID()).write(out);
        }

        std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                       StringRef InFile) override {
            _rewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
            return std::make_unique<HLR::FunctionExprConsumer>(_fileData.callbackInvokeExprList,
                                                               _fileData.functionDecayExprList);
        }

    private:
        struct ConsumedFileData {
            llvm::SmallVector<const CallExpr *, 20> callbackInvokeExprList;
            llvm::SmallVector<const DeclRefExpr *, 20> functionDecayExprList;
        };
        ConsumedFileData _fileData;
        Rewriter _rewriter;
    };

    const char *name = "rewrite";

    const char *help = "Rewrite preprocessed files";

    int main(int argc, char *argv[]) {
        static cl::OptionCategory myOptionCat(
            "Lorelei Host Library Rewriter - Rewrite (Single File)");
        static cl::opt<std::string> outputOption("o", cl::desc("Specify output file"),
                                                 cl::value_desc("output file"),
                                                 cl::cat(myOptionCat));
        static cl::extrahelp commonHelp(CommonOptionsParser::HelpMessage);

        auto expectedParser = CommonOptionsParser::create(
            argc, const_cast<const char **>(argv), myOptionCat, cl::NumOccurrencesFlag::Required);
        if (!expectedParser) {
            llvm::errs() << expectedParser.takeError();
            return 0;
        }
        auto &parser = expectedParser.get();

        g_ctx().initialCwd = fs::current_path();
        g_ctx().outputPath = outputOption.getValue();

        ClangTool tool(parser.getCompilations(), parser.getSourcePathList());
        if (int ret = tool.run(newFrontendActionFactory<MyASTFrontendAction>().get()); ret != 0) {
            return ret;
        }

        std::error_code ec;
        llvm::raw_fd_ostream out(g_ctx().outputPath.empty() ? "-" : g_ctx().outputPath, ec);
        if (ec) {
            llvm::errs() << "Error occurs opening output file: " << ec.message() << "\n";
            return 1;
        }
        out << g_ctx().outBuffer;
        return 0;
    }

}
