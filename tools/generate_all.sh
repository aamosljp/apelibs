#!/usr/bin/env bash

cd "$(dirname "$0")/.." || exit

# ============================================================================
# Topological sort: generate dependencies before dependents
# ============================================================================

declare -A state  # "": unvisited, "visiting": in progress, "done": emitted

ordered=()

visit() {
    local lib="$1"
    if [[ "${state[$lib]}" == "done" ]]; then
        return
    fi
    if [[ "${state[$lib]}" == "visiting" ]]; then
        echo "ERROR: circular dependency detected involving '$lib'" >&2
        exit 1
    fi
    state["$lib"]="visiting"

    local deps_file="src/$lib/deps.txt"
    if [[ -f "$deps_file" ]]; then
        while IFS= read -r dep || [[ -n "$dep" ]]; do
            dep="$(echo "$dep" | xargs)"
            [[ -z "$dep" || "$dep" == \#* ]] && continue
            if [[ ! -d "src/$dep" ]]; then
                echo "ERROR: $lib depends on '$dep' which does not exist in src/" >&2
                exit 1
            fi
            visit "$dep"
        done < "$deps_file"
    fi

    state["$lib"]="done"
    ordered+=("$lib")
}

# Visit all libraries
for d in src/*; do
    [[ -d "$d" ]] || continue
    visit "$(basename "$d")"
done

# Generate in dependency order
for lib in "${ordered[@]}"; do
    echo "Building $lib"
    ./tools/generate_from_source.sh "$lib"
done
