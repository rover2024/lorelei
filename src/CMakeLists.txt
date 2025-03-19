function(lorelei_add_simple_library _target)
    file(GLOB_RECURSE _src *.h *.hpp *.c *.cpp)
    add_library(${_target} STATIC ${_src})
    target_include_directories(${_target} PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/..>"
    )
    target_compile_options(${_target} PRIVATE -fPIC)
endfunction()

function(lorelei_install_tool _target)
    if(LORELEI_INSTALL)
        if(LORELEI_VCPKG_TOOLS_HINT)
            set(_tools_dir tools/${LORELEI_INSTALL_NAME})
        else()
            set(_tools_dir ${CMAKE_INSTALL_BINDIR})
        endif()

        install(TARGETS ${_target}
            EXPORT ${LORELEI_INSTALL_NAME}Targets
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" OPTIONAL
        )
    endif()
endfunction()

function(lorelei_install_library _target)
    if(LORELEI_INSTALL)
        target_include_directories(${_target} PUBLIC "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
        install(TARGETS ${_target}
            EXPORT ${LORELEI_INSTALL_NAME}Targets
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" OPTIONAL
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" OPTIONAL
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" OPTIONAL
        )
    endif()
endfunction()

# add_subdirectory(libs)
add_subdirectory(3rdparty)

add_subdirectory(runtimes)

if(LORELEI_BUILD_TOOLS)
    add_subdirectory(tools)
endif()

if(LORELEI_INSTALL)
    # Add install target
    set(_install_dir ${CMAKE_INSTALL_LIBDIR}/cmake/${LORELEI_INSTALL_NAME})

    # Add version file
    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/${LORELEI_INSTALL_NAME}ConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
    )

    # Add configuration file
    configure_package_config_file(
        ${CMAKE_CURRENT_LIST_DIR}/${LORELEI_INSTALL_NAME}Config.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/${LORELEI_INSTALL_NAME}Config.cmake"
        INSTALL_DESTINATION ${_install_dir}
        NO_CHECK_REQUIRED_COMPONENTS_MACRO
    )

    # Install cmake files
    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${LORELEI_INSTALL_NAME}Config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${LORELEI_INSTALL_NAME}ConfigVersion.cmake"
        DESTINATION ${_install_dir}
    )

    # Install cmake targets files
    install(EXPORT ${LORELEI_INSTALL_NAME}Targets
        FILE "${LORELEI_INSTALL_NAME}Targets.cmake"
        NAMESPACE LORELEI::
        DESTINATION ${_install_dir}
    )

    install(DIRECTORY ${LORELEI_SOURCE_DIR}/include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.h"
    )
endif()