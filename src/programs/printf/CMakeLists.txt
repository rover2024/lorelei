project(prog_printf)

file(GLOB_RECURSE _src *.h *.cpp)
add_executable(${PROJECT_NAME} ${_src})

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

target_link_libraries(${PROJECT_NAME} PRIVATE SDL2-2.0)

target_include_directories(${PROJECT_NAME} PRIVATE "/home/overworld/Documents/ccxxprojs/SDL/include")

lorelei_install_program(${PROJECT_NAME})