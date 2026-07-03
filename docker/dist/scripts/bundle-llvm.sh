#!/usr/bin/env bash
#
# Bundle /usr/lib/llvm-N into a dist tree so its clang/LLVM travel with it. /usr/lib/llvm-N holds the
# arch the tree targets: the native arch's on a native build, or the extracted target-arch debs on a
# cross build (see bootstrap-dist.sh). No target binary is run here, only copied.
set -euo pipefail
TREE="$1"
LLVM_VER="$2"
# The LLVM tree to bundle. Native builds pass nothing (the system /usr/lib/llvm-N); a cross build passes
# the target-arch prefix that prepare-cross.sh extracted into.
LLVM_SRC="${3:-/usr/lib/llvm-${LLVM_VER}}"

cp -a "$LLVM_SRC" "$TREE/lib/llvm-${LLVM_VER}"

# Drop the static LLVM/Clang archives (~600 MB). They exist only to link tools against LLVM; the devkit
# ships a prebuilt LoreTLC and uses clang dynamically (against libLLVM.so / libclang-cpp.so), so nothing
# here needs them at run time. The .so's, the clang resource headers and the tools all stay.
rm -f "$TREE/lib/llvm-${LLVM_VER}/lib"/*.a

mkdir -p "$TREE/bin"
for tool in clang clang++ clang-cpp lld ld.lld llvm-ar llvm-nm llvm-objcopy llvm-strip; do
    if [ -e "$TREE/lib/llvm-${LLVM_VER}/bin/$tool" ]; then
        ln -sf "../lib/llvm-${LLVM_VER}/bin/$tool" "$TREE/bin/$tool"
    fi
done

# llvm-N/lib/lib{LLVM,clang-cpp}.so.<ver> are symlinks into /usr/lib/<triplet>, so the copied tree has
# only dangling links. Materialize the real target where clang's RUNPATH ($ORIGIN/../lib = llvm-N/lib)
# finds them, and mirror into <tree>/lib for the host tools (LoreTLC) whose RUNPATH is <tree>/lib.
llvm_lib="$TREE/lib/llvm-${LLVM_VER}/lib"
for link in "$llvm_lib"/libLLVM.so.*.* "$llvm_lib"/libclang-cpp.so.*.*; do
    base="$(basename "$link")"
    real="$(readlink -f "$LLVM_SRC/lib/$base")"
    [ -f "$real" ] || continue
    rm -f "$llvm_lib/$base"
    cp "$real" "$llvm_lib/$base"
    ln -sf "llvm-${LLVM_VER}/lib/$base" "$TREE/lib/$base"
done

# LoreTLC (Clang LibTooling) looks for its resource dir at bin/../lib/clang/N.
ln -sfn "llvm-${LLVM_VER}/lib/clang" "$TREE/lib/clang"
