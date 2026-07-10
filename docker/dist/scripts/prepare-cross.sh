#!/usr/bin/env bash
#
# Prepare cross-compiling to <arch> on this amd64 host: install the cross gcc, fetch our self-contained
# target-arch clang/LLVM, and make the thunked-library dev/runtime packages available in the multiarch
# tree.
#
# The thunk dev/runtime packages are apt-installed for the target arch (their maintainer scripts run
# fine on the non-emulated host) so apt drags their dependency closure into the multiarch tree. The
# target-arch clang/LLVM is not a distro package: it comes from fetch-llvm.sh (see below).
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

# The target-arch clang/LLVM: our self-contained build for this arch (see fetch-llvm.sh), extracted
# into a dedicated prefix. It cannot share /opt/lore-llvm/x86_64 with the native LLVM (whose clang must
# stay runnable here). Its cmake config is relocatable, so pointing Clang_DIR/LLVM_DIR under it resolves
# the target libclang-cpp.so / libLLVM.so that LoreTLC links, and the prefix carries ld.lld too (the
# guest toolchain links with -fuse-ld=lld, and on the target host the bundled clang finds it alongside).
LLVM_PREFIX="/opt/lore-llvm/$ARCH"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
bash "$SCRIPT_DIR/fetch-llvm.sh" "$ARCH" "$LLVM_PREFIX"

echo "[prepare-cross] $ARCH ready (LLVM under $LLVM_PREFIX)"
