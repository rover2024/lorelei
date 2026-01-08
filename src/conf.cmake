# ----------------------------------
# Language Configurations
# ----------------------------------
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)
set(CMAKE_C_VISIBILITY_PRESET "hidden")
set(CMAKE_CXX_VISIBILITY_PRESET "hidden")

if(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} /EHsc-")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc- /GR-")
else()
    set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-rtti")

    if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-z,defs")
    endif()
endif()

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# ----------------------------------
# Project Constants
# ----------------------------------
set(LORE_INCLUDE_DIR "../include")

function(_lore_common_configure_target _target)
    qm_configure_target(${_target}
        FEATURES cxx_std_17
    )
    lore_set_default_install_rpath(${_target})
endfunction()

set(LORE_PRE_CONFIGURE_COMMANDS _lore_common_configure_target)

# ----------------------------------
# Include Build Helpers
# ----------------------------------
set(QM_BUILD_REPO_HELPERS_FUNCTION_PREFIX lore)
include(${LORE_SOURCE_DIR}/cmake/QMBuildRepoHelpers.cmake) # generic apis

include(${LORE_SOURCE_DIR}/cmake/LoreBuildApi.cmake) # specific apis