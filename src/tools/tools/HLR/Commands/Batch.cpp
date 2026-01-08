// SPDX-License-Identifier: MIT

#include <string>
#include <filesystem>

#include <llvm/Support/Program.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/VirtualFileSystem.h>
#include <llvm/Support/CommandLine.h>
#include <clang/Tooling/CommonOptionsParser.h>

#include <LoreBase/CoreLib/ADT/ScopeGuard.h>
#include <LoreTools/HLRUtils/SourceStatistics.h>

extern "C" {
extern unsigned char res_FileContext_h_c[];
extern unsigned int res_FileContext_h_c_len;
};

using namespace clang;
using namespace clang::tooling;

namespace cl = llvm::cl;
namespace fs = std::filesystem;

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

    static llvm::Error runStat(const std::string &thisExePath,
                               const llvm::ArrayRef<std::string> &sourcePathList,
                               llvm::StringRef statResultFile, llvm::StringRef buildPath) {
        llvm::SmallVector<llvm::StringRef, 32> args = {
            thisExePath, "stat", "-o", statResultFile, "-p", buildPath,
        };
        args.insert(args.end(), sourcePathList.begin(), sourcePathList.end());

        std::string err;
        if (int ret;
            (ret = llvm::sys::ExecuteAndWait(thisExePath, args, {}, {}, 0, 0, &err)) != 0) {
            return llvm::make_error<llvm::StringError>("Failed to run stat command: " + err,
                                                       llvm::inconvertibleErrorCode());
        }
        return llvm::Error::success();
    }

    static llvm::Error runMarkMacros(const std::string &thisExePath, llvm::StringRef sourcePath,
                                     llvm::StringRef outFile, llvm::StringRef buildPath) {
        llvm::SmallVector<llvm::StringRef, 8> args = {
            thisExePath, "mark-macros", "-o", outFile, "-p", buildPath, sourcePath,
        };
        std::string err;
        if (int ret;
            (ret = llvm::sys::ExecuteAndWait(thisExePath, args, {}, {}, 0, 0, &err)) != 0) {
            return llvm::make_error<llvm::StringError>("Failed to run mark-macros command: " + err,
                                                       llvm::inconvertibleErrorCode());
        }
        return llvm::Error::success();
    }

    static llvm::Error runPreprocess(const std::string &thisExePath, llvm::StringRef sourcePath,
                                     llvm::StringRef outFile, llvm::StringRef buildPath) {
        llvm::SmallVector<llvm::StringRef, 8> args = {
            thisExePath, "preprocess", "-o", outFile, "-p", buildPath, sourcePath,
        };
        std::string err;
        if (int ret;
            (ret = llvm::sys::ExecuteAndWait(thisExePath, args, {}, {}, 0, 0, &err)) != 0) {
            return llvm::make_error<llvm::StringError>("Failed to run preprocess command: " + err,
                                                       llvm::inconvertibleErrorCode());
        }
        return llvm::Error::success();
    }

    int main(int argc, char *argv[]) {
        static cl::OptionCategory myOptionCat("Lorelei Host Library Rewriter - Batch");
        static cl::opt<std::string> outputDirOption(
            "o", cl::desc("Output directory of generated files"), cl::Required,
            cl::cat(myOptionCat), cl::sub(cl::SubCommand::getAll()));
        static cl::opt<std::string> buildPathOption("p", cl::desc("Build path"), cl::Required,
                                                    cl::cat(myOptionCat),
                                                    cl::sub(cl::SubCommand::getAll()));
        static cl::list<std::string> sourcePathsOption(
            cl::Positional, cl::desc("<source0> [... <sourceN>]"),
            cl::NumOccurrencesFlag::OneOrMore, cl::cat(myOptionCat),
            cl::sub(cl::SubCommand::getAll()));
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
        if (auto err = runStat(thisExePath, sourcePathList, statResultFile, buildPathOption); err) {
            llvm::errs() << "Failed to run stat command: " << err << "\n";
            return 1;
        }

        HLR::SourceStatistics stat;
        if (std::string err; !stat.loadFromJson(statResultFile.str().str(), err)) {
            llvm::errs() << "Failed to parse stat result file: " << err << "\n";
            return 1;
        }

        /// STEP: Call "mark-macros", "preprocess" and "rewrite" on each file


        /// STEP: Generate common header and unique source

        return 0;
    }
}
