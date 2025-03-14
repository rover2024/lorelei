function(lorelei_add_simple_library _target)
    file(GLOB_RECURSE _src *.h *.hpp *.c *.cpp)
    add_library(${PROJECT_NAME} STATIC ${_src})
    target_include_directories(${_target} PUBLIC 
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/..>"
    )
endfunction()

add_subdirectory(json11)

add_subdirectory(ff_scanf)

add_subdirectory(mpaland_printf)