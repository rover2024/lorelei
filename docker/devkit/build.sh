#!/usr/bin/env bash
#
# Build the lorelei devkit image and extract the tarball. Works with plain `docker build` (no buildx
# needed): it builds the `build` stage, then docker cp's the tarball out of it. (The scratch `export`
# stage is only for the buildx `-o type=local` path; docker cp cannot run on a scratch image.)
#
# Usage: docker/devkit/build.sh [-o|--output PATH]
#   PATH is a directory (the tarball is placed inside as lorelei-devkit-<arch>.tar.xz) or a *.tar.xz
#   file path (used verbatim, so you choose the name). Default: docker/devkit/out/. Relative paths are
#   resolved against the current directory. A bare positional argument is accepted for the same purpose.
# For a different host arch use buildx instead (see the Dockerfile header).
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
output="$repo_root/docker/devkit/out"
image="lorelei-devkit:latest"

while [ $# -gt 0 ]; do
    case "$1" in
        -o|--output) output="${2:?--output needs a path}"; shift 2 ;;
        -h|--help)
            echo "usage: docker/devkit/build.sh [-o|--output PATH]"
            echo "  PATH: a directory (tarball placed inside as lorelei-devkit-<arch>.tar.xz) or a"
            echo "        *.tar.xz file path (used verbatim). Default: docker/devkit/out/"
            exit 0 ;;
        -*) echo "unknown option: $1" >&2; exit 2 ;;
        *) output="$1"; shift ;;
    esac
done

# Resolve relative to the invoking cwd before we cd into the repo.
output="$(realpath -m -- "$output")"

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

if [[ "$output" == *.tar.xz ]]; then
    # A file path: copy the produced tarball to exactly this name.
    mkdir -p "$(dirname "$output")"
    tmp="$(mktemp -d)"
    docker cp "$cid:/home/build/out/." "$tmp/"
    mv "$(ls "$tmp"/*.tar.xz | head -1)" "$output"
    rm -rf "$tmp"
    echo "devkit tarball: $output"
    ls -la "$output"
else
    # A directory: drop the tarball inside under its default name.
    mkdir -p "$output"
    docker cp "$cid:/home/build/out/." "$output/"
    echo "devkit tarball(s) in: $output"
    ls -la "$output"
fi
