#!/usr/bin/env bash
#
# End-to-end ThunkExample run, built through LoreMakeThunk instead of the in-tree CMake targets.
#
# The CMake path (run_manual_tlc) needs the GTL and the HTL in one build tree, so it is x86_64-only.
# LoreMakeThunk builds each side with the compiler its config names, so this works on any host: the
# HTL is native, the GTL and the guest program are x86_64, and qemu runs the program over the pack.
#
# Everything the build needs is named by a MakeThunkConfig.json, so this script assumes no layout.
#
#   CONFIG     a MakeThunkConfig.json (else DEVKIT's)
#   DEVKIT     an unpacked devkit, i.e. use its share/lorelei/MakeThunkConfig.json
#   GUEST_CC   x86_64 C compiler for the guest program (default: the devkit's clang wrapper)
#   GUEST_RT   dir holding the x86_64 LoreGuestRT   (default: <devkit>/x86_64/lib)
#   HOST_RT    dir holding the native LoreHostRT    (default: <devkit>/lib)
#   SYSROOT    guest sysroot for qemu -L            (default: <devkit>/x86_64/sysroot if present)
#   QEMU       qemu-x86_64 carrying the dlcall plugin
#   PLUGIN     libdlcall.so
#   OUT        build directory (default: a temporary one, removed on exit)
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FIXTURE="$HERE/../../auto/TLC/TestData"
MAKETHUNK="${MAKETHUNK:-$HERE/../../../../scripts/LoreMakeThunk.py}"

: "${QEMU:?set QEMU to a qemu-x86_64 with the dlcall plugin}"
: "${PLUGIN:?set PLUGIN to libdlcall.so}"

DEVKIT="${DEVKIT:-${LORELEI_DEVKIT:-}}"
CONFIG="${CONFIG:-}"
if [ -z "$CONFIG" ]; then
    [ -n "$DEVKIT" ] || { echo "set CONFIG or DEVKIT" >&2; exit 2; }
    CONFIG="$DEVKIT/share/lorelei/MakeThunkConfig.json"
fi
GUEST_CC="${GUEST_CC:-${DEVKIT:+$DEVKIT/bin/x86_64-linux-gnu-clang}}"
GUEST_RT="${GUEST_RT:-${DEVKIT:+$DEVKIT/x86_64/lib}}"
HOST_RT="${HOST_RT:-${DEVKIT:+$DEVKIT/lib}}"
SYSROOT="${SYSROOT:-${DEVKIT:+$DEVKIT/x86_64/sysroot}}"
: "${GUEST_CC:?set GUEST_CC (or DEVKIT)}"
: "${GUEST_RT:?set GUEST_RT (or DEVKIT)}"
: "${HOST_RT:?set HOST_RT (or DEVKIT)}"

if [ -n "${OUT:-}" ]; then
    mkdir -p "$OUT"
else
    OUT="$(mktemp -d)"
    trap 'rm -rf "$OUT"' EXIT
fi

# 1. The host library the host thunk dispatches to: the example implementation, built native. It is
# also what LoreMakeThunk dumps the SONAME from and links the HTL against.
mkdir -p "$OUT/host"
"${HOST_CXX:-c++}" -shared -fPIC -std=gnu++20 -I"$FIXTURE" \
    "$FIXTURE/ThunkExample.cpp" -o "$OUT/host/libThunkExample.so"

# 2. The thunk pack. The fixture already carries the four intermediates, and its manifests are not
# mechanical (Manifest_host.cpp adds the long double type filters), so pass all four through rather
# than letting LoreMakeThunk generate them.
python3 "$MAKETHUNK" --config "$CONFIG" --name ThunkExample \
    --lib "$OUT/host/libThunkExample.so" \
    --desc "$FIXTURE/Desc.h" \
    --symbols "$FIXTURE/Symbols.conf" \
    --manifest-host "$FIXTURE/Manifest_host.cpp" \
    --manifest-guest "$FIXTURE/Manifest_guest.cpp" \
    -o "$OUT/thunks" -- -I"$FIXTURE"

# 3. The guest program, an ordinary x86_64 binary linked against the generated guest thunk. The GTL
# has libLoreGuestRT.so as a NEEDED, so -rpath-link points the linker at it without recording a path
# in the program: at run time the guest LD_LIBRARY_PATH below supplies it.
"$GUEST_CC" "$HERE/Program.c" -I"$FIXTURE" \
    -L"$OUT/thunks/x86_64" -lThunkExample \
    -L"$GUEST_RT" -Wl,-rpath-link,"$GUEST_RT" -o "$OUT/Program"

# 4. Run it. The guest finds the GTL and the guest runtime on the guest LD_LIBRARY_PATH; qemu itself
# finds the host runtime and the host library on its own.
echo "== running ThunkExample under qemu =="
LD_LIBRARY_PATH="$HOST_RT:$OUT/host" \
    "$QEMU" ${SYSROOT:+-L "$SYSROOT"} -plugin "$PLUGIN" \
    -E LD_LIBRARY_PATH="$GUEST_RT:$OUT/thunks/x86_64" \
    "$OUT/Program"
