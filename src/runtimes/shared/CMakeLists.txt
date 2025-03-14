file(GLOB_RECURSE _src *.h *.hpp *.c *.cpp)
target_sources(${PROJECT_NAME} PRIVATE ${_src})

target_link_libraries(${PROJECT_NAME} PRIVATE ff_scanf mpaland_printf)