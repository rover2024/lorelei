project(prog_aes)

file(GLOB_RECURSE _src *.h *.c)
add_executable(${PROJECT_NAME} ${_src})

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

target_link_libraries(${PROJECT_NAME} PRIVATE ssl crypto)

lorelei_install_program(${PROJECT_NAME})