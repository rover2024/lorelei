project(LoreDLCall)

enable_language(ASM)

# Pick up the C++/headers everywhere, plus only this target arch's trampoline shim assembly (the
# other arches' .S cannot assemble here), mirroring TinyCoroutine.
file(GLOB_RECURSE _src *.h *.cpp Tools/Arch/${LORE_HOST_ARCH}/*.S)

# Use a tiny JSON library instead of linking against the huge LLVM JSON library.
# Shared so host/guest thunks resolve VariadicAdaptor (and friends) against this one copy at
# runtime instead of each statically pulling LoreDLCall.a, which dragged in the printf parser's
# mp_vsnprintf reference without its definition. Public classes are tagged LOREDLCALL_EXPORT (see
# Global.h). The bundled json11/printf/avcall keep their hidden visibility, so mp_vsnprintf and the
# JSON parser stay internal to this library.
lore_add_library(${PROJECT_NAME} SHARED
    SOURCES ${_src}
    LINKS LoreSupport
    LINKS_PRIVATE $<BUILD_INTERFACE:json11> $<BUILD_INTERFACE:printf>
)
qm_include_recursive(${PROJECT_NAME} PRIVATE
    ${LORE_INCLUDE_DIR}/lorelei/DLCall
)

# Link avcall statically (libavcall.a) so it is baked into this shared library under its hidden
# visibility, rather than leaving a runtime dependency on libavcall.so.
set(_saved_lib_suffixes ${CMAKE_FIND_LIBRARY_SUFFIXES})
set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
find_library(_avcall_lib avcall)
set(CMAKE_FIND_LIBRARY_SUFFIXES ${_saved_lib_suffixes})

if(NOT _avcall_lib)
    message(FATAL_ERROR "static avcall library (libavcall.a) not found")
endif()

if(NOT WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${_avcall_lib})
else()
    if(FFCALL_INCLUDE_DIR)
        target_include_directories(${PROJECT_NAME} PRIVATE ${FFCALL_INCLUDE_DIR})
    endif()

    if(FFCALL_LIBRARY_DIR)
        target_link_directories(${PROJECT_NAME} PRIVATE ${FFCALL_LIBRARY_DIR})
        target_link_libraries(${PROJECT_NAME} PRIVATE avcall.lib)
    endif()
endif()
