// SPDX-License-Identifier: MIT

#include <string>
#include <filesystem>

#include <llvm/Support/Program.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>

#include <LoreTools/Basic/TypeUtils.h>
#include <LoreTools/HLRUtils/ASTConsumers.h>
#include <LoreTools/HLRUtils/SourceStatistics.h>

using namespace clang;
using namespace clang::tooling;

namespace cl = llvm::cl;
namespace fs = std::filesystem;

namespace lore::tool::command::stat {

    struct GlobalContext {
        /// Global states
        std::filesystem::path initialCwd;
        std::string configPath;
        std::string outputPath;

        /// Runtime data
        HLR::SourceStatistics sourceStat;
    };

    static GlobalContext &g_ctx() {
        static GlobalContext instance;
        return instance;
    }

    class MyASTFrontendAction : public ASTFrontendAction {
    public:
        void EndSourceFileAction() override {
            auto &CI = getCompilerInstance();
            auto &SM = CI.getSourceManager();
            auto &AST = CI.getASTContext();

            auto inFile = getCurrentFile().str();
            const auto &fileData = _fileDataMap[inFile];

            for (const auto &E : fileData.callbackInvokeExprList) {
                auto T = realCalleeType(E, AST);
                g_ctx().sourceStat.callbackCheckGuardSignatures.insert(getTypeString(T));
            }

            for (const auto &E : fileData.functionDecayExprList) {
                auto FD = E->getDecl()->getAsFunction();
                auto T = AST.getPointerType(FD->getType());
                assert(FD);
                g_ctx().sourceStat.functionDecayGuardStats[getTypeString(T)].locations.insert(
                    FD->getBeginLoc().printToString(SM));
            }
        }

        std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                       StringRef InFile) override {
            auto &fileData = _fileDataMap[InFile.str()];
            return std::make_unique<HLR::FunctionExprConsumer>(fileData.callbackInvokeExprList,
                                                               fileData.functionDecayExprList);
        }

    private:
        struct ConsumedFileData {
            llvm::SmallVector<const CallExpr *, 20> callbackInvokeExprList;
            llvm::SmallVector<const DeclRefExpr *, 20> functionDecayExprList;
        };
        std::map<std::string, ConsumedFileData> _fileDataMap;
    };

    const char *name = "stat";

    const char *help = "Probe statistics of input files";

    int main(int argc, char *argv[]) {
        cl::OptionCategory myOptionCat("Lorelei Host Library Rewriter - Statistics");
        cl::opt<std::string> configOption(
            "c", cl::desc("Specify file containing signatures of interest"),
            cl::value_desc("config file"), cl::cat(myOptionCat));
        cl::opt<std::string> outputOption("o", cl::desc("Specify output file"),
                                          cl::value_desc("output file"), cl::cat(myOptionCat));
        cl::extrahelp commonHelp(CommonOptionsParser::HelpMessage);

        auto expectedParser =
            CommonOptionsParser::create(argc, const_cast<const char **>(argv), myOptionCat);
        if (!expectedParser) {
            llvm::errs() << expectedParser.takeError();
            return 0;
        }
        auto &parser = expectedParser.get();

        g_ctx().initialCwd = fs::current_path();
        g_ctx().outputPath = outputOption.getValue();

        ClangTool tool(parser.getCompilations(), parser.getSourcePathList());
        if (int ret = tool.run(newFrontendActionFactory<MyASTFrontendAction>().get()); ret == 1) {
            return ret;
        }

        if (std::string err; !g_ctx().sourceStat.saveAsJson(
                g_ctx().outputPath.empty() ? "-" : g_ctx().outputPath, err)) {
            llvm::errs() << "Error occurs opening output file: " << err << "\n";
            return 1;
        }
        return 0;
    }

}
