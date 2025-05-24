project(loretlc)

file(GLOB_RECURSE _src *.h *.cpp)
add_executable(${PROJECT_NAME} ${_src})

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

target_include_directories(${PROJECT_NAME} PRIVATE . core passes)

lorelei_link_clang(${PROJECT_NAME})

lorelei_install_tool(${PROJECT_NAME})