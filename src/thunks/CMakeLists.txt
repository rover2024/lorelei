# Macros
function(lorelib_get_subdirs _dir _out)
    file(GLOB _subdirs RELATIVE ${_dir} ${_dir}/*)

    set(_result)

    foreach(_subdir IN LISTS _subdirs)
        if(IS_DIRECTORY ${_dir}/${_subdir})
            list(APPEND _result ${_subdir})
        endif()
    endforeach()

    if(_result)
        set(${_out} ${_result} PARENT_SCOPE)
    endif()
endfunction()

function(lorelib_add_sub_project _subdir)
    project(${_subdir})

    set(LORELEI_GUEST_PROJECT ${PROJECT_NAME})
    set(LORELEI_HOST_PROJECT ${PROJECT_NAME}_HTL)

    # Guest
    if(LORELEI_ENABLE_GUEST)
        add_library(${LORELEI_GUEST_PROJECT} SHARED _template/lorelib_guest.c)
        target_link_options(${LORELEI_GUEST_PROJECT} PRIVATE "-Wl,-z,defs")
        target_link_libraries(${LORELEI_GUEST_PROJECT} PRIVATE loregrt dl)
        target_include_directories(${LORELEI_GUEST_PROJECT} PRIVATE
            ${_subdir}
            ${LORELEI_SOURCE_DIR}/include
        )
        set_target_properties(${LORELEI_GUEST_PROJECT} PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${LORELEI_GUEST_ARCH}-lore-guest
        )
    endif()

    # Host
    if(LORELEI_ENABLE_HOST)
        add_library(${LORELEI_HOST_PROJECT} SHARED _template/lorelib_host.c)
        target_link_options(${LORELEI_HOST_PROJECT} PRIVATE "-Wl,-z,defs")
        target_link_libraries(${LORELEI_HOST_PROJECT} PRIVATE lorehrt dl)
        target_include_directories(${LORELEI_HOST_PROJECT} PRIVATE
            ${_subdir}
            ${LORELEI_SOURCE_DIR}/include
        )
        set_target_properties(${LORELEI_HOST_PROJECT} PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${LORELEI_HOST_ARCH}-lore-host
        )
    endif()

    set(LORELIB_GUEST_SYMLINKS)
    set(LORELIB_HOST_SYMLINKS)
    set(LORELIB_TLC_OPTIONS)
    set(LORELIB_TLC_IGNORE FALSE)
    set(LORELIB_SKIP_INSTALL FALSE)

    # Add options
    if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/${_subdir}/options.cmake")
        include("${_subdir}/options.cmake")
    endif()

    if(LORELEI_ENABLE_GUEST)
        if(LORELEI_INSTALL AND NOT LORELIB_SKIP_INSTALL)
            install(TARGETS ${LORELEI_GUEST_PROJECT}
                LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/${LORELEI_GUEST_ARCH}-lore-guest" OPTIONAL
            )
        endif()

        if(LORELIB_GUEST_SYMLINKS)
            foreach(_item IN LISTS LORELIB_GUEST_SYMLINKS)
                add_custom_command(TARGET ${LORELEI_GUEST_PROJECT} POST_BUILD
                    COMMAND ln -sf $<TARGET_FILE_NAME:${LORELEI_GUEST_PROJECT}> ${_item}
                    WORKING_DIRECTORY $<TARGET_FILE_DIR:${LORELEI_GUEST_PROJECT}>
                )

                if(LORELEI_INSTALL AND NOT LORELIB_SKIP_INSTALL)
                    install(CODE "
                        execute_process(
                            COMMAND ln -sf $<TARGET_FILE_NAME:${LORELEI_GUEST_PROJECT}> ${_item}
                            WORKING_DIRECTORY \${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/${LORELEI_GUEST_ARCH}-lore-guest
                        )
                    ")
                endif()
            endforeach()
        endif()
    endif()

    if(LORELEI_ENABLE_HOST)
        if(LORELEI_INSTALL AND NOT LORELIB_SKIP_INSTALL)
            install(TARGETS ${LORELEI_HOST_PROJECT}
                LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/${LORELEI_HOST_ARCH}-lore-host" OPTIONAL
            )
        endif()

        if(LORELIB_HOST_SYMLINKS)
            foreach(_item IN LISTS LORELIB_HOST_SYMLINKS)
                add_custom_command(TARGET ${LORELEI_HOST_PROJECT} POST_BUILD
                    COMMAND ln -sf $<TARGET_FILE_NAME:${LORELEI_HOST_PROJECT}> ${_item}
                    WORKING_DIRECTORY $<TARGET_FILE_DIR:${LORELEI_HOST_PROJECT}>
                )

                if(LORELEI_INSTALL AND NOT LORELIB_SKIP_INSTALL)
                    install(CODE "
                        execute_process(
                            COMMAND ln -sf $<TARGET_FILE_NAME:${LORELEI_HOST_PROJECT}> ${_item}
                            WORKING_DIRECTORY \${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/${LORELEI_HOST_ARCH}-lore-host
                        )
                    ")
                endif()
            endforeach()
        endif()
    endif()

    if(NOT LORELIB_TLC_IGNORE)
        set(_common_tlc_options
            -I.
            -I${LORELEI_SOURCE_DIR}/include
        )

        if(LORELEI_BUILD_TOOLS)
            add_custom_target(thunks_${_subdir})
            add_dependencies(thunks_all thunks_${_subdir})
            add_custom_command(TARGET thunks_${_subdir} POST_BUILD
                COMMAND $<TARGET_FILE:loretlc> -c symbols.txt lorelib_manifest.c -- ${_common_tlc_options} ${LORELIB_TLC_OPTIONS}
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${_subdir}
            )
        endif()
    endif()
endfunction()

if(LORELEI_BUILD_TOOLS)
    # Add target to generate thunks manually
    add_custom_target(thunks_all)
endif()

# Add subdirectories
lorelib_get_subdirs(${CMAKE_CURRENT_SOURCE_DIR} _subdirs)

foreach(_subdir IN LISTS _subdirs)
    if(NOT _subdir MATCHES "^_.+")
        lorelib_add_sub_project(${_subdir})
    endif()
endforeach()