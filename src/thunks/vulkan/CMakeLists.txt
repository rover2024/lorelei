project(vulkan)

# Generate Vulkan API macro
set(_vulkan_api_header_dir ${LORE_THUNK_SOURCE_DIR}/vulkan)
set(_vulkan_api_header_file ${_vulkan_api_header_dir}/VulkanApis.h)
set(_script_file ${CMAKE_CURRENT_SOURCE_DIR}/VulkanApiGen.cmake)
set(_input_file ${CMAKE_CURRENT_SOURCE_DIR}/Symbols_autogen.conf)
execute_process(
    COMMAND "${CMAKE_COMMAND}" "-DINPUT=${_input_file}" "-DOUTPUT=${_vulkan_api_header_file}" -P "${_script_file}"
)

include("../AddThunk.cmake")

set(ALL_extra_includes ${LORE_VULKAN_INCLUDE_DIR} ${_vulkan_api_header_dir})
set(GTL_alias libvulkan.so.1)
set(HTL_extra_links X11_HMW xcb_HMW)

add_thunk()
