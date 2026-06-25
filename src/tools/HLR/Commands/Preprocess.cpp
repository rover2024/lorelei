// SPDX-License-Identifier: MIT
#include <cstdlib>

#include <clang/Basic/SourceManager.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <llvm/Support/Regex.h>

#include "MarkMacros.h"

using namespace clang;
using namespace clang::tooling;

namespace cl = llvm::cl;
namespace fs = std::filesystem;

namespace lore::tool::command::preprocess {

    using mark_macros::MACRO_BEGIN_TAG;
    using mark_macros::MACRO_END_TAG;

    struct GlobalContext {
        /// Global states
        SmallString<128> initialCwd;
        std::string outputPath;

        /// Runtime data
        std::string outBuffer;
    };

    static GlobalContext &g_ctx() {
        static GlobalContext instance;
        return instance;
    }

    static std::string replaceMacros(const std::string_view &orgCode,
                                     const std::string_view &preprocessedCode) {
        // Collect expansions
        llvm::SmallVector<std::string_view> expansions;
        {
            size_t index = 0;
            while (true) {
                size_t beginIndex = preprocessedCode.find(MACRO_BEGIN_TAG, index);
                if (beginIndex == std::string::npos) {
                    break;
                }
                size_t contentIndex = beginIndex + sizeof(MACRO_BEGIN_TAG) - 1;
                size_t endIndex = preprocessedCode.find(MACRO_END_TAG, contentIndex);
                if (endIndex == std::string::npos) {
                    break;
                }
                expansions.emplace_back(preprocessedCode.data() + contentIndex,
                                        endIndex - contentIndex);
                index = endIndex + sizeof(MACRO_END_TAG) - 1;
            }
        }

        // Replace expansions in original code
        SmallVector<std::string_view> resultViews;
        {
            size_t orgIndex = 0;
            size_t expansionIndex = 0;

            while (orgIndex < orgCode.size()) {
                size_t beginIndex = orgCode.find(MACRO_BEGIN_TAG, orgIndex);
                if (beginIndex == std::string::npos) {
                    resultViews.emplace_back(orgCode.data() + orgIndex);
                    break;
                }
                resultViews.emplace_back(orgCode.data() + orgIndex, beginIndex - orgIndex);

                size_t contentIndex = beginIndex + sizeof(MACRO_BEGIN_TAG) - 1;
                size_t endIndex = orgCode.find(MACRO_END_TAG, contentIndex);

                if (endIndex == std::string::npos) {
                    resultViews.emplace_back(orgCode.data() + beginIndex);
                    break;
                }
                if (expansionIndex < expansions.size()) {
                    resultViews.emplace_back(expansions[expansionIndex]);
                    expansionIndex++;
                } else {
                    resultViews.emplace_back(orgCode.data() + contentIndex,
                                             endIndex - contentIndex);
                }
                orgIndex = endIndex + sizeof(MACRO_END_TAG) - 1;
            }

            if (expansionIndex < expansions.size()) {
                llvm::errs() << "Warning: Found " << expansions.size() << " expansions but only "
                             << expansionIndex << " macro tags in original code\n";
            }
        }

        std::string result;
        {
            size_t expectedSize = 0;
            for (auto &view : resultViews) {
                expectedSize += view.size();
            }
            result.reserve(expectedSize);
            for (auto &view : resultViews) {
                result += view;
            }
        }
        return result;
    }

    class MyASTFrontendAction : public PrintPreprocessedAction {
    public:
        void ExecuteAction() override {
            CompilerInstance &CI = getCompilerInstance();
            if (CI.getLangOpts().CPlusPlus) {
                llvm::errs() << "error: HLR preprocess only supports C input. "
                                "Use C language flags (e.g. -xc -std=c11).\n";
                std::exit(1);
            }
            SourceManager &SM = CI.getSourceManager();
            PreprocessorOutputOptions &PPOpts = CI.getPreprocessorOutputOpts();

            PPOpts.ShowCPP = true;
            PPOpts.ShowComments = true;
            PPOpts.ShowMacroComments = false;
            PPOpts.ShowMacros = false;
            PPOpts.ShowIncludeDirectives = false;
            PPOpts.ShowLineMarkers = false;

            // Print preprocessed code to string
            std::string preprocessedCode;
            {
                llvm::raw_string_ostream out(preprocessedCode);
                DoPrintPreprocessedInput(CI.getPreprocessor(), &out,
                                         CI.getPreprocessorOutputOpts());
                out.flush();
            }

            // Read main file
            std::string_view orgCode;
            {
                bool invalid = false;
                llvm::StringRef content = SM.getBufferData(SM.getMainFileID(), &invalid);
                if (invalid) {
                    llvm::errs() << "Failed to get main file content\n";
                    std::abort();
                }
                orgCode = content;
            }
            g_ctx().outBuffer = replaceMacros(orgCode, preprocessedCode);
        }
    };

    const char *name = "preprocess";

    const char *help = "Execute macro expansion and replace marked macros";

    int main(int argc, char *argv[]) {
        static cl::OptionCategory myOptionCat(
            "Lorelei Host Library Rewriter - Preprocess (Single File)");
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
