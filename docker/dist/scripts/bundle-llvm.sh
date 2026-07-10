#!/usr/bin/env bash
#
# Bundle our self-contained Clang/LLVM prefix into a dist tree so its clang/LLVM travel with it. The
# prefix holds the arch the tree targets: the native x86_64 build for a native build, or the target-arch
# prefix that prepare-cross.sh fetched for a cross build (see fetch-llvm.sh). No target binary is run
# here, only copied.
set -euo pipefail
TREE="$1"
LLVM_VER="$2"
# The LLVM prefix to bundle. Native builds default to /opt/lore-llvm/x86_64; a cross build passes the
# target-arch prefix.
LLVM_SRC="${3:-/opt/lore-llvm/x86_64}"

cp -a "$LLVM_SRC" "$TREE/lib/llvm-${LLVM_VER}"

llvm_dir="$TREE/lib/llvm-${LLVM_VER}"

# Keep the devkit small: the end user's thunk build reaches LoreTLC through lorelei::LoreTLC and drives
# the bundled clang, never compiling against LLVM itself, so the dev headers, the cmake package files and
# the static archives are all dead weight here. The .so's, the clang resource headers and the toolchain
# binaries stay. (LoreTLC linked against this LLVM at build time, from the untrimmed prefix.)
rm -f  "$llvm_dir/lib"/*.a
rm -rf "$llvm_dir/include" "$llvm_dir/lib/cmake"
# libclang.so (the C API), libLTO and libRemarks: unused here. LoreTLC links libclang-cpp.so and the
# bundled clang driver links libclang-cpp.so / libLLVM.so; nothing loads these.
rm -f  "$llvm_dir/lib"/libclang.so* "$llvm_dir/lib"/libLTO.so* "$llvm_dir/lib"/libRemarks.so*
# Oversized tools nothing in the devkit uses (the tablegens are build-only; exegesis and c-index-test
# are benchmarking / libclang test drivers).
rm -f  "$llvm_dir/bin"/llvm-exegesis "$llvm_dir/bin"/c-index-test "$llvm_dir/bin"/*tblgen

mkdir -p "$TREE/bin"
for tool in clang clang++ clang-cpp lld ld.lld llvm-ar llvm-nm llvm-objcopy llvm-strip; do
    if [ -e "$llvm_dir/bin/$tool" ]; then
        ln -sf "../lib/llvm-${LLVM_VER}/bin/$tool" "$TREE/bin/$tool"
    fi
done

# Mirror lib{LLVM,clang-cpp}.so.<ver> into <tree>/lib for the host tools (LoreTLC) whose RUNPATH is
# <tree>/lib; the bundled clang finds them under its own RUNPATH ($ORIGIN/../lib = llvm-N/lib).
llvm_lib="$llvm_dir/lib"
for link in "$llvm_lib"/libLLVM.so.*.* "$llvm_lib"/libclang-cpp.so.*.*; do
    base="$(basename "$link")"
    [ -f "$link" ] || continue
    ln -sf "llvm-${LLVM_VER}/lib/$base" "$TREE/lib/$base"
done

# LoreTLC (Clang LibTooling) looks for its resource dir at bin/../lib/clang/N.
ln -sfn "llvm-${LLVM_VER}/lib/clang" "$TREE/lib/clang"
