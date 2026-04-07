// SPDX-License-Identifier: MIT

#include <string>
#include <sstream>

#include <llvm/Support/Program.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>

#include <lorelei/Base/Support/ConfigFile.h>
#include <lorelei/Tools/ToolUtils/TypeUtils.h>

#include "Utils/ASTConsumers.h"
#include "Utils/SourceStatistics.h"

using namespace clang;
using namespace clang::tooling;

namespace cl = llvm::cl;

namespace lore::tool::command::stat {

    struct GlobalContext {
        /// Global states
        SmallString<128> initialCwd;
        std::string configPath;
        std::string outputPath;

        /// Runtime
        std::set<std::string> interestedSignatures;
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
                auto typeStr = getTypeString(T);
                if (!g_ctx().interestedSignatures.count(typeStr)) {
                    continue;
                }
                g_ctx().sourceStat.callbackCheckGuardSignatures.insert(typeStr);
            }

            for (const auto &E : fileData.functionDecayExprList) {
                auto FD = E->getDecl()->getAsFunction();
                auto T = AST.getPointerType(FD->getType());
                auto typeStr = getTypeString(T);
                if (!g_ctx().interestedSignatures.count(typeStr)) {
                    continue;
                }
                assert(FD);
                g_ctx().sourceStat.functionDecayGuardStats[typeStr].locations.insert(
                    std::make_pair(FD->getBeginLoc().printToString(SM), inFile));
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
        static cl::OptionCategory myOptionCat("Lorelei Host Library Rewriter - Statistics");
        static cl::opt<std::string> configOption(
            "c", cl::desc("Specify file containing signatures of interest"),
            cl::value_desc("config file"), cl::cat(myOptionCat));
        static cl::opt<std::string> outputOption("o", cl::desc("Specify output file"),
                                                 cl::value_desc("output file"),
                                                 cl::cat(myOptionCat));
        static cl::extrahelp commonHelp(CommonOptionsParser::HelpMessage);

        auto expectedParser =
            CommonOptionsParser::create(argc, const_cast<const char **>(argv), myOptionCat);
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

        /// STEP: Read config file
        {
            auto buffer = llvm::MemoryBuffer::getFile(configOption.getValue());
            if (std::error_code EC = buffer.getError()) {
                llvm::errs() << "Failed to open config file: " << EC.message() << "\n";
                return 1;
            }
            std::istringstream iss(buffer->get()->getBuffer().str());
            std::string line;
            int i = 0;
            while (std::getline(iss, line)) {
                StringRef trimmedLine = StringRef(line).trim();
                if (trimmedLine.empty() || trimmedLine.starts_with("#")) {
                    continue;
                }
                g_ctx().interestedSignatures.insert(trimmedLine.str());
            }
        }

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
