#include <filesystem>
#include <cstdlib>

#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>

#include <llvm/Support/CommandLine.h>

#include <lorelei-c/Core/ProcInfo_c.h>
#include <lorelei/Utils/Text/ConfigFile.h>
#include <lorelei/TLC/Plugin/ASTActionPlugin.h>
#include <lorelei/TLC/ASTMetaItem/MetaProcDescItem.h>
#include <lorelei/TLC/Core/DocumentContext.h>

#include <stdcorelib/support/sharedlibrary.h>
#include <stdcorelib/path.h>

namespace cl = llvm::cl;

using namespace clang;
using namespace clang::tooling;

static constexpr const char GENERATED_FILE_WARNING[] =
    R"(/****************************************************************************
** Lorelei thunk library code from reading C++ file '%s'
**
** Created by: Lorelei Thunk Library compiler
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/
)";

namespace TLC::commands::thunkGen {

    /// GlobalContext - Stores global objects for the tool.
    struct GlobalContext {
        std::filesystem::path workingDirectory;

        // Input files
        std::filesystem::path thunkConfigPath;
        std::filesystem::path inputFilePath;
        llvm::SmallVector<std::filesystem::path> preincPaths;

        // Output files
        std::filesystem::path outputCallbacksFilePath;
        std::filesystem::path outputFilePath;

        // Plugins
        llvm::SmallVector<std::filesystem::path> pluginPaths;
        llvm::SmallVector<stdc::SharedLibrary> plugins;
        llvm::SmallVector<ASTActionPlugin *> pluginInstances;

        // User data
        lore::ConfigFile thunkConfig;

        TLC::DocumentContext docCtx;

        void loadPlugins() {
            for (const auto &path : pluginPaths) {
                stdc::SharedLibrary plugin;
                if (!plugin.open(path, stdc::SharedLibrary::ResolveAllSymbolsHint)) {
                    llvm::errs() << "Failed to load plugin \"" << path
                                 << "\": " << plugin.lastError() << "\n";
                    continue;
                }
                using Entry = ASTActionPlugin *(*) ();
                Entry entry = (Entry) plugin.resolve("TLC_PluginInstance");
                if (!entry) {
                    llvm::errs() << "Failed to resolve entry for plugin \"" << path
                                 << "\": " << plugin.lastError() << "\n";
                    continue;
                }
                auto instance = entry();
                if (!instance) {
                    llvm::errs() << "Failed to create instance for plugin \"" << path
                                 << "\": " << plugin.lastError() << "\n";
                    continue;
                }
                pluginInstances.push_back(instance);
                plugins.push_back(std::move(plugin));
            }

            for (const auto &plugin : pluginInstances) {
                plugin->initialize();
            }
        }
    };

    static GlobalContext g_ctx;

    /// MyConsumer - The consumer that handles the AST and calls the plugins.
    class MyConsumer : public ASTConsumer {
    public:
        MyConsumer(llvm::SmallVector<std::unique_ptr<ASTConsumerAddOn>> addOns)
            : addOns(std::move(addOns)) {
        }

        void Initialize(ASTContext &Context) override {
            g_ctx.docCtx.initialize(Context, g_ctx.thunkConfig);
            for (auto &addOn : addOns) {
                addOn->initialize(Context);
            }
        }

        void HandleTranslationUnit(ASTContext &Context) override {
            g_ctx.docCtx.handleTranslationUnit(Context);
            for (auto &addOn : addOns) {
                addOn->handleTranslationUnit(Context);
            }
        }

    private:
        llvm::SmallVector<std::unique_ptr<ASTConsumerAddOn>> addOns;
    };

    /// MyAction - The frontend action that creates the consumer and calls the plugins.
    class MyASTFrontendAction : public ASTFrontendAction {
    public:
        bool BeginSourceFileAction(CompilerInstance &CI) override {
            if (!g_ctx.docCtx.beginSourceFileAction(CI)) {
                return false;
            }
            // Call begin for each plugin
            for (const auto &plugin : g_ctx.pluginInstances) {
                if (!plugin->beginSourceFileAction(CI)) {
                    return false;
                }
            }
            return true;
        }

        std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                       StringRef InFile) override {
            // Create add-on instances for plugins
            llvm::SmallVector<std::unique_ptr<ASTConsumerAddOn>> addOns;
            for (const auto &plugin : g_ctx.pluginInstances) {
                auto addOn = plugin->createASTConsumerAddOn(CI, InFile);
                if (addOn) {
                    addOns.push_back(std::move(addOn));
                }
            }
            return std::make_unique<MyConsumer>(std::move(addOns));
        }

