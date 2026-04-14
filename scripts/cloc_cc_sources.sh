#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

if ! command -v cloc >/dev/null 2>&1; then
    echo "error: cloc not found in PATH" >&2
    exit 1
fi

tmp_list="$(mktemp)"
trap 'rm -f "$tmp_list"' EXIT

# C/C++ source and header extensions.
match_expr=(
    -name '*.c' -o
    -name '*.cc' -o
    -name '*.cpp' -o
    -name '*.cxx' -o
    -name '*.h' -o
    -name '*.hh' -o
    -name '*.hpp' -o
    -name '*.hxx'
)

# include/: count all matching files.
find "$repo_root/include" -type f \( "${match_expr[@]}" \) -print >>"$tmp_list"

# src/: exclude src/3rdparty and all directories named tests.
find "$repo_root/src" \
    \( -type d \( -name '3rdparty' -o -iname 'tests' \) -prune \) -o \
    \( -type f \( "${match_expr[@]}" \) -print \) >>"$tmp_list"

# Remove accidental duplicates while preserving deterministic order.
sort -u "$tmp_list" -o "$tmp_list"

if [[ ! -s "$tmp_list" ]]; then
    echo "No matching files found." >&2
    exit 0
fi

cloc --list-file="$tmp_list"
