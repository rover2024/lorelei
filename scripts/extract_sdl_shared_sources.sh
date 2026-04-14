#!/usr/bin/env bash
set -euo pipefail

# Extract source files that are compiled into the SDL shared library target.
# Priority:
# 1) Use jq if available (robust JSON parse).
# 2) Fallback to a simple line-based parser for compile_commands.json.

compile_commands="${1:-/home/overworld/Documents/rover2024/SDL/build/compile_commands.json}"
output_file="${2:-}"

if [[ ! -f "$compile_commands" ]]; then
    echo "error: compile_commands.json not found: $compile_commands" >&2
    exit 1
fi

tmp="$(mktemp)"
cleanup() {
    rm -f "$tmp"
}
trap cleanup EXIT

if command -v jq >/dev/null 2>&1; then
    # Find candidate object dirs for shared target entries (SDL2_EXPORTS + CMakeFiles/*.dir).
    mapfile -t candidates < <(
        jq -r '
            .[]
            | select(
                (.command | contains("-DSDL2_EXPORTS"))
                and (.output | test("/CMakeFiles/[^/]+\\.dir/"))
            )
            | (.output | capture("/CMakeFiles/(?<name>[^/]+)\\.dir/").name)
        ' "$compile_commands" | sort -u
    )

    if [[ "${#candidates[@]}" -eq 0 ]]; then
        echo "error: no shared SDL target candidates found in $compile_commands" >&2
        exit 1
    fi

    selected=""
    for c in "${candidates[@]}"; do
        if [[ "$c" == "SDL2" || "$c" == "SDL3" ]]; then
            selected="$c"
            break
        fi
    done
    if [[ -z "$selected" ]]; then
        selected="${candidates[0]}"
    fi

    jq -r --arg target "$selected" '
        .[]
        | select(.output | test("/CMakeFiles/" + $target + "\\.dir/"))
        | .file
    ' "$compile_commands" | sort -u >"$tmp"
else
    # Fallback parser (expects keys "command", "file", "output" in each object).
    awk '
        BEGIN { cmd=""; file=""; out=""; inobj=0; target="" }
        /^[[:space:]]*{/ { inobj=1; cmd=""; file=""; out=""; next }
        inobj && /"command":[[:space:]]*"/ {
            line=$0
            sub(/^.*"command":[[:space:]]*"/, "", line)
            sub(/",[[:space:]]*$/, "", line)
            cmd=line
            next
        }
        inobj && /"file":[[:space:]]*"/ {
            line=$0
            sub(/^.*"file":[[:space:]]*"/, "", line)
            sub(/",[[:space:]]*$/, "", line)
            file=line
            next
        }
        inobj && /"output":[[:space:]]*"/ {
            line=$0
            sub(/^.*"output":[[:space:]]*"/, "", line)
            sub(/"[[:space:]]*[,]?$/, "", line)
            out=line
            next
        }
        inobj && /^[[:space:]]*}[[:space:]]*,?[[:space:]]*$/ {
            inobj=0
            if (cmd ~ /-DSDL2_EXPORTS/ && out ~ /\/CMakeFiles\/[^/]+\.dir\//) {
                split(out, parts, "/CMakeFiles/")
                if (length(parts) > 1) {
                    split(parts[2], rest, ".dir/")
                    cand=rest[1]
                    if (target == "" || cand == "SDL2" || cand == "SDL3") {
                        target=cand
                    }
                }
            }
            records[++n_cmd]=cmd
            files[n_cmd]=file
            outs[n_cmd]=out
        }
        END {
            if (target == "") {
                print "error: no shared SDL target candidates found" >"/dev/stderr"
                exit 1
            }
            for (i=1; i<=n_cmd; ++i) {
                if (outs[i] ~ ("/CMakeFiles/" target "\\.dir/")) {
                    print files[i]
                }
            }
        }
    ' "$compile_commands" | sort -u >"$tmp"
fi

if [[ -n "$output_file" ]]; then
    cp "$tmp" "$output_file"
fi

cat "$tmp"
