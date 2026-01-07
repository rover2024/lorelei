#define TOOL_MAIN_COMMAND_FOREACH(F)                                                               \
    F(batch)                                                                                       \
    F(mark_macros)                                                                                 \
    F(preprocess)                                                                                  \
    F(rewrite)                                                                                     \
    F(stat)                                                                                        \
    F(help)

#define TOOL_MAIN_VERSION     TOOL_VERSION
#define TOOL_MAIN_DESCRIPTION "Lorelei Host Library Rewriter (Lorelei " TOOL_MAIN_VERSION ")"

#include "../ToolMain/main.cpp.inc"
