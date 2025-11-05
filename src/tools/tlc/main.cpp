#include <cstdio>
#include <cstring>
#include <vector>
#include <algorithm>

#include <stdcorelib/system.h>

namespace TLC::commands::thunkGen {
    int main(int argc, char *argv[]);
}

namespace TLC::commands::help {
    int main(int argc, char *argv[]);
}

namespace TLC::Internal {

    struct Command {
        const char *name;
        int (*main)(int argc, char **argv);
        const char *help;
    };

    static const Command commands[] = {
        {"thunk-gen", TLC::commands::thunkGen::main, "Generate thunk definitions"},
        {"help",      TLC::commands::help::main,     "Show this help message"    },
    };

    static const std::string &app_name() {
        static const std::string name = stdc::system::application_name();
        return name;
    }

}

namespace TLC::commands::help {

    int main(int argc, char *argv[]) {
        std::ignore = argc;
        std::ignore = argv;

        printf("Usage: %s <command> [args...]\n", Internal::app_name().c_str());
        printf("Available commands:\n");
        for (const auto command : TLC::Internal::commands) {
            printf("    %-15s    %s\n", command.name, command.help);
        }
        return 0;
    }

}

int main(int argc, char *argv[]) {
    return TLC::commands::thunkGen::main(argc, argv);

    // The sub-commands are not used now
    if (argc < 2) {
        return TLC::commands::help::main(argc, argv);
    }

    for (const auto command : TLC::Internal::commands) {
        if (std::strcmp(argv[1], command.name) == 0) {
            std::string commandName = TLC::Internal::app_name() + " " + command.name;
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