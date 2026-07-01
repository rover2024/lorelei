#!/usr/bin/env bash
#
# Install one cross toolchain and its target runtime/dev libraries on an amd64 build machine, so it can
# cross-compile a host deploy tree for that arch (see build-deploy.sh). Run at deploy time via sudo
# rather than baked into the image, so the shared one-click image is untouched.
#
# Usage: bootstrap-cross.sh <aarch64|riscv64>
#
# arm64/riscv64 packages live on ports.ubuntu.com (the amd64 archive carries only amd64/i386), so add a
# ports source for the requested arch and pin the existing archive sources to amd64. No-op on a
# non-amd64 host.
set -euo pipefail
export DEBIAN_FRONTEND=noninteractive
USE_USTC_MIRROR="${USE_USTC_MIRROR:-0}"

arch="${1:?usage: bootstrap-cross.sh <aarch64|riscv64>}"
case "$arch" in
    aarch64) dpkg_arch=arm64;   cross=aarch64-linux-gnu ;;
    riscv64) dpkg_arch=riscv64; cross=riscv64-linux-gnu ;;
    *) echo "[bootstrap-cross] unsupported arch '$arch'"; exit 1 ;;
esac

host_arch="$(dpkg --print-architecture)"
if [ "$host_arch" != "amd64" ]; then
    echo "[bootstrap-cross] host is $host_arch, not amd64; nothing to cross-install"
    exit 0
fi

dpkg --add-architecture "$dpkg_arch"

# Pin the existing amd64-archive sources to amd64 (once), so apt does not try to fetch the ports arch
# from them.
if ! grep -q '^Architectures:' /etc/apt/sources.list.d/ubuntu.sources; then
    sed -i "/^Types: deb/a Architectures: amd64" /etc/apt/sources.list.d/ubuntu.sources
fi

# arm64/riscv64 packages come from the ports mirror. Keep a single ports source and add this arch to it
# (so calling the script for a second arch extends the same source rather than clobbering it).
codename="$(. /etc/os-release && echo "$VERSION_CODENAME")"
ports_mirror="http://ports.ubuntu.com/ubuntu-ports"
[ "$USE_USTC_MIRROR" = "1" ] && ports_mirror="http://mirrors.ustc.edu.cn/ubuntu-ports"
ports_file=/etc/apt/sources.list.d/ports.sources
if [ ! -f "$ports_file" ]; then
    cat >"$ports_file" <<EOF
Types: deb
URIs: ${ports_mirror}
Suites: ${codename} ${codename}-updates ${codename}-security
Components: main restricted universe multiverse
Architectures: ${dpkg_arch}
Signed-By: /usr/share/keyrings/ubuntu-archive-keyring.gpg
EOF
elif ! grep -qw "${dpkg_arch}" "$ports_file"; then
    sed -i "s/^Architectures: .*/& ${dpkg_arch}/" "$ports_file"
fi

apt-get update

# The cross compiler (from the amd64 archive; it bundles the target libc/libstdc++ and is
# multiarch-aware) plus the target libraries the host tree links against: ffcall (avcall, the
# VariadicAdaptor backend) and the wrapped zlib / liblzma. Their headers are arch-independent and come
# from the amd64 -dev packages already installed; only the target .so/.a are added here.
apt-get install -y --no-install-recommends \
    "gcc-${cross}" "g++-${cross}" \
    "libffcall-dev:${dpkg_arch}" "zlib1g-dev:${dpkg_arch}" "liblzma-dev:${dpkg_arch}"

echo "[bootstrap-cross] $arch done"
