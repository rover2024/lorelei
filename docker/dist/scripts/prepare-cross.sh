#!/usr/bin/env bash
#
# Prepare cross-compiling to <arch> on this amd64 host: install the cross gcc, and make the target-arch
# clang/LLVM plus the thunked-library dev/runtime packages available in the multiarch tree.
#
# The LLVM/thunk packages are download-only + dpkg-deb extracted, not apt-installed: installing a
# foreign-arch package runs its maintainer scripts, some of which exec target binaries (e.g. python's
# postinst) and fail on a non-emulated host. Extraction runs no scripts and populates the same
# multiarch paths (/usr/lib/<triplet>, /usr/lib/llvm-N) an install would.
set -euo pipefail
ARCH="$1"                       # aarch64 | riscv64
LLVM_VER="${LLVM_VER:-20}"
USE_USTC_MIRROR="${USE_USTC_MIRROR:-0}"
export DEBIAN_FRONTEND=noninteractive

case "$ARCH" in
    aarch64) dpkg_arch=arm64 ;;
    riscv64) dpkg_arch=riscv64 ;;
    *) echo "[prepare-cross] unsupported arch '$ARCH'"; exit 1 ;;
esac

apt-get install -y --no-install-recommends "gcc-${ARCH}-linux-gnu" "g++-${ARCH}-linux-gnu"

# Target-arch apt source (ports), once. Pin the existing sources to amd64 so apt does not try the ports
# arch from the amd64 archive.
if ! dpkg --print-foreign-architectures | grep -qx "$dpkg_arch"; then
    dpkg --add-architecture "$dpkg_arch"
    grep -q '^Architectures: amd64' /etc/apt/sources.list.d/ubuntu.sources \
        || sed -i '/^Types: deb/a Architectures: amd64' /etc/apt/sources.list.d/ubuntu.sources
    ports="http://ports.ubuntu.com/ubuntu-ports"
    [ "$USE_USTC_MIRROR" = "1" ] && ports="http://mirrors.ustc.edu.cn/ubuntu-ports"
    cat > "/etc/apt/sources.list.d/$dpkg_arch.sources" <<EOF
Types: deb
URIs: $ports
Suites: noble noble-updates noble-security
Components: main restricted universe multiverse
Architectures: $dpkg_arch
Signed-By: /usr/share/keyrings/ubuntu-archive-keyring.gpg
EOF
    apt-get update
fi

# The host lorelei/thunks link the target-arch ffcall and the thunked libs (zlib, lzma). Install them
# (not download-only) so apt drags their dependency closure into the multiarch tree: critically
# libc6:<arch> + libc6-dev:<arch>, which the cross toolchain resolves via /usr/lib/<triplet> and
# /lib/<triplet> (the toolchain file uses the multiarch libc, not the cross gcc's bundled sysroot).
# These are simple packages whose maintainer scripts run fine on the non-emulated host (unlike
# clang/LLVM below, whose postinst execs target binaries).
apt-get install -y --no-install-recommends \
    "libffcall-dev:$dpkg_arch" "zlib1g-dev:$dpkg_arch" "liblzma-dev:$dpkg_arch"

# The target-arch clang/LLVM go into a dedicated prefix: they cannot share /usr/lib/llvm-N with the
# native x86_64 LLVM (whose clang binary must stay runnable here). ClangConfig.cmake ships in clang-N.
# The LLVM cmake config is relocatable, so pointing Clang_DIR here resolves the target libs under it.
LLVM_PREFIX="/opt/xllvm/$ARCH"
rm -rf "$LLVM_PREFIX"
mkdir -p "$LLVM_PREFIX"
apt-get install -y --no-install-recommends --download-only \
    "clang-${LLVM_VER}:$dpkg_arch" "llvm-${LLVM_VER}-dev:$dpkg_arch" "libclang-${LLVM_VER}-dev:$dpkg_arch"
for d in /var/cache/apt/archives/*_${dpkg_arch}.deb /var/cache/apt/archives/*_all.deb; do
    [ -e "$d" ] || continue
    dpkg-deb --fsys-tarfile "$d" | tar -x --skip-old-files -C "$LLVM_PREFIX" 2>/dev/null || true
done

echo "[prepare-cross] $ARCH ready (LLVM under $LLVM_PREFIX)"
