#!/usr/bin/env bash
#
# One-click test. Runs:
#   1. the lorelei auto tests (ctest);
#   2. the in-tree ThunkExample end-to-end test (src/tests/manual/TLC), under the patched QEMU with
#      the dlcall plugin.
# Everything is pre-built in the image, so this only runs.
set -euo pipefail
: "${LORELEI_SRC:?}"
: "${QEMU_BUILD_DIR:?}"

echo "== 1/2  lorelei auto tests (ctest) =="
ctest --test-dir "$LORELEI_SRC/build" --output-on-failure

echo
echo "== 2/2  ThunkExample end-to-end (in-tree manual test) =="
# run_manual_tlc is x86_64-only: the in-tree test needs the GTL and HTL in one build tree, which a
# cross host cannot produce. Run.cmake picks QEMU_BUILD_DIR up from the environment.
if [ "$(uname -m)" = "x86_64" ]; then
    cmake --build "$LORELEI_SRC/build" --target run_manual_tlc
else
    echo "  skipped (x86_64-only)"
fi

echo
echo "All tests passed."
