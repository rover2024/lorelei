function(lore_add_simple_library _target)
    file(GLOB_RECURSE _src *.h *.hpp *.c *.cpp)
    add_library(${_target} STATIC ${_src})
    target_include_directories(${_target} PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/..>"
    )
    target_compile_options(${_target} PRIVATE -fPIC -fvisibility=hidden)
endfunction()

add_subdirectory(json11)

add_subdirectory(mpaland_printf)