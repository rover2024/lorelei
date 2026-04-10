// SPDX-License-Identifier: MIT

#include <string>
#include <sstream>

#include <llvm/Support/Program.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>

using namespace clang;
using namespace clang::tooling;

namespace cl = llvm::cl;

namespace lore::tool::command::stat {

    const char *name = "stat";

    const char *help = "Probe statistics of input files";

    int main(int argc, char *argv[]) {
        return 0;
    }

}
