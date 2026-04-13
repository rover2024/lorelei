// SPDX-License-Identifier: MIT

#include <string>
#include <map>
#include <filesystem>
#include <string>
#include <system_error>

#include <llvm/Support/CommandLine.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/Tooling/ArgumentsAdjusters.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>

#include <lorelei/Tools/TLCApi/Core/Pass.h>
#include <lorelei/Tools/TLCApi/Core/DocumentContext.h>
#include <lorelei/Tools/TLCApi/Utils/ManifestStatistics.h>
#include <lorelei/Tools/ToolUtils/CommonMatchFinder.h>
#include <lorelei/Tools/ToolUtils/TypeUtils.h>

extern "C" {
extern unsigned char res_Warning_txt_c[];
extern unsigned int res_Warning_txt_c_len;
};

using namespace clang;
using namespace clang::tooling;
using namespace clang::ast_matchers;

namespace cl = llvm::cl;

namespace lore::tool::command::generate {

    struct GlobalContext {
        /// Global states
        SmallString<128> initialCwd;
        std::string manifestStatPath;
        std::string outputPath;

        TLC::ManifestStatistics stat;
        TLC::DocumentContext doc;

        std::string outBuffer;
    };

    static GlobalContext &g_ctx() {
        static GlobalContext instance;
        return instance;
    }

    static std::string buildBridgeTranslationUnitText(llvm::StringRef manifestPath,
                                                      const TLC::ManifestStatistics &stat) {
        const auto &escapeForCIncludePath = [](llvm::StringRef path) {
            std::string out;
            out.reserve(path.size());
            for (char ch : path) {
                if (ch == '\\' || ch == '"') {
                    out.push_back('\\');
                }
                out.push_back(ch);
            }
            return out;
        };

        std::string text;
        text += "// Auto-generated virtual translation unit for TLC generate\n";
        text += "#include \"";
        text += escapeForCIncludePath(manifestPath);
        text += "\"\n\n";
        text += "namespace lore::thunk::__bridge__ {\n";

        int i = 0;
        for (const auto &[_, callbackInfo] : stat.callbacks) {
            i++;
            text += "using ";
            if (!callbackInfo.alias.empty()) {
                text += callbackInfo.alias;
            } else {
                text += "PFN_UnknownCallback_" + std::to_string(i);
            }
            text += " = " + callbackInfo.signature + ";\n";
        }

        text += "}\n";
        return text;
    }

    class MyASTConsumer : public ASTConsumer {
    public:
        void HandleTranslationUnit(ASTContext &ast) override {
            g_ctx().doc.handleTranslationUnit(ast);
        }
    };

    class MyASTFrontendAction : public ASTFrontendAction {
    public:
        bool BeginSourceFileAction(CompilerInstance &CI) override {
            if (!g_ctx().doc.beginSourceFileAction(CI)) {
                return false;
            }
            return true;
        }

        std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &, StringRef) override {
            return std::make_unique<MyASTConsumer>();
        }

