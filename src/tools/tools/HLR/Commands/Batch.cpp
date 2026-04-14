// SPDX-License-Identifier: MIT

#include <string>
#include <filesystem>

#include <llvm/Support/Program.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/VirtualFileSystem.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/MemoryBuffer.h>
#include <clang/Tooling/CommonOptionsParser.h>

#include <lorelei/Base/Support/ScopeGuard.h>
#include <lorelei/Base/Support/StringExtras.h>

#include "Utils/SourceStatistics.h"

extern "C" {
extern unsigned char res_FileContext_h_c[];
extern unsigned int res_FileContext_h_c_len;

extern unsigned char res_Warning_txt_c[];
extern unsigned int res_Warning_txt_c_len;
};

using namespace clang;
using namespace clang::tooling;

namespace cl = llvm::cl;

namespace lore::tool::command::batch {

    const char *name = "batch";

    const char *help = "Run batch on input files";

    static llvm::Error parseOptions(int &argc, const char **argv, cl::OptionCategory &cat,
                                    const char *overview = nullptr) {
        cl::ResetAllOptionOccurrences();
        cl::HideUnrelatedOptions(cat);

        std::string err;
        llvm::raw_string_ostream os(err);
        // Stop initializing if command-line option parsing failed.
        if (!cl::ParseCommandLineOptions(argc, argv, overview, &os)) {
            return llvm::make_error<llvm::StringError>(err, llvm::inconvertibleErrorCode());
        }
        cl::PrintOptionValues();
        return llvm::Error::success();
    }

    static llvm::Error runSubCommand(StringRef thisExePath, StringRef subCommand,
                                     const llvm::ArrayRef<std::string> &sourcePathList,
                                     llvm::StringRef outFile, llvm::StringRef buildPath,
                                     const llvm::ArrayRef<StringRef> &extraArgs = {}) {
        llvm::SmallVector<llvm::StringRef, 32> args = {
            thisExePath, subCommand, "-o", outFile, "-p", buildPath,
        };
        args.insert(args.end(), sourcePathList.begin(), sourcePathList.end());
        args.insert(args.end(), extraArgs.begin(), extraArgs.end());

        std::string err;
        int ret = llvm::sys::ExecuteAndWait(thisExePath, args, {}, {}, 0, 0, &err);
        if (ret != 0) {
            return llvm::make_error<llvm::StringError>(
                "Failed to run " + subCommand + " command: " + err, llvm::inconvertibleErrorCode());
        }
        return llvm::Error::success();
    }

    static llvm::Error adjustContent(StringRef filePath, llvm::StringRef header) {
        auto buffer = llvm::MemoryBuffer::getFile(filePath);
        if (auto ec = buffer.getError()) {
            return llvm::make_error<llvm::StringError>(ec.message(),
                                                       llvm::inconvertibleErrorCode());
        }
        // Copy the original content before reopening the same path for writing.
        // Otherwise truncation can invalidate the mapped buffer and trigger SIGBUS.
        std::string oldContent = buffer->get()->getBuffer().str();

        std::error_code ec;
        llvm::raw_fd_ostream out(filePath, ec);
        if (ec) {
            return llvm::make_error<llvm::StringError>(ec.message(),
                                                       llvm::inconvertibleErrorCode());
        }
        out << llvm::format<const char *, const char *>((const char *) res_Warning_txt_c,
                                                        filePath.data(), TOOL_VERSION)
            << "\n\n"
            << header << oldContent;
        return llvm::Error::success();
    }

