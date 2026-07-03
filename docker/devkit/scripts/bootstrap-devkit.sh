#!/usr/bin/env bash
#
# Install what is needed to BUILD the lorelei devkit: a clang/LLVM 20 toolchain (bundled into the
# prefix and used to compile both sides), CMake/Ninja, the native lorelei host dependencies, and the
# amd64 apt source so the x86_64 guest sysroot debs can be downloaded later (see build-devkit.sh).
#
# Unlike docker/ci/scripts/bootstrap-ubuntu.sh this pulls no QEMU or end-to-end test dependencies and no
# x86_64 cross gcc: one clang cross-targets x86_64 with the bundled sysroot.
set -euo pipefail

export DEBIAN_FRONTEND=noninteractive
USE_USTC_MIRROR="${USE_USTC_MIRROR:-0}"
LLVM_VER="${LLVM_VER:-20}"

echo 'Acquire::Retries "5";' > /etc/apt/apt.conf.d/80-retries

if [ "$USE_USTC_MIRROR" = "1" ]; then
    sed -i -E 's@https?://(archive|security)\.ubuntu\.com/ubuntu@http://mirrors.ustc.edu.cn/ubuntu@g; s@https?://ports\.ubuntu\.com/ubuntu-ports@http://mirrors.ustc.edu.cn/ubuntu-ports@g' \
        /etc/apt/sources.list.d/ubuntu.sources
fi

apt-get update
apt-get install -y --no-install-recommends \
    ca-certificates git curl wget xz-utils file patchelf \
    cmake ninja-build python3 pkg-config

# Clang/LLVM: the toolchain compiler (bundled into the prefix) and, for building LoreTLC, the Clang
# LibTooling headers/libraries. Ubuntu 24.04 ships LLVM 20 in its own repositories.
apt-get install -y --no-install-recommends \
    "clang-${LLVM_VER}" "lld-${LLVM_VER}" "llvm-${LLVM_VER}-dev" "libclang-${LLVM_VER}-dev"

# Native lorelei host dependency: ffcall backs the VariadicAdaptor in LoreHostRT.
apt-get install -y --no-install-recommends libffcall-dev

# Make the versioned clang the default names the build scripts expect.
update-alternatives --install /usr/bin/clang clang "/usr/bin/clang-${LLVM_VER}" 100
update-alternatives --install /usr/bin/clang++ clang++ "/usr/bin/clang++-${LLVM_VER}" 100

# Add the amd64 architecture so the x86_64 guest sysroot dev/runtime debs can be downloaded. On an
# amd64 host amd64 is already native, so this is only needed on a cross (e.g. arm64) host.
host_arch="$(dpkg --print-architecture)"
if [ "$host_arch" != "amd64" ]; then
    dpkg --add-architecture amd64
    codename="$(. /etc/os-release && echo "$VERSION_CODENAME")"
    sed -i "/^Types: deb/a Architectures: ${host_arch}" /etc/apt/sources.list.d/ubuntu.sources
    amd64_mirror="http://archive.ubuntu.com/ubuntu"
    [ "$USE_USTC_MIRROR" = "1" ] && amd64_mirror="http://mirrors.ustc.edu.cn/ubuntu"
    cat >/etc/apt/sources.list.d/amd64.sources <<EOF
Types: deb
URIs: ${amd64_mirror}
Suites: ${codename} ${codename}-updates ${codename}-security
Components: main restricted universe multiverse
Architectures: amd64
Signed-By: /usr/share/keyrings/ubuntu-archive-keyring.gpg
EOF
    apt-get update
fi

echo "[bootstrap-devkit] done"