        void EndSourceFileAction() override {
            CompilerInstance &CI = getCompilerInstance();
            DiagnosticsEngine &DE = CI.getDiagnostics();
            if (DE.hasErrorOccurred()) {
                return;
            }

            g_ctx().doc.endSourceFileAction();

            // Generate output
            {
                llvm::raw_string_ostream out(g_ctx().outBuffer);
                out << llvm::format<const char *, const char *>((const char *) res_Warning_txt_c,
                                                                g_ctx().manifestStatPath.c_str(),
                                                                TOOL_VERSION)
                    << "\n";
                out << "\n";
                g_ctx().doc.generateOutput(out);
            }
        }
    };

    const char *name = "generate";

    const char *help = "Generate thunk library sources for input files";

    int main(int argc, char *argv[]) {
        static cl::OptionCategory myOptionCat("Lorelei TLC - Generate");
        static cl::opt<TLC::DocumentContext::Mode> modeOption(
            "m", cl::desc("Generate mode"),
            cl::values(
                clEnumValN(TLC::DocumentContext::Guest, "guest", "Generate guest thunk source"),
                clEnumValN(TLC::DocumentContext::Host, "host", "Generate host thunk source")),
            cl::cat(myOptionCat), cl::Required);
        static cl::opt<std::string> statOption("s", cl::desc("Specify TLC stat JSON file"),
                                               cl::value_desc("stat json"), cl::cat(myOptionCat),
                                               cl::Required);
        static cl::opt<std::string> outputOption("o", cl::desc("Specify output file"),
                                                 cl::value_desc("output file"),
                                                 cl::cat(myOptionCat));
        static cl::extrahelp commonHelp(CommonOptionsParser::HelpMessage);

        auto expectedParser = CommonOptionsParser::create(argc, const_cast<const char **>(argv),
                                                          myOptionCat, cl::Required);
        if (!expectedParser) {
            llvm::errs() << expectedParser.takeError();
            return 1;
        }
        auto &parser = expectedParser.get();
        if (std::error_code ec; (ec = llvm::sys::fs::current_path(g_ctx().initialCwd))) {
            llvm::errs() << "Failed to get current path: " << ec.message() << "\n";
            return 1;
        }

        const auto &sourcePathList = parser.getSourcePathList();
        const auto manifestPath = sourcePathList.front();
        std::filesystem::path manifestIncludePathFs(manifestPath);
        if (manifestIncludePathFs.is_relative()) {
            manifestIncludePathFs = std::filesystem::path(std::string(g_ctx().initialCwd.str())) /
                                    manifestIncludePathFs;
        }
        manifestIncludePathFs = manifestIncludePathFs.lexically_normal();
        const auto manifestIncludePath = manifestIncludePathFs.string();

        TLC::ManifestStatistics stat;
        if (std::string err; !stat.loadFromJson(statOption.getValue(), err)) {
            llvm::errs() << "error: failed to parse stat json: " << err << "\n";
            return 1;
        }
        g_ctx().manifestStatPath = manifestPath;
        g_ctx().outputPath = outputOption.getValue();
        g_ctx().stat = std::move(stat);

        auto mode = modeOption.getValue();

        // Build requested data and initialize document context
        {
            TLC::DocumentContext::RequestedProcData requestedData;

            for (const auto &[name, _] :
                 g_ctx().stat.functions[TLC::ManifestStatistics::GuestToHost]) {
                (void) _;
                requestedData.functions[TLC::ProcSnippet::GuestToHost].insert(name);
            }
            for (const auto &[name, _] :
                 g_ctx().stat.functions[TLC::ManifestStatistics::HostToGuest]) {
                (void) _;
                requestedData.functions[TLC::ProcSnippet::HostToGuest].insert(name);
            }
            for (const auto &[signature, info] : g_ctx().stat.callbacks) {
                requestedData.callbacks[signature] = info.alias;
            }

            g_ctx().doc.initialize(mode, g_ctx().stat.fileName, manifestPath,
                                   std::move(requestedData));
        }

        const auto manifestFsPath = std::filesystem::path(manifestPath);
        const std::string bridgePath =
            (manifestFsPath.parent_path() /
             (manifestFsPath.stem().string() + "_TLCBridge.cpp"))
                .lexically_normal()
                .string();
        const std::string bridgeText =
            buildBridgeTranslationUnitText(manifestIncludePath, g_ctx().stat);

        ClangTool tool(parser.getCompilations(), {manifestPath});
        tool.mapVirtualFile(bridgePath, bridgeText);
        tool.appendArgumentsAdjuster(
            [manifestPath = manifestPath, bridgePath = bridgePath,
             initialCwd = std::string(g_ctx().initialCwd.str())](const CommandLineArguments &args,
                                                                  llvm::StringRef) {
                const auto normalizePath = [&](llvm::StringRef pathStr) {
                    std::filesystem::path p(pathStr.str());
                    if (p.is_relative()) {
                        p = std::filesystem::path(initialCwd) / p;
                    }
                    p = p.lexically_normal();
                    return p.string();
                };

                const auto manifestNorm = normalizePath(manifestPath);
                CommandLineArguments adjusted = args;
                for (auto &arg : adjusted) {
                    if (arg == manifestPath || normalizePath(arg) == manifestNorm) {
                        arg = bridgePath;
                    }
                }
                return adjusted;
            });
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
