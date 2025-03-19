project(tst_vacall)

file(GLOB_RECURSE _src *.h *.c *.hpp *.cpp)
add_executable(${PROJECT_NAME} ${_src})

target_link_libraries(${PROJECT_NAME} PRIVATE loreshared)