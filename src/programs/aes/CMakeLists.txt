project(prog_aes)

file(GLOB_RECURSE _src *.h *.c)
add_executable(${PROJECT_NAME} ${_src})

target_link_libraries(${PROJECT_NAME} PRIVATE ssl crypto)

lorelei_install_program(${PROJECT_NAME})