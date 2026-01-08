// SPDX-License-Identifier: MIT

#include <cstdio>
#include <cstring>
#include <vector>
#include <algorithm>
#include <string>

#include <llvm/Support/Path.h>
#include <llvm/Support/FileSystem.h>

// Required definitions:
//     TOOL_MAIN_COMMAND_FOREACH(F)
//     TOOL_MAIN_DESCRIPTION
//     TOOL_MAIN_VERSION

namespace lore::tool::command {

#define _F(COMMAND)                                                                                \
  namespace COMMAND {                                                                              \
    extern const char *name;                                                                       \
    extern const char *help;                                                                       \
    extern int main(int argc, char *argv[]);                                                       \
  }
    TOOL_MAIN_COMMAND_FOREACH(_F)
#undef _F

    struct Command {
        const char *name;
        int (*main)(int argc, char **argv);
        const char *help;
    };

    static const Command commands[] = {
#define _F(COMMAND) {COMMAND::name, COMMAND::main, COMMAND::help},
        TOOL_MAIN_COMMAND_FOREACH(_F)
#undef _F
    };

    static std::string app_name(const char *argv0) {
        using llvm::sys::fs::getMainExecutable;
        using llvm::sys::path::stem;
        return stem(getMainExecutable(argv0, (void *) app_name)).str();
    }

}

namespace lore::tool::command::help {

    const char *name = "help";

    const char *help = "Show this help message";

    int main(int argc, char *argv[]) {
        std::ignore = argc;
        std::ignore = argv;

        printf("Usage: %s <command> [args...]\n", app_name(argv[0]).c_str());
        printf("%s\n", TOOL_MAIN_DESCRIPTION);
        printf("\n");
        printf("Commands:\n");
        for (const auto command : commands) {
            printf("    %-15s    %s\n", command.name, command.help);
        }
        printf("\n");
        printf("Options:\n");
        printf("    %-15s    %s\n", "--version, -v", "Show version information");
        printf("    %-15s    %s\n", "--help, -h", "Show this help message");
        return 0;
    }

}

int main(int argc, char *argv[]) {
    if (argc < 2 || std::strcmp(argv[1], "--help") == 0 || std::strcmp(argv[1], "-h") == 0) {
        return lore::tool::command::help::main(argc, argv);
    }
    if (std::strcmp(argv[1], "--version") == 0 || std::strcmp(argv[1], "-v") == 0) {
        printf("%s\n", TOOL_MAIN_VERSION);
        return 0;
    }
    for (const auto command : lore::tool::command::commands) {
        if (std::strcmp(argv[1], command.name) == 0) {
            std::string commandName = lore::tool::command::app_name(argv[0]) + " " + command.name;
            std::vector<char *> args;
            args.resize(argc - 1);
            args[0] = commandName.data();
            std::copy(argv + 2, argv + argc, args.begin() + 1);
            return command.main(args.size(), args.data());
        }
    }
    printf("Unknown command: %s\n", argv[1]);
    return 1;
}
