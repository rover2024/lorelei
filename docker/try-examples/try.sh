#!/usr/bin/env bash
# One-command local trial: build the try image (once) and run both examples against a devkit you mount.
#
#   ./try.sh /path/to/devkit-dir   an unpacked devkit prefix for this host's architecture
#
# Requires docker. Unpack a lorelei devkit tarball somewhere first, then pass its directory here; it is
# mounted read-only into the container. qemu and the dlcall plugin are downloaded and built by the image.
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
IMAGE=lorelei-try

devkit=${1:-}
if [ -z "$devkit" ] || [ ! -x "$devkit/bin/x86_64-linux-gnu-clang" ]; then
    echo "usage: $0 /path/to/unpacked-devkit" >&2
    echo "       (the directory must contain bin/x86_64-linux-gnu-clang)" >&2
    exit 1
fi

echo ">> building the $IMAGE image (the first build downloads and compiles qemu; later builds are cached) ..."
# Forward host proxy env and USE_USTC_MIRROR into the build so the in-container downloads can reach the
# network, optionally via the USTC mirror. These are standard docker build args, not baked into the
# image, and nothing is added for any that is unset. To use the mirror: USE_USTC_MIRROR=1 ./try.sh ...
build_args=()
for v in http_proxy https_proxy no_proxy HTTP_PROXY HTTPS_PROXY NO_PROXY USE_USTC_MIRROR; do
    [ -n "${!v:-}" ] && build_args+=(--build-arg "$v=${!v}")
done
docker build "${build_args[@]}" -t "$IMAGE" "$HERE"

echo ">> running the hello and demo examples ..."
docker run --rm -v "$(cd "$devkit" && pwd):/opt/devkit:ro" "$IMAGE"
