#!/usr/bin/env bash
#
# Build the lorelei distribution image and extract the tarballs. Works with plain `docker build` (no
# buildx): it builds the `build` stage, then docker cp's the tarballs out of it.
#
# Usage: docker/dist/build.sh [-o|--output DIR] [-t|--targets "x86_64 aarch64 riscv64"]
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
output="$repo_root/docker/dist/out"
targets="x86_64"
image="lorelei-dist:latest"

while [ $# -gt 0 ]; do
    case "$1" in
        -o|--output) output="${2:?}"; shift 2 ;;
        -t|--targets) targets="${2:?}"; shift 2 ;;
        -h|--help) echo "usage: docker/dist/build.sh [-o DIR] [-t \"x86_64 aarch64 riscv64\"]"; exit 0 ;;
        *) echo "unknown arg: $1" >&2; exit 2 ;;
    esac
done
output="$(realpath -m -- "$output")"
cd "$repo_root"

build_args=(--build-arg "DIST_TARGETS=$targets")
for v in http_proxy https_proxy HTTP_PROXY HTTPS_PROXY no_proxy NO_PROXY USE_USTC_MIRROR; do
    [ -n "${!v:-}" ] && build_args+=(--build-arg "$v=${!v}")
done

docker build "${build_args[@]}" --target build -f docker/dist/Dockerfile -t "$image" .

cid="$(docker create "$image")"
trap 'docker rm -f "$cid" >/dev/null 2>&1 || true' EXIT
mkdir -p "$output"
docker cp "$cid:/home/build/out/." "$output/"
echo "dist tarballs in: $output"
ls -la "$output"
