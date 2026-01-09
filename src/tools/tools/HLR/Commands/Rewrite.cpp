#include <string>

#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <llvm/Support/MemoryBuffer.h>

#include <LoreTools/Basic/RewriteInsertion.h>
#include <LoreTools/Basic/TypeUtils.h>
#include <LoreTools/HLRUtils/ASTConsumers.h>
#include <LoreTools/HLRUtils/SourceStatistics.h>

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;

namespace cl = llvm::cl;

namespace lore::tool::command::rewrite {

    struct FDGInstanceInfo {
        size_t index;
        StringRef file;
    };

    struct GlobalContext {
        /// Global states
        SmallString<128> initialCwd;
        std::string outputPath;

        /// Runtime data
        std::map<std::string, size_t> CCGIndexMap;
        std::map<std::string, size_t> FDGIndexMap;
        std::vector<std::map<std::string, FDGInstanceInfo>> FDGInstanceInfoMaps;
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

            auto inFile = getCurrentFile().str();

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
            std::map<std::pair<size_t, size_t>, std::string> fdgImplList;
            for (const CallExpr *CE : std::as_const(_fileData.callbackInvokeExprList)) {
                auto T = realCalleeType(CE, AST);
                auto signature = getTypeString(T);
                auto it = g_ctx().CCGIndexMap.find(signature);
                if (it == g_ctx().CCGIndexMap.end()) {
                    continue;
                }

                auto range = getExprFullExpansionRange(CE->getCallee());
                std::string textBefore = "LORE_CCG_" + std::to_string(it->second + 1) + "(";
                std::string textAfter = ")";
                insertions.insert({range, std::move(textBefore), std::move(textAfter)});
            }

            for (const DeclRefExpr *DRE : std::as_const(_fileData.functionDecayExprList)) {
                auto FD = DRE->getDecl()->getAsFunction();
                assert(FD);
                auto T = AST.getPointerType(FD->getType());
                auto signature = getTypeString(T);
                auto guardIter = g_ctx().FDGIndexMap.find(signature);
                if (guardIter == g_ctx().FDGIndexMap.end()) {
                    continue;
                }
                size_t guardIdx = guardIter->second;

                auto location = FD->getBeginLoc().printToString(SM);
                const auto &instanceMap = g_ctx().FDGInstanceInfoMaps[guardIdx];
                auto instanceIter = instanceMap.find(location);
                if (instanceIter == instanceMap.end()) {
                    continue;
                }
                size_t instanceIdx = instanceIter->second.index;

                if (instanceIter->second.file == inFile) {
                    fdgImplList.try_emplace(std::make_pair(guardIdx, instanceIdx),
                                            FD->getNameAsString());
                }

                auto range = getExprFullExpansionRange(DRE);
                std::string textBefore = "LORE_FDG_" + std::to_string(guardIdx + 1) + "_" +
                                         std::to_string(instanceIdx + 1) + "(";
                std::string textAfter = ")";
                insertions.insert({range, std::move(textBefore), std::move(textAfter)});
            }

            std::string fdgImplText;
            if (!fdgImplList.empty()) {
                fdgImplText += "\n";
                for (const auto &kv : fdgImplList) {
                    const auto &[index, name] = kv;
                    fdgImplText += "LORE_FDG_IMPL(" + std::to_string(index.first + 1) + ", " +
                                   std::to_string(index.second + 1) + ", " + name + ")\n";
                }
                fdgImplText += "\n";
            }

            if (insertions.ranges().empty() && fdgImplText.empty()) {
                return;
            }

            for (const auto &ins : std::as_const(insertions.ranges())) {
                _rewriter.InsertText(ins.range.getBegin(), ins.textBefore, false);
                _rewriter.InsertText(ins.range.getEnd(), ins.textAfter, true);
            }
            if (!fdgImplText.empty()) {
                _rewriter.InsertText(SM.getLocForEndOfFile(SM.getMainFileID()), fdgImplText, true);
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
        static cl::opt<std::string> statOption("s", cl::desc("Specify statistics file"),
                                               cl::value_desc("statistics file"),
                                               cl::cat(myOptionCat), cl::Required);
        static cl::opt<std::string> outputOption("o", cl::desc("Specify output file"),
                                                 cl::value_desc("output file"),
                                                 cl::cat(myOptionCat));
        static cl::extrahelp commonHelp(CommonOptionsParser::HelpMessage);

        auto expectedParser = CommonOptionsParser::create(argc, const_cast<const char **>(argv),
                                                          myOptionCat, cl::Required);
        if (!expectedParser) {
            llvm::errs() << expectedParser.takeError();
            return 0;
        }
        auto &parser = expectedParser.get();

        if (std::error_code ec; (ec = llvm::sys::fs::current_path(g_ctx().initialCwd))) {
            llvm::errs() << "Failed to get current path: " << ec.message() << "\n";
            return 1;
        }
        g_ctx().outputPath = outputOption.getValue();

        // Build statistics index maps
        HLR::SourceStatistics stat;
        if (std::string err; !stat.loadFromJson(statOption.getValue(), err)) {
            llvm::errs() << "Failed to parse statistics file: " << err << "\n";
            return 1;
        }

        {
            size_t i = 0;
            for (const auto &guard : std::as_const(stat.callbackCheckGuardSignatures)) {
                g_ctx().CCGIndexMap[guard] = i;
                i++;
            }

            i = 0;
            for (const auto &guard : std::as_const(stat.functionDecayGuardStats)) {
                g_ctx().FDGIndexMap[guard.first] = i;
                i++;

                std::map<std::string, FDGInstanceInfo> FDGInstanceMap;
                size_t j = 0;
                for (const auto &loc : std::as_const(guard.second.locations)) {
                    FDGInstanceMap[loc.first] = {j, loc.second};
                    ++j;
                }
                g_ctx().FDGInstanceInfoMaps.emplace_back(std::move(FDGInstanceMap));
            }
        }

        ClangTool tool(parser.getCompilations(), parser.getSourcePathList());
        if (int ret = tool.run(newFrontendActionFactory<MyASTFrontendAction>().get()); ret != 0) {
            return ret;
        }

        if (!g_ctx().outBuffer.empty()) {
            std::error_code ec;
            llvm::raw_fd_ostream out(g_ctx().outputPath.empty() ? "-" : g_ctx().outputPath, ec);
            if (ec) {
                llvm::errs() << "Error occurs opening output file: " << ec.message() << "\n";
                return 1;
            }
            out << g_ctx().outBuffer;
        }
        return 0;
    }

}
