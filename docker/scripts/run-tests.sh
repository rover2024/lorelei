#!/usr/bin/env bash
#
# One-click test. Runs, under the patched QEMU with the dlcall plugin:
#   1. the lorelei auto tests (ctest);
#   2. the in-tree ThunkExample end-to-end test (src/tests/manual/TLC);
#   3. the distro's minizip binary over the zlib thunk from lorelei-thunks, timed three ways;
#   4. the distro's xz binary over the lzma thunk from lorelei-thunks, timed three ways.
# Everything is pre-built in the image, so this only generates the input and runs.
set -euo pipefail
: "${LORELEI_SRC:?}"
: "${INSTALL_DIR:?}"
: "${QEMU_BUILD_DIR:?}"

QEMU="$QEMU_BUILD_DIR/qemu-x86_64"
PLUGIN="$QEMU_BUILD_DIR/contrib/plugins/libdlcall.so"
work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT

echo "== 1/4  lorelei auto tests (ctest) =="
ctest --test-dir "$LORELEI_SRC/build" --output-on-failure

echo
echo "== 2/4  ThunkExample end-to-end (in-tree manual test) =="
# run_manual_tlc is x86_64-only (the in-tree test needs the GTL and HTL in one build tree, which a
# cross host cannot produce); on a cross host the minizip run below is the real end-to-end test.
if [ "$(uname -m)" = "x86_64" ]; then
    cmake --build "$LORELEI_SRC/build" --target run_manual_tlc
else
    echo "  skipped (x86_64-only)"
fi

echo
echo "== 3/4  minizip over the zlib thunk: native vs emulated vs lorelei =="
# The distro's unmodified x86_64 minizip binary calls zlib. Run it three ways: natively on the host,
# emulated under QEMU, and emulated under QEMU with the dlcall plugin so its zlib calls cross to the
# host's libz via the thunk. minizip itself stays in the guest; only zlib runs on the host, so the
# lorelei run should land near the native time and well under the fully-emulated one.
# native uses the host's own minizip; the emulated and lorelei runs use an x86_64 minizip under QEMU
# (the native one on an x86_64 host, the minizip-x86_64 the bootstrap unpacked on a cross host).
native_minizip="$(command -v minizip)"
guest_minizip="$(command -v minizip-x86_64 || command -v minizip)"
archive="$work/archive.bin"
python3 "$LORELEI_SRC/docker/benchmark/GenerateArchive.py" "$archive" "${ARCHIVE_SIZE:-64M}"

# Run a command, hide its output, and print the wall-clock time on its own line.
timed() {
    local label="$1"; shift
    local start end
    start=$(date +%s.%N)
    "$@" >/dev/null
    end=$(date +%s.%N)
    awk -v l="$label" -v s="$start" -v e="$end" 'BEGIN { printf "  %-28s %6.2fs\n", l, e - s }'
}

timed "native (host minizip)" \
    "$native_minizip" -8 -o "$work/native.zip" "$archive"

timed "emulated (qemu, no plugin)" \
    "$QEMU" "$guest_minizip" -8 -o "$work/emulated.zip" "$archive"

# The host runtime (LoreHostRT / LoreDLCall, this machine's arch) and the host thunk (HTL) live under
# install; the x86_64 guest runtime and the GTL live under install/x86_64. LORELEI_ROOT points at the
# host install so HTL_DIR and the ThunkDB (which lists libz) resolve there, and LORELEI_GUEST_ROOT
# points at the x86_64 install for GTL_DIR. The QEMU process loads the host runtime via
# LD_LIBRARY_PATH, and the guest minizip loads the GTL plus the x86_64 runtime from the path passed
# through with -E.
host_lib="$INSTALL_DIR/lib"
guest_lib="$INSTALL_DIR/x86_64/lib"
gtl_dir="$INSTALL_DIR/x86_64/lib/x86_64-LoreGTL"
timed "lorelei (qemu + zlib thunk)" \
    env LORELEI_ROOT="$INSTALL_DIR" LORELEI_GUEST_ROOT="$INSTALL_DIR/x86_64" \
        LD_LIBRARY_PATH="$host_lib" \
        "$QEMU" -plugin "$PLUGIN" \
        -E LD_LIBRARY_PATH="$gtl_dir:$guest_lib" \
        "$guest_minizip" -8 -o "$work/lorelei.zip" "$archive"

echo
echo "== 4/4  xz over the lzma thunk: native vs emulated vs lorelei =="
# Same idea as minizip, with the distro's unmodified xz binary over the lzma thunk. liblzma is
# buffer-based (lzma_code works over next_in/next_out, no FILE* crosses the boundary), so the thunk
# applies cleanly. A separate, modest input keeps this quick, since xz is much slower than gzip.
guest_xz="$(command -v xz-x86_64 || command -v xz)"
xz_input="$work/xz_input.bin"
python3 "$LORELEI_SRC/docker/benchmark/GenerateArchive.py" "$xz_input" "${XZ_ARCHIVE_SIZE:-64M}"

timed "native (host xz)" \
    "$(command -v xz)" -6 -c "$xz_input"

timed "emulated (qemu, no plugin)" \
    "$QEMU" "$guest_xz" -6 -c "$xz_input"

timed "lorelei (qemu + lzma thunk)" \
    env LORELEI_ROOT="$INSTALL_DIR" LORELEI_GUEST_ROOT="$INSTALL_DIR/x86_64" \
        LD_LIBRARY_PATH="$host_lib" \
        "$QEMU" -plugin "$PLUGIN" \
        -E LD_LIBRARY_PATH="$gtl_dir:$guest_lib" \
        "$guest_xz" -6 -c "$xz_input"

# Correctness: the lorelei-compressed stream decompresses back to the original. Use file output
# (-k -f) rather than stdout so the qemu process's own output cannot mix into the compressed bytes.
cp "$xz_input" "$work/xz_check.bin"
env LORELEI_ROOT="$INSTALL_DIR" LORELEI_GUEST_ROOT="$INSTALL_DIR/x86_64" \
    LD_LIBRARY_PATH="$host_lib" \
    "$QEMU" -plugin "$PLUGIN" \
    -E LD_LIBRARY_PATH="$gtl_dir:$guest_lib" \
    "$guest_xz" -6 -k -f "$work/xz_check.bin"
xz -dc "$work/xz_check.bin.xz" | cmp - "$xz_input"
echo "  xz output verified (decompresses to the original)"

echo
echo "All tests passed."
