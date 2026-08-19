#!/usr/bin/env bash
#
# One-click test. Runs:
#   1. the lorelei auto tests (ctest);
#   2. the ThunkExample end-to-end test (src/tests/manual/TLC), built through LoreMakeThunk and run
#      under the patched QEMU with the dlcall plugin.
set -euo pipefail
: "${LORELEI_SRC:?}"
: "${INSTALL_DIR:?}"
: "${QEMU_BUILD_DIR:?}"

echo "== 1/2  lorelei auto tests (ctest) =="
ctest --test-dir "$LORELEI_SRC/build" --output-on-failure

echo
echo "== 2/2  ThunkExample end-to-end (via LoreMakeThunk) =="
# Unlike the CMake target (run_manual_tlc), which needs the GTL and the HTL in one build tree and so
# is x86_64-only, this builds each side with the compiler the config names, so it runs on every host
# arch. The guest program needs the x86_64 loader and libc, which the bootstrap installs as amd64
# multiarch, so no -L sysroot is passed.
case "$(uname -m)" in
    x86_64|amd64) guest_cc=gcc ;;
    riscv64)      guest_cc=x86_64-unknown-linux-gnu-gcc ;;
    *)            guest_cc=x86_64-linux-gnu-gcc ;;
esac
CONFIG="$INSTALL_DIR/share/lorelei/MakeThunkConfig.json" \
GUEST_CC="$guest_cc" \
GUEST_RT="$INSTALL_DIR/x86_64/lib" \
HOST_RT="$INSTALL_DIR/lib" \
QEMU="$QEMU_BUILD_DIR/qemu-x86_64" \
PLUGIN="$QEMU_BUILD_DIR/contrib/plugins/libdlcall.so" \
    bash "$LORELEI_SRC/src/tests/manual/TLC/RunMakeThunk.sh"

echo
echo "All tests passed."
