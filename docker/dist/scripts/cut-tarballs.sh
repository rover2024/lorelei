#!/usr/bin/env bash
#
# Cut the two shipping tarballs from one built tree, each unpacking to a self-contained top-level
# directory named for itself (not the internal build-tree name):
#   lorelei-devkit-<arch>   the full toolchain + lorelei runtime + headers/sysroot/cmake
#   lorelei-runtime-<arch>  the lorelei runtime .so's alone, no toolchain
#
# No thunk pack: thunks build against the devkit, so they ship from their own repository rather than
# from this one.
set -euo pipefail
TREE="$1"
TARGET="$2"
OUT="$3"
LLVM_VER="$4"
parent="$(dirname "$TREE")"

pack() {  # <dir> -> <OUT>/<basename>.tar.xz
    local dir="$1"
    echo "[cut] packaging $(basename "$dir") ($(du -sh "$dir" | cut -f1))..."
    XZ_OPT="-T0" tar -C "$parent" -cJf "$OUT/$(basename "$dir").tar.xz" "$(basename "$dir")"
}

# --- runtime: a stripped copy without the toolchain ------------------------------------------------
echo "[cut] deriving the runtime tree..."
rt="$parent/lorelei-runtime-$TARGET"
rm -rf "$rt"
cp -a "$TREE" "$rt"
# devel / toolchain (host side)
rm -rf "$rt/bin" "$rt/include" "$rt/lib/cmake" "$rt/lib/llvm-${LLVM_VER}" "$rt/lib/clang" \
       "$rt/lib/cxx" "$rt/lib/cxx-link" "$rt/share/lorelei/toolchains"
rm -f  "$rt"/lib/libLLVM.so* "$rt"/lib/libclang-cpp.so* "$rt"/lib/libLoreTLCApi.so "$rt"/lib/libLoreClangExtras.a
rm -f  "$rt/share/lorelei/MakeThunkConfig.json"   # names the toolchain the runtime cut drops
# devel / sysroot (guest side)
rm -rf "$rt/x86_64/sysroot" "$rt/x86_64/include" "$rt/x86_64/lib/cmake"
pack "$rt"

# --- devkit: the whole tree ------------------------------------------------------------------------
echo "[cut] deriving the devkit tree..."
dk="$parent/lorelei-devkit-$TARGET"
rm -rf "$dk"
mv "$TREE" "$dk"
pack "$dk"

# The extracted trees exist only to be tarred; drop them so they do not linger in the image layer.
rm -rf "$dk" "$rt"
echo "[cut] wrote $OUT/lorelei-{devkit,runtime}-$TARGET.tar.xz"
