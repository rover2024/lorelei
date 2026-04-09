#define TOOL_MAIN_COMMAND_FOREACH(F)                                                               \
    F(batch)                                                                                       \
    F(generate)                                                                                    \
    F(stat)                                                                                        \
    F(help)

#define TOOL_MAIN_VERSION     TOOL_VERSION
#define TOOL_MAIN_DESCRIPTION "Lorelei Thunk Library Compiler (Lorelei " TOOL_MAIN_VERSION ")"

#include "ToolMain/main.cpp.inc"