    int main(int argc, char *argv[]) {
        static cl::OptionCategory myOptionCat("Lorelei Host Library Rewriter - Batch");
        static cl::opt<std::string> statOption("s", cl::desc("Specify TLC stat JSON file"),
                                               cl::value_desc("stat json"), cl::cat(myOptionCat),
                                               cl::Required);
        static cl::opt<std::string> outputDirOption("o",
                                                    cl::desc("Output directory of generated files"),
                                                    cl::Required, cl::cat(myOptionCat));
        static cl::opt<std::string> buildPathOption("p", cl::desc("Build path"), cl::Required,
                                                    cl::cat(myOptionCat));
        static cl::list<std::string> sourcePathsOption(cl::Positional,
                                                       cl::desc("<source0> [... <sourceN>]"),
                                                       cl::OneOrMore, cl::cat(myOptionCat));
        static cl::extrahelp commonHelp(CommonOptionsParser::HelpMessage);

        /// STEP: Parse command line options
        if (auto err = parseOptions(argc, const_cast<const char **>(argv), myOptionCat); err) {
            llvm::errs() << err;
            return 1;
        }

        /// STEP: Load the compilation database
        std::unique_ptr<CompilationDatabase> compilations;
        if (std::string err;
            !(compilations = CompilationDatabase::autoDetectFromDirectory(buildPathOption, err))) {
            llvm::errs() << "Error while trying to load a compilation database:\n"
                         << err << "Running without flags.\n";
            return 1;
        }

        std::vector<std::string> sourcePathList = sourcePathsOption;

        SmallString<128> initialCwd;
        if (std::error_code ec; (ec = llvm::sys::fs::current_path(initialCwd))) {
            llvm::errs() << "Failed to get current path: " << ec.message() << "\n";
            return 1;
        }

        SmallString<128> outputDir = StringRef(outputDirOption.getValue());
        llvm::sys::fs::make_absolute(initialCwd, outputDir);

        const auto &getRelativeToOutputDir = [&](llvm::StringRef path) {
            SmallString<128> fileDir = StringRef(path);
            llvm::sys::fs::make_absolute(initialCwd, fileDir);
            fileDir = llvm::sys::path::parent_path(fileDir);

            auto relativePath =
                std::filesystem::relative(outputDir.str().str(), fileDir.str().str());
            return str::conv<std::filesystem::path>::normalize_separators(relativePath.string(),
                                                                          false);
        };

        SmallString<128> outputCommonHeader = outputDir;
        outputCommonHeader += "/FileContext.h";
        SmallString<128> outputSingleSource = outputDir;
        outputSingleSource += "/FileContext.c";

        /// STEP: Make stat result file
        SmallString<128> statResultFile;
        if (std::error_code ec;
            (ec = llvm::sys::fs::createTemporaryFile("prefix", "suffix", statResultFile))) {
            llvm::errs() << "Failed to create temporary file: " << ec.message() << "\n";
            return 1;
        }

        auto statResultFileRemover = makeScopeGuard([&statResultFile]() {
            if (std::error_code ec; (ec = llvm::sys::fs::remove(statResultFile))) {
                llvm::errs() << "Failed to remove temporary file " << statResultFile << ": "
                             << ec.message() << "\n";
            }
        });

        /// STEP: Call "stat" on all files
        llvm::errs() << "Running stat command on " << sourcePathList.size() << " files...\n";

        auto thisExePath = llvm::sys::fs::getMainExecutable(argv[0], (void *) main);
        if (auto err =
                runSubCommand(thisExePath, "stat", sourcePathList, statResultFile, buildPathOption,
                              statOption.empty() ? ArrayRef<StringRef>()
                                                 : ArrayRef<StringRef>({"-s", statOption}));
            err) {
            llvm::errs() << "Failed to run stat command: " << err << "\n";
            return 1;
        }

        HLR::SourceStatistics stat;
        if (std::string err; !stat.loadFromJson(statResultFile.str().str(), err)) {
            llvm::errs() << "Failed to parse stat result file: " << err << "\n";
            return 1;
        }

        std::set<std::string> filesNeedPatchNormalized;
        for (const auto &file : stat.filesNeedPatch) {
            SmallString<128> normalized = StringRef(file);
            llvm::sys::fs::make_absolute(initialCwd, normalized);
            normalized = std::filesystem::path(normalized.str().str()).lexically_normal().string();
            filesNeedPatchNormalized.insert(normalized.str().str());
        }

        /// STEP: Make output directory
        if (!llvm::sys::fs::exists(outputDir)) {
            if (std::error_code ec; (ec = llvm::sys::fs::create_directories(outputDir))) {
                llvm::errs() << "Failed to create output directory " << outputDir << ": "
                             << ec.message() << "\n";
                return 1;
            }
        }

        /// STEP: Call "mark-macros", "preprocess" and "rewrite" on each file
        llvm::errs() << "Running mark-macros/preprocess/rewrite on " << sourcePathList.size()
                     << " files...\n";
        bool hasAnySourceChange = false;
        std::string fileContextEntrySource;
        for (size_t i = 0; i < sourcePathList.size(); ++i) {
            const auto &file = sourcePathList[i];
            llvm::errs() << "[" + std::to_string(i + 1) + "/" +
                                std::to_string(sourcePathList.size()) + "] Processing file " + file
                         << ".\n";
            SmallString<128> normalizedFile = StringRef(file);
            llvm::sys::fs::make_absolute(initialCwd, normalizedFile);
            normalizedFile =
                std::filesystem::path(normalizedFile.str().str()).lexically_normal().string();
            if (!filesNeedPatchNormalized.count(normalizedFile.str().str())) {
                continue;
            }

            std::string contentBefore;
            if (auto buffer = llvm::MemoryBuffer::getFile(file)) {
                contentBefore = buffer.get()->getBuffer().str();
            } else if (auto ec = buffer.getError()) {
                llvm::errs() << "Failed to read input file before mark-macros " << file << ": "
                             << ec.message() << "\n";
                return 1;
            } else {
                llvm::errs() << "Failed to read input file before mark-macros " << file << "\n";
                return 1;
            }

            if (auto err = runSubCommand(thisExePath, "mark-macros", file, file, buildPathOption,
                                         {"-s", statResultFile});
                err) {
                llvm::errs() << "Failed to run mark-macros command: " << err << "\n";
                return 1;
            }
            if (auto err = runSubCommand(thisExePath, "preprocess", file, file, buildPathOption);
                err) {
                llvm::errs() << "Failed to run preprocess command: " << err << "\n";
                return 1;
            }
            if (auto err = runSubCommand(thisExePath, "rewrite", file, file, buildPathOption,
                                         {"-s", statResultFile});
                err) {
                llvm::errs() << "Failed to run rewrite command: " << err << "\n";
                return 1;
            }

            auto bufferAfter = llvm::MemoryBuffer::getFile(file);
            if (auto ec = bufferAfter.getError()) {
                llvm::errs() << "Failed to read output file after rewrite " << file << ": "
                             << ec.message() << "\n";
                return 1;
            }

            if (bufferAfter.get()->getBuffer() == contentBefore) {
                llvm::errs() << "No source changes were applied by HLR batch for this file.\n";
                continue;
            }

            if (fileContextEntrySource.empty()) {
                fileContextEntrySource = file;
            }
            hasAnySourceChange = true;

            // Add common header inclusion
            std::string fileToInclude = getRelativeToOutputDir(file) + "/FileContext.h";
            if (auto err = adjustContent(file, "#include \"" + fileToInclude + "\"\n"); err) {
                llvm::errs() << "Failed to adjust content: " << err << "\n";
                return 1;
            }
        }

        if (!hasAnySourceChange) {
            llvm::errs() << "No source changes were applied by HLR batch.\n";
            return 0;
        }

        /// STEP: Write common header
        {
            std::error_code ec;
            llvm::raw_fd_ostream out(outputCommonHeader, ec);
            if (ec) {
                llvm::errs() << "Failed to create common header file " << outputCommonHeader << ": "
                             << ec.message() << "\n";
                return 1;
            }
            out << "#pragma once\n\n";
            out << StringRef((const char *) res_FileContext_h_c);
            out << "\n\n";

            size_t i = 0;
            for (const auto &guard : stat.callbackCheckGuardSignatures) {
                out << "LORE_CCG_DECL(" << i + 1 << ")\n";
                ++i;
            }
            out << "\n";

            i = 0;
            for (const auto &guard : stat.functionDecayGuardStats) {
                size_t j = 0;
                for (const auto &_ : guard.second.locations) {
                    out << "LORE_FDG_DECL(" << i + 1 << ", " << j + 1 << ")\n";
                    ++j;
                }
                ++i;
            }
            out << "\n";

            i = 0;
            for (const auto &guard : stat.callbackCheckGuardSignatures) {
                out << "#define LORE_CCG_" << i + 1 << "(X) LORE_CCG_GET(" << i + 1 << ", (X))\n";
                ++i;
            }
            out << "\n";

            i = 0;
            for (const auto &guard : stat.functionDecayGuardStats) {
                size_t j = 0;
                for (const auto &_ : guard.second.locations) {
                    out << "#define LORE_FDG_" << i + 1 << "_" << j + 1 << "(X) LORE_FDG_GET("
                        << i + 1 << ", " << j + 1 << ", (X))\n";
                    ++j;
                }
                ++i;
            }
        }

        /// STEP: Write single source
        {
            std::error_code ec;
            llvm::raw_fd_ostream out(outputSingleSource, ec);
            if (ec) {
                llvm::errs() << "Failed to create single source file " << outputSingleSource << ": "
                             << ec.message() << "\n";
                return 1;
            }

            size_t i = 0;
            for (const auto &guard : stat.callbackCheckGuardSignatures) {
                out << "LORE_CCG_IMPL(" << i + 1 << ")\n";
                ++i;
            }

            i = 0;
            out << "static struct LoreStaticCallCheckGuardInfo LoreFileContext_CCGs[] = {\n";
            for (const auto &guard : stat.callbackCheckGuardSignatures) {
                out << "    {\"" << guard << "\", &LoreFileContext_CCG_Tramp_" << i + 1 << "},\n";
                ++i;
            }
            out << "};\n\n";

            i = 0;
            for (const auto &guard : stat.functionDecayGuardStats) {
                out << "static struct LoreStaticFunctionDecayGuardPair *LoreFileContext_FDG_Proc_"
                    << i + 1 << "_instances[] = {\n";
                size_t j = 0;
                for (const auto &_ : guard.second.locations) {
                    out << "    &LoreFileContext_FDG_Proc_" << i + 1 << "_" << j + 1 << ",\n";
                    ++j;
                }
                out << "};\n\n";
                ++i;
            }

            i = 0;
            out << "static struct LoreStaticFunctionDecayGuardInfo LoreFileContext_FDGs[] = {\n";
            for (const auto &guard : stat.functionDecayGuardStats) {
                out << "    {\"" << guard.first << "\", " << guard.second.locations.size()
                    << ", LoreFileContext_FDG_Proc_" << i + 1 << "_instances},\n";
                ++i;
            }
            out << "};\n\n";

            out << "__LORE_PRIV__ struct LoreFileContext LoreFileContext_instance = {\n";
            out << "    .runtimeContext = 0,\n";
            out << "    .emuAddr = 0,\n";
            out << "    .setThreadCallback = 0,\n";
            out << "    .numCCGs = " << stat.callbackCheckGuardSignatures.size() << ",\n";
            out << "    .CCGs = LoreFileContext_CCGs,\n";
            out << "#ifdef LORE_DISABLE_FDG\n";
            out << "    .numFDGs = 0,\n";
            out << "    .FDGs = 0,\n";
            out << "#else\n";
            out << "    .numFDGs = " << stat.functionDecayGuardStats.size() << ",\n";
            out << "    .FDGs = LoreFileContext_FDGs,\n";
            out << "#endif\n";
            out << "};\n\n";

            out << "__LORE_EXPORT__ struct LoreFileContext *__Lore_GetFileContext() {\n";
            out << "    return &LoreFileContext_instance;\n";
            out << "}\n\n";
        }

        /// STEP: Append single source inclusion to the first source file
        {
            std::error_code ec;
            llvm::raw_fd_ostream out(fileContextEntrySource, ec, llvm::sys::fs::OF_Append);
            if (ec) {
                llvm::errs() << "Failed to append single source inclusion to the first source file "
                             << fileContextEntrySource << ": " << ec.message() << "\n";
                return 1;
            }

            std::string fileToInclude =
                getRelativeToOutputDir(fileContextEntrySource) + "/FileContext.c";
            out << "\n";
            out << "#include \"" << fileToInclude << "\"\n";
        }
        return 0;
    }

}
