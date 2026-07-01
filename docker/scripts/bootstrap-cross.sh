#!/usr/bin/env bash
#
# Install the aarch64 cross toolchain and the arm64 runtime/dev libraries, so an amd64 build machine
# can cross-compile the aarch64 host deploy tree (see build-deploy.sh). Run at deploy time via sudo
# rather than baked into the image, so the shared one-click image is untouched.
#
# arm64 packages live on ports.ubuntu.com (the amd64 archive carries only amd64/i386), so add a ports
# source restricted to arm64 and pin the existing archive sources to amd64. No-op on a non-amd64 host.
set -euo pipefail
export DEBIAN_FRONTEND=noninteractive
USE_USTC_MIRROR="${USE_USTC_MIRROR:-0}"

host_arch="$(dpkg --print-architecture)"
if [ "$host_arch" != "amd64" ]; then
    echo "[bootstrap-cross] host is $host_arch, not amd64; nothing to cross-install"
    exit 0
fi

dpkg --add-architecture arm64

# Pin the existing amd64-archive sources to amd64, so apt does not try to fetch arm64 from them.
if ! grep -q '^Architectures:' /etc/apt/sources.list.d/ubuntu.sources; then
    sed -i "/^Types: deb/a Architectures: amd64" /etc/apt/sources.list.d/ubuntu.sources
fi

codename="$(. /etc/os-release && echo "$VERSION_CODENAME")"
ports_mirror="http://ports.ubuntu.com/ubuntu-ports"
[ "$USE_USTC_MIRROR" = "1" ] && ports_mirror="http://mirrors.ustc.edu.cn/ubuntu-ports"
cat >/etc/apt/sources.list.d/arm64.sources <<EOF
Types: deb
URIs: ${ports_mirror}
Suites: ${codename} ${codename}-updates ${codename}-security
Components: main restricted universe multiverse
Architectures: arm64
Signed-By: /usr/share/keyrings/ubuntu-archive-keyring.gpg
EOF

apt-get update

# The aarch64 cross compiler (from the amd64 archive; it bundles the aarch64 libc/libstdc++ and is
# multiarch-aware) plus the arm64 libraries the host tree links against: ffcall (avcall, the
# VariadicAdaptor backend) and the wrapped zlib / liblzma. Their headers are arch-independent and come
# from the amd64 -dev packages already installed; only the arm64 .so/.a are added here.
apt-get install -y --no-install-recommends \
    gcc-aarch64-linux-gnu g++-aarch64-linux-gnu \
    libffcall-dev:arm64 zlib1g-dev:arm64 liblzma-dev:arm64

echo "[bootstrap-cross] done"
