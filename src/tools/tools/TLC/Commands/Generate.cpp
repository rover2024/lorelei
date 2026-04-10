// SPDX-License-Identifier: MIT

#include <string>
#include <filesystem>

#include <llvm/Support/Program.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/VirtualFileSystem.h>
#include <llvm/Support/CommandLine.h>
#include <clang/Tooling/CommonOptionsParser.h>

#include <lorelei/Base/Support/ScopeGuard.h>
#include <lorelei/Base/Support/StringExtras.h>

using namespace clang;
using namespace clang::tooling;

namespace cl = llvm::cl;

namespace lore::tool::command::generate {

    const char *name = "generate";

    const char *help = "Generate thunk library sources for input files";

    int main(int argc, char *argv[]) {
        return 0;
    }

}
