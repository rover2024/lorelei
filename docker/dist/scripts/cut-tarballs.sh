#!/usr/bin/env bash
#
# Cut the two shipping tarballs from one built tree: the full devkit (everything) and a runtime (a copy
# stripped of build-only bits: tools, headers, cmake packages, the bundled LLVM, the guest sysroot and
# the generated thunk sources). What the runtime keeps is the lorelei runtime .so's, the thunk
# libraries (HTL/GTL) and ThunkDB.json.
#
# Each tarball unpacks to a top-level directory matching its own name (lorelei-devkit-<arch> /
# lorelei-runtime-<arch>), not the internal build-tree name.
set -euo pipefail
TREE="$1"
TARGET="$2"
OUT="$3"
LLVM_VER="$4"
parent="$(dirname "$TREE")"

# --- runtime: a stripped copy, named for its tarball -----------------------------------------------
echo "[cut] deriving the runtime tree..."
rt="$parent/lorelei-runtime-$TARGET"
rm -rf "$rt"
cp -a "$TREE" "$rt"
# devel / toolchain (host side)
rm -rf "$rt/bin" "$rt/include" "$rt/lib/cmake" "$rt/lib/llvm-${LLVM_VER}" "$rt/lib/clang" \
       "$rt/share/lorelei/toolchains" "$rt/share/lorelei/thunks"
rm -f  "$rt"/lib/libLLVM.so* "$rt"/lib/libclang-cpp.so* "$rt"/lib/libLoreTLCApi.so "$rt"/lib/libLoreClangExtras.a
# devel / sysroot (guest side); the sysroot is under x86_64/sysroot/, separate from the guest lorelei
# runtime + GTL in x86_64/lib/, so dropping it leaves those intact.
rm -rf "$rt/x86_64/sysroot" "$rt/x86_64/include" "$rt/x86_64/lib/cmake" "$rt/x86_64/share/lorelei/thunks"
echo "[cut] packaging runtime ($(du -sh "$rt" | cut -f1))..."
XZ_OPT="-T0" tar -C "$parent" -cJf "$OUT/lorelei-runtime-$TARGET.tar.xz" "lorelei-runtime-$TARGET"

# --- devkit: the whole tree, renamed for its tarball -----------------------------------------------
dk="$parent/lorelei-devkit-$TARGET"
rm -rf "$dk"
mv "$TREE" "$dk"
echo "[cut] packaging devkit ($(du -sh "$dk" | cut -f1))..."
XZ_OPT="-T0" tar -C "$parent" -cJf "$OUT/lorelei-devkit-$TARGET.tar.xz" "lorelei-devkit-$TARGET"

# The extracted trees exist only to be tarred; drop them so they do not linger in the image layer.
rm -rf "$dk" "$rt"
echo "[cut] wrote $OUT/lorelei-{devkit,runtime}-$TARGET.tar.xz"
