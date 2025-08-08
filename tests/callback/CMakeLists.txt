project(tst_callback)

file(GLOB_RECURSE _src *.h *.c *.hpp *.cpp)
add_executable(${PROJECT_NAME} ${_src})

target_link_libraries(${PROJECT_NAME} PRIVATE loreshared)
target_compile_options(${PROJECT_NAME} PRIVATE -fPIC -ffixed-${LORELEI_CALLBACK_REGISTER})