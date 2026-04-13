if(NOT DEFINED INPUT)
    message(FATAL_ERROR "INPUT is required")
endif()

if(NOT DEFINED OUTPUT)
    message(FATAL_ERROR "OUTPUT is required")
endif()

if(NOT EXISTS "${INPUT}")
    message(FATAL_ERROR "Input file not found: ${INPUT}")
endif()

file(READ "${INPUT}" _content)
string(REPLACE "\r\n" "\n" _content "${_content}")
string(REPLACE "\r" "\n" _content "${_content}")

string(REGEX MATCHALL "PFN_vk[A-Za-z0-9_]+" _pfn_symbols "${_content}")

if(_pfn_symbols STREQUAL "")
    message(FATAL_ERROR "No PFN_vk* symbols found in: ${INPUT}")
endif()

set(_apis)

foreach(_sym IN LISTS _pfn_symbols)
    string(REGEX REPLACE "^PFN_" "" _api "${_sym}")
    list(FIND _apis "${_api}" _idx)

    if(_idx EQUAL -1)
        list(APPEND _apis "${_api}")
    endif()
endforeach()

list(LENGTH _apis _api_count)
math(EXPR _last_idx "${_api_count} - 1")

set(_out "")
string(APPEND _out "// Auto-generated from ${INPUT}. Do not edit manually.\n")
string(APPEND _out "#define LORE_THUNK_VULKAN_API_FOREACH(F) \\\n")

set(_i 0)

foreach(_api IN LISTS _apis)
    if(_i LESS _last_idx)
        string(APPEND _out "    F(${_api}) \\\n")
    else()
        string(APPEND _out "    F(${_api})\n")
    endif()

    math(EXPR _i "${_i} + 1")
endforeach()

file(WRITE "${OUTPUT}" "${_out}")
