project(loretlc2)

file(GLOB_RECURSE _src *.h *.cpp)
add_executable(${PROJECT_NAME} ${_src})

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

target_include_directories(${PROJECT_NAME} PRIVATE .
    ast
    base
    parse
    passes
    passes/builder
    passes/decoration
    passes/entryexit
)

lorelei_link_clang(${PROJECT_NAME})

lorelei_install_tool(${PROJECT_NAME})