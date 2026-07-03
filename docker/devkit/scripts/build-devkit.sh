#!/usr/bin/env bash
#
# Assemble a self-contained lorelei devkit prefix, then tar it. The prefix bundles everything a thunk
# author needs so their per-thunk config names no machine path (see MakeThunk.py --devkit-prefix):
#
#   <devkit>/bin/clang, clang++            the bundled LLVM (host arch), cross-targets x86_64 too
#   <devkit>/bin/x86_64-linux-gnu-clang(++)  wrappers: clang --target=x86_64-linux-gnu --sysroot=<devkit>/x86_64
#   <devkit>/lib/cmake/{qmsetup,lorelei}   host qmsetup + host lorelei (LoreTLC, LoreHostRT)
#   <devkit>/share/lorelei/toolchains/x86_64-linux-gnu.cmake
#   <devkit>/x86_64/                       guest sysroot: libc/libstdc++ headers+libs AND guest lorelei
#
# It is per-host-arch: on x86_64 the guest equals the host; on arm64 the same bundled clang builds the
# x86_64 guest against the bundled sysroot. QEMU, the dlcall plugin and any runner are deliberately NOT
# included (the user installs those).
set -euo pipefail
: "${LORELEI_SRC:?}"    # lorelei v2 source tree
: "${REPOS_DIR:?}"      # scratch dir for external deps (qmsetup)
: "${DEVKIT_PREFIX:?}"      # devkit prefix to assemble
: "${OUT_DIR:?}"        # where the tarball is written

LLVM_VER="${LLVM_VER:-20}"
GCC_VER="${GCC_VER:-14}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
COMMON_SCRIPTS="${LORELEI_SRC}/docker/common"
host_arch="$(uname -m)"

mkdir -p "$DEVKIT_PREFIX" "$OUT_DIR"

# --- 1. qmsetup (host) -----------------------------------------------------------------------------
INSTALL_DIR="$DEVKIT_PREFIX" bash "$COMMON_SCRIPTS/build-qmsetup.sh"

# --- 2. lorelei host side (native clang): LoreTLC + LoreHostRT ---------------------------------------
cd "$LORELEI_SRC"
cmake -B build-tc-host -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_INSTALL_PREFIX="$DEVKIT_PREFIX" \
    -Dqmsetup_DIR="$DEVKIT_PREFIX/lib/cmake/qmsetup" \
    -DLORE_BUILD_TOOLS=TRUE \
    -DLORE_BUILD_GUEST_TARGETS=FALSE \
    -DLORE_BUILD_TESTS=OFF
cmake --build build-tc-host --target install

# --- 3. bundle LLVM into the prefix (relocatable) ---------------------------------------------------
# clang locates its resource dir (lib/clang/<v>, which holds the x86_64 compiler headers) relative to
# the real clang binary, so keeping /usr/lib/llvm-N's own layout under <devkit>/lib/llvm-N makes it
# relocatable. bin/ symlinks point at it.
llvm_src="/usr/lib/llvm-${LLVM_VER}"
cp -a "$llvm_src" "$DEVKIT_PREFIX/lib/llvm-${LLVM_VER}"
mkdir -p "$DEVKIT_PREFIX/bin"
for tool in clang clang++ clang-cpp lld ld.lld llvm-ar llvm-nm llvm-objcopy llvm-strip; do
    if [ -e "$DEVKIT_PREFIX/lib/llvm-${LLVM_VER}/bin/$tool" ]; then
        ln -sf "../lib/llvm-${LLVM_VER}/bin/$tool" "$DEVKIT_PREFIX/bin/$tool"
    fi
done

# In LLVM's packaging llvm-N/lib/lib{LLVM,clang-cpp}.so.<ver> are symlinks into /usr/lib/<triplet>, so
# the copied llvm-N tree has only dangling links (the real ~130MB libLLVM is not there). Replace them
# with the real targets, where the bundled clang's RUNPATH ($ORIGIN/../lib = llvm-N/lib) finds them, and
# mirror them into <devkit>/lib for the host tools (LoreTLC) whose RUNPATH is <devkit>/lib instead.
llvm_lib="$DEVKIT_PREFIX/lib/llvm-${LLVM_VER}/lib"
for link in "$llvm_lib"/libLLVM.so.*.* "$llvm_lib"/libclang-cpp.so.*.*; do
    base="$(basename "$link")"
    real="$(readlink -f "/usr/lib/llvm-${LLVM_VER}/lib/$base")"
    [ -f "$real" ] || continue
    rm -f "$llvm_lib/$base"
    cp "$real" "$llvm_lib/$base"
    ln -sf "llvm-${LLVM_VER}/lib/$base" "$DEVKIT_PREFIX/lib/$base"
done

