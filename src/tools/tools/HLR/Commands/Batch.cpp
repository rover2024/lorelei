// SPDX-License-Identifier: MIT

#include <string>

#include <llvm/Support/Program.h>

extern "C" {
extern unsigned char res_FileContext_h_c[];
extern unsigned int res_FileContext_h_c_len;
};

namespace lore::tool::command::batch {

    const char *name = "batch";

    const char *help = "Run batch on input files";

    int main(int argc, char *argv[]) {
        return 0;
    }

}
