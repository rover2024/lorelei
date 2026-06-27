# Launches the ThunkExample guest program under the patched QEMU. Run via `cmake -P` by the
# run_manual_tlc target so QEMU_BUILD_DIR can come from the environment at build time. The rest is
# passed in as -D defines: PROGRAM (the guest program), WORK (its build directory, holding the GTL,
# HTL and ThunkDB.json), RT_DIR (where the Lore runtimes live) and QEMU_BUILD_DIR (the cache value,
# used when the environment does not set one).

if(DEFINED ENV{QEMU_BUILD_DIR})
    set(_qemu "$ENV{QEMU_BUILD_DIR}")
elseif(QEMU_BUILD_DIR)
    set(_qemu "${QEMU_BUILD_DIR}")
else()
    message(FATAL_ERROR
        "QEMU_BUILD_DIR is not set.\n"
        "Give it through the environment or -D, pointing at your QEMU build/release directory:\n"
        "  QEMU_BUILD_DIR=/path/to/qemu/build/release cmake --build <build> --target run_manual_tlc")
endif()

set(_qemu_bin "${_qemu}/qemu-x86_64")
set(_plugin "${_qemu}/contrib/plugins/libdlcall.so")

if(NOT EXISTS "${_qemu_bin}" OR NOT EXISTS "${_plugin}")
    message(FATAL_ERROR
        "QEMU not found under '${_qemu}'.\n"
        "  expected: ${_qemu_bin}\n"
        "            ${_plugin}\n"
        "Set QEMU_BUILD_DIR (environment or -D) to your QEMU build/release directory.")
endif()

# The host process (QEMU itself) loads LoreHostRT, LoreDLCall and the HTL from its own
# LD_LIBRARY_PATH; the guest side loads the GTL, LoreGuestRT and LoreDLCall from the path passed
# through with -E. The config variables point the shorthand database entry at the work directory
# (the runtime otherwise expects the lib/<arch>-Lore{G,H}TL install layout).
execute_process(
    COMMAND ${CMAKE_COMMAND} -E env
        LD_LIBRARY_PATH=${RT_DIR}:${WORK}
        LORELEI_THUNK_DATABASE=${WORK}/ThunkDB.json
        "LORELEI_THUNKS_CONFIG_VARIABLES=GTL_DIR=${WORK};HTL_DIR=${WORK}"
        ${_qemu_bin}
            -plugin ${_plugin}
            -E LD_LIBRARY_PATH=${WORK}:${RT_DIR}
            ${PROGRAM}
    RESULT_VARIABLE _rc
)

if(NOT _rc EQUAL 0)
    message(FATAL_ERROR "ThunkExample guest test failed (exit ${_rc})")
endif()
