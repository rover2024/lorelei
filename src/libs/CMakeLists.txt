add_subdirectory(DLCall)

add_subdirectory(Support)

if(LORE_BUILD_TOOLS)
    add_subdirectory(ClangExtras)
    add_subdirectory(TLCApi)
endif()