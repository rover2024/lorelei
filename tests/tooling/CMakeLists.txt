project(tst_tooling)

file(GLOB_RECURSE _src *.h *.c *.hpp *.cpp)
add_executable(${PROJECT_NAME} ${_src})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

find_package(Clang REQUIRED)
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PUBLIC ${LLVM_LIBRARY_DIRS})
target_compile_definitions(${PROJECT_NAME} PUBLIC ${LLVM_DEFINITIONS})
target_link_libraries(${PROJECT_NAME} PRIVATE
    clangAST
    clangBasic
    clangFrontend
    clangSerialization
    clangTooling
)