# LoreTLC (Clang LibTooling) looks for its resource dir (stddef.h, stdarg.h, ...) at bin/../lib/clang/N.
# The bundled resource dir is under llvm-N/lib/clang, so point lib/clang there.
ln -sfn "llvm-${LLVM_VER}/lib/clang" "$DEVKIT_PREFIX/lib/clang"

# --- 4. x86_64 guest sysroot (self-contained, host-arch independent) --------------------------------
# Download the amd64 dev/runtime debs and extract them into <devkit>/x86_64, so the sysroot carries its own
# libc/libstdc++ headers and libraries and the user needs no system x86_64 dev packages. Using the debs
# (not the host's merged /usr) keeps it correct even when the build host is arm64.
sysroot="$DEVKIT_PREFIX/x86_64"
mkdir -p "$sysroot"
tmp="$(mktemp -d)"
(
    cd "$tmp"
    # libc/libstdc++ headers + libraries, plus ffcall: the guest DLCall links libavcall.a (the ffcall
    # VariadicAdaptor backend), so libffcall-dev provides libavcall.a and avcall.h in the sysroot.
    apt-get download \
        libc6:amd64 libc6-dev:amd64 linux-libc-dev:amd64 \
        "libgcc-s1:amd64" "libgcc-${GCC_VER}-dev:amd64" \
        "libstdc++6:amd64" "libstdc++-${GCC_VER}-dev:amd64" \
        libffcall-dev:amd64
    for deb in *.deb; do dpkg-deb -x "$deb" "$sysroot"; done
)
rm -rf "$tmp"
# usrmerge compat: the noble debs are merged-/usr (only usr/ is extracted), but libc.so's linker
# script still names /lib/x86_64-linux-gnu/libc.so.6 and /lib64/ld-linux-x86-64.so.2, which the linker
# reroots under the sysroot. Point /lib and /lib64 at usr/ so those resolve.
ln -s usr/lib "$sysroot/lib"
ln -s usr/lib64 "$sysroot/lib64"

# --- 5. wrappers -----------------------------------------------------------------------------------
# For compiling the user's own x86_64 guest test programs by hand; the GTL cmake build uses the guest
# toolchain file instead.
cat > "$DEVKIT_PREFIX/bin/x86_64-linux-gnu-clang" <<EOF
#!/bin/sh
tc="\$(CDPATH= cd -- "\$(dirname -- "\$0")/.." && pwd)"
exec "\$tc/bin/clang" --target=x86_64-linux-gnu --sysroot="\$tc/x86_64" "\$@"
EOF
cat > "$DEVKIT_PREFIX/bin/x86_64-linux-gnu-clang++" <<EOF
#!/bin/sh
tc="\$(CDPATH= cd -- "\$(dirname -- "\$0")/.." && pwd)"
exec "\$tc/bin/clang++" --target=x86_64-linux-gnu --sysroot="\$tc/x86_64" "\$@"
EOF
chmod +x "$DEVKIT_PREFIX/bin/x86_64-linux-gnu-clang" "$DEVKIT_PREFIX/bin/x86_64-linux-gnu-clang++"

# --- 6. guest cmake toolchain file ------------------------------------------------------------------
mkdir -p "$DEVKIT_PREFIX/share/lorelei/toolchains"
cp "$SCRIPT_DIR/../cmake/toolchain/x86_64-linux-gnu.cmake" "$DEVKIT_PREFIX/share/lorelei/toolchains/x86_64-linux-gnu.cmake"

# --- 7. lorelei guest side (bundled clang + guest toolchain file): LoreGuestRT -----------------------
# This doubles as an in-build validation of the sysroot + guest.cmake: if LoreGuestRT cross-builds and
# links here, a thunk GTL will too.
cd "$LORELEI_SRC"
cmake -B build-tc-guest -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE="$DEVKIT_PREFIX/share/lorelei/toolchains/x86_64-linux-gnu.cmake" \
    -DCMAKE_INSTALL_PREFIX="$DEVKIT_PREFIX/x86_64" \
    -Dqmsetup_DIR="$DEVKIT_PREFIX/lib/cmake/qmsetup" \
    -DLORE_BUILD_TOOLS=FALSE \
    -DLORE_BUILD_GUEST_TARGETS=TRUE \
    -DLORE_BUILD_TESTS=OFF
cmake --build build-tc-guest --target install

# --- 8. package ------------------------------------------------------------------------------------
# The prefix bundles a full LLVM (multi-GB unpacked), so compress with all cores; single-threaded xz
# here looks like a hang.
tar_name="lorelei-devkit-${host_arch}.tar.xz"
echo "[build-devkit] packaging $tar_name (xz -T0, this is the big one)..."
XZ_OPT="-T0" tar -C "$(dirname "$DEVKIT_PREFIX")" -cJf "$OUT_DIR/$tar_name" "$(basename "$DEVKIT_PREFIX")"
echo "[build-devkit] wrote $OUT_DIR/$tar_name"
