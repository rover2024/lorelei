#!/usr/bin/env bash
#
# Install what is needed to BUILD the lorelei distribution on an x86_64 host: a native clang/LLVM (for
# the native-arch build and for building the x86_64 guest side), CMake/Ninja, and the native lorelei
# host dependencies. Cross targets additionally add their toolchain and target-arch LLVM on demand via
# prepare-cross.sh; this only sets up the pieces shared by every target.
set -euo pipefail

export DEBIAN_FRONTEND=noninteractive
USE_USTC_MIRROR="${USE_USTC_MIRROR:-0}"
LLVM_VER="${LLVM_VER:-20}"

echo 'Acquire::Retries "5";' > /etc/apt/apt.conf.d/80-retries

if [ "$USE_USTC_MIRROR" = "1" ]; then
    sed -i -E 's@https?://(archive|security)\.ubuntu\.com/ubuntu@http://mirrors.ustc.edu.cn/ubuntu@g' \
        /etc/apt/sources.list.d/ubuntu.sources
fi

apt-get update
apt-get install -y --no-install-recommends \
    ca-certificates git curl wget xz-utils file patchelf pv \
    cmake ninja-build python3 pkg-config

# Clang/LLVM: the native compiler (bundled for the x86_64 devkit) plus the Clang LibTooling headers for
# building LoreTLC.
apt-get install -y --no-install-recommends \
    "clang-${LLVM_VER}" "lld-${LLVM_VER}" "llvm-${LLVM_VER}-dev" "libclang-${LLVM_VER}-dev"
update-alternatives --install /usr/bin/clang clang "/usr/bin/clang-${LLVM_VER}" 100
update-alternatives --install /usr/bin/clang++ clang++ "/usr/bin/clang++-${LLVM_VER}" 100

# Native lorelei host dependency: ffcall backs the VariadicAdaptor in LoreHostRT. zlib/lzma dev provide
# the headers and link libraries for the stable thunks the distribution ships. (Cross targets add the
# target-arch versions of these via prepare-cross.sh.)
apt-get install -y --no-install-recommends libffcall-dev zlib1g-dev liblzma-dev

echo "[bootstrap-dist] done"
