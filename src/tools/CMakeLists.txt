function(lorelei_link_clang _target)
    find_package(Clang REQUIRED)
    target_include_directories(${_target} SYSTEM PUBLIC ${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS})
    target_link_directories(${_target} PUBLIC ${LLVM_LIBRARY_DIRS})
    target_compile_definitions(${_target} PUBLIC ${LLVM_DEFINITIONS})
    target_link_libraries(${_target} PRIVATE
        clangAST
        clangBasic
        clangFrontend
        clangSerialization
        clangTooling
    )
endfunction()

add_subdirectory(ppf)

add_subdirectory(cfic)

add_subdirectory(tlc)