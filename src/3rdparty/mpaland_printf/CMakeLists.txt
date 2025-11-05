project(mpaland_printf)

lore_add_simple_library(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${LORELEI_SOURCE_DIR}/include>
)