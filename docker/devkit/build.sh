#!/usr/bin/env bash
#
# Build the lorelei devkit image and extract the tarball to ./out. Works with plain `docker build`
# (no buildx needed): it builds the `build` stage, then docker cp's the tarball out of it. (The scratch
# `export` stage is only for the buildx `-o type=local` path; docker cp cannot run on a scratch image.)
#
# Usage: docker/devkit/build.sh [output_dir]
# For a different host arch use buildx instead (see the Dockerfile header).
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
out_dir="${1:-$repo_root/docker/devkit/out}"
image="lorelei-devkit:latest"

cd "$repo_root"

# Build proxy/mirror args through if set in the environment.
build_args=()
for v in http_proxy https_proxy HTTP_PROXY HTTPS_PROXY no_proxy NO_PROXY USE_USTC_MIRROR; do
    [ -n "${!v:-}" ] && build_args+=(--build-arg "$v=${!v}")
done

docker build "${build_args[@]}" --target build -f docker/devkit/Dockerfile -t "$image" .

# Copy the tarball out of a throwaway container of the build stage (it holds it at /home/build/out).
cid="$(docker create "$image")"
trap 'docker rm -f "$cid" >/dev/null 2>&1 || true' EXIT
mkdir -p "$out_dir"
docker cp "$cid:/home/build/out/." "$out_dir/"
echo "devkit tarball(s) in: $out_dir"
ls -la "$out_dir"
