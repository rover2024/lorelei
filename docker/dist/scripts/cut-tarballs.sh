#!/usr/bin/env bash
#
# Cut the three shipping tarballs from one built tree, each unpacking to a self-contained top-level
# directory named for itself (not the internal build-tree name):
#   lorelei-devkit-<arch>   the full toolchain + lorelei runtime + headers/sysroot/cmake, no thunks
#   lorelei-runtime-<arch>  the lorelei runtime .so's alone, no toolchain and no thunks
#   lorelei-thunks-<arch>   the prebuilt thunks (HTL + GTL + ThunkDB.json), a drop-in LORELEI_THUNK_PATH
#                           prefix mirroring the deployed layout (it depends on a runtime for the
#                           lorelei .so's the thunks link against)
#
# The base trees (devkit, runtime) stay independent of how many thunks were built: the thunks live only
# in lorelei-thunks-<arch>, discovered at run time via LORELEI_THUNK_PATH.
set -euo pipefail
TREE="$1"
TARGET="$2"
OUT="$3"
LLVM_VER="$4"
parent="$(dirname "$TREE")"

# The thunk bits, in their deployed-layout positions (host thunk by host arch, guest thunk always
# x86_64), plus the generated thunk sources the build leaves behind on both sides.
htl="lib/${TARGET}-LoreHTL"
gtl="x86_64/lib/x86_64-LoreGTL"
db="share/lorelei/ThunkDB.json"
gen_host="share/lorelei/thunks"
gen_guest="x86_64/share/lorelei/thunks"

pack() {  # <dir> -> <OUT>/<basename>.tar.xz
    local dir="$1"
    echo "[cut] packaging $(basename "$dir") ($(du -sh "$dir" | cut -f1))..."
    XZ_OPT="-T0" tar -C "$parent" -cJf "$OUT/$(basename "$dir").tar.xz" "$(basename "$dir")"
}

# --- thunks: only the thunk bits, copied out before the base trees are stripped of them ------------
echo "[cut] deriving the thunks tree..."
tk="$parent/lorelei-thunks-$TARGET"
rm -rf "$tk"
mkdir -p "$tk/lib" "$tk/x86_64/lib" "$tk/share/lorelei"
cp -a "$TREE/$htl" "$tk/lib/"
cp -a "$TREE/$gtl" "$tk/x86_64/lib/"
[ -f "$TREE/$db" ] && cp -a "$TREE/$db" "$tk/share/lorelei/"
pack "$tk"

# --- runtime: a stripped copy with neither the toolchain nor the thunks ----------------------------
echo "[cut] deriving the runtime tree..."
rt="$parent/lorelei-runtime-$TARGET"
rm -rf "$rt"
cp -a "$TREE" "$rt"
# devel / toolchain (host side)
rm -rf "$rt/bin" "$rt/include" "$rt/lib/cmake" "$rt/lib/llvm-${LLVM_VER}" "$rt/lib/clang" \
       "$rt/share/lorelei/toolchains" "$rt/$gen_host"
rm -f  "$rt"/lib/libLLVM.so* "$rt"/lib/libclang-cpp.so* "$rt"/lib/libLoreTLCApi.so "$rt"/lib/libLoreClangExtras.a
# devel / sysroot (guest side)
rm -rf "$rt/x86_64/sysroot" "$rt/x86_64/include" "$rt/x86_64/lib/cmake" "$rt/$gen_guest"
# thunks (they ship in lorelei-thunks-<arch>, not here)
rm -rf "$rt/$htl" "$rt/$gtl" "$rt/$db"
pack "$rt"

# --- devkit: the whole tree minus the thunks -------------------------------------------------------
echo "[cut] deriving the devkit tree..."
dk="$parent/lorelei-devkit-$TARGET"
rm -rf "$dk"
mv "$TREE" "$dk"
rm -rf "$dk/$htl" "$dk/$gtl" "$dk/$db" "$dk/$gen_host" "$dk/$gen_guest"
pack "$dk"

# The extracted trees exist only to be tarred; drop them so they do not linger in the image layer.
rm -rf "$dk" "$rt" "$tk"
echo "[cut] wrote $OUT/lorelei-{devkit,runtime,thunks}-$TARGET.tar.xz"
