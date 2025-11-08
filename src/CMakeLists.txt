# Disable exceptions
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -fvisibility=hidden")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-rtti -fvisibility=hidden")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-z,defs")

include("BuildAPI.cmake")

add_subdirectory(3rdparty)

add_subdirectory(libs)

if(LORELEI_BUILD_TOOLS)
    add_subdirectory(tools)
endif()

if(LORELEI_BUILD_THUNKS)
    add_subdirectory(thunks2)
endif()

if(LORELEI_INSTALL)
    qm_basic_install(
        NAME ${LORELEI_INSTALL_NAME}
        VERSION ${PROJECT_VERSION}
        INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/${LORELEI_INSTALL_NAME}
        CONFIG_TEMPLATE "${LORELEI_INSTALL_NAME}Config.cmake.in"
        NAMESPACE ${LORELEI_INSTALL_NAME}::
        EXPORT ${LORELEI_INSTALL_NAME}Targets
        WRITE_CONFIG_OPTIONS NO_CHECK_REQUIRED_COMPONENTS_MACRO
    )

    # Install headers
    install(DIRECTORY ${LORELEI_SOURCE_DIR}/include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.h"
    )

    # Install shared files
    include("../share/install.cmake")
endif()