        void EndSourceFileAction() override {
            std::filesystem::current_path(g_ctx.workingDirectory);

            g_ctx.docCtx.endSourceFileAction();
            // Call end for each plugin
            for (const auto &plugin : g_ctx.pluginInstances) {
                plugin->endSourceFileAction();
            }

            // Generate output
            {
                std::error_code ec;
                llvm::raw_fd_ostream file(
                    g_ctx.outputFilePath.empty() ? "-" : g_ctx.outputFilePath.string(), ec);
                if (ec) {
                    llvm::errs() << "Error occurs opening output file \""
                                 << g_ctx.outputFilePath.string() << "\": " << ec.message() << "\n";
                }

                file << llvm::format(GENERATED_FILE_WARNING,
                                     g_ctx.inputFilePath.filename().string().c_str())
                     << "\n";
                for (const auto &path : g_ctx.preincPaths) {
                    file << "#include \"" << path.string() << "\"\n";
                }
                file << "\n";
                g_ctx.docCtx.generateOutput(file);
            }

            // Generate callbacks if necessary
            if (!g_ctx.outputCallbacksFilePath.empty()) {
                std::error_code ec;
                llvm::raw_fd_ostream file(g_ctx.outputCallbacksFilePath.string(), ec);
                if (ec) {
                    llvm::errs() << "Error occurs opening output file \""
                                 << g_ctx.outputCallbacksFilePath.string() << "\": " << ec.message()
                                 << "\n";
                }
                generateCallbacks(file);
            }
        }

    private:
        void generateCallbacks(llvm::raw_ostream &os) {
        }
    };

    int main(int argc, char *argv[]) {
        cl::OptionCategory optionCategory("Thunk Generator Options");

        /// -s <file>
        ///   Symbol list file.
        cl::opt<std::string> thunkConfigOption("s", cl::desc("Thunk library configuration file"),
                                               cl::value_desc("file"), cl::cat(optionCategory));

        /// --out-callbacks <file>
        ///   Output callbacks file.
        cl::opt<std::string> outputCallbacksFileOption(
            "out-callbacks", cl::desc("Output callbacks file"), cl::value_desc("file"),
            cl::cat(optionCategory));

        /// -o <file>
        ///   Output header file.
        cl::opt<std::string> outputFileOption("o", cl::desc("Output header file"),
                                              cl::value_desc("file"), cl::cat(optionCategory));

        /// --preinc <file>
        ///   Pre-include files.
        cl::list<std::string> preincludeListOptions("preinc", cl::desc("Pre-include files"),
                                                    cl::value_desc("file"),
                                                    cl::cat(optionCategory));

        /// --plugin <file>
        ///   ASTConsumer plugins to load.
        cl::list<std::string> pluginListOptions("plugin", cl::desc("ASTConsumer plugins to load"),
                                                cl::value_desc("file"), cl::cat(optionCategory));

        auto expectedParser =
            CommonOptionsParser::create(argc, (const char **) argv, optionCategory);
        if (!expectedParser) {
            llvm::errs() << expectedParser.takeError();
            return 1;
        }

        g_ctx.workingDirectory = std::filesystem::current_path();

        if (auto path = thunkConfigOption.getValue(); !path.empty()) {
            g_ctx.thunkConfigPath = std::filesystem::path(path);
            std::ignore = g_ctx.thunkConfig.load(g_ctx.thunkConfigPath);
        }
        if (auto path = outputCallbacksFileOption.getValue(); !path.empty()) {
            g_ctx.outputCallbacksFilePath = std::filesystem::path(path);
        }
        if (auto path = outputFileOption.getValue(); !path.empty()) {
            g_ctx.outputFilePath = std::filesystem::path(path);
        }
        for (const auto &path : preincludeListOptions) {
            g_ctx.preincPaths.push_back(stdc::clean_path(std::filesystem::absolute(path)));
        }
        g_ctx.pluginPaths.assign(pluginListOptions.begin(), pluginListOptions.end());
        if (auto path = outputFileOption.getValue(); !path.empty()) {
            g_ctx.outputFilePath = std::filesystem::path(path);
        }
        g_ctx.loadPlugins();

        auto &parser = expectedParser.get();
        auto &sourcePathList = parser.getSourcePathList();
        if (sourcePathList.size() != 1) {
            llvm::errs() << "This commond only supports one source file.\n";
            return 1;
        }
        ClangTool tool(parser.getCompilations(), sourcePathList);
        g_ctx.inputFilePath = sourcePathList.front();
        return tool.run(newFrontendActionFactory<MyASTFrontendAction>().get());
    }

}