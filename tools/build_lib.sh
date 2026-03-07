#!/usr/bin/env bash

cd "$(dirname "$0")/.." || exit

exec_name="$0"

lib_name="$1"

source "tools/lib/option_parser"

add_option "cc" "string" "C compiler to use (default: gcc)"
add_option "cxx" "string" "C++ compiler to use (default: g++)"
add_option "exclude-files" "string" "comma-separated list of files to ignore"
add_option "output" "string" "Output filename (default: lib<name>.so)"

parse_options "${@:2}"

if [[ -z "$lib_name" ]]; then
    print_usage "$exec_name"
    exit 1
fi

run_command() {
    echo "CMD: $*"
    eval "$*"
}

if [[ -n "${options[cc]}" ]]; then
    CC="${options[cc]}"
else
    CC="gcc"
fi

if [[ -n "${options[cxx]}" ]]; then
    CXX="${options[cxx]}"
else
    CXX="g++"
fi

output_name="${options[output]}"
if [[ -z "$output_name" ]]; then
    output_name="lib${lib_name}.so"
fi

exclude_files=()
if [[ -n "${options[exclude-files]}" ]]; then
    IFS='.' read -ra exclude_files <<< "${options[exclude-files]}"
fi

if [[ -z "$lib_name" || ! -d "src/$lib_name" ]]; then
    printf "ERROR: %s: No such directory\n" "$lib_name"
    exit 1
fi

# ============================================================================
# Collect dependency source dirs (recursive)
# ============================================================================

declare -A visited_deps
collect_deps() {
    local lib="$1"
    local deps_file="src/$lib/deps.txt"
    if [[ ! -f "$deps_file" ]]; then
        return
    fi
    while IFS= read -r dep || [[ -n "$dep" ]]; do
        dep="$(echo "$dep" | xargs)"
        [[ -z "$dep" || "$dep" == \#* ]] && continue
        if [[ -n "${visited_deps[$dep]}" ]]; then
            continue
        fi
        visited_deps["$dep"]=1
        collect_deps "$dep"
    done < "$deps_file"
}

collect_deps "$lib_name"
dep_libs=("${!visited_deps[@]}")

# ============================================================================
# Build include paths
# ============================================================================

warnings="-Wall"

include_flags="-Isrc/$lib_name"
for dep in "${dep_libs[@]}"; do
    include_flags="$include_flags -Isrc/$dep"
done

# Check if the library has any C++ source files
has_cpp=0
for f in src/"$lib_name"/*.cpp; do
    [[ -f "$f" ]] && has_cpp=1 && break
done

if [[ ! -d "lib" ]]; then
    run_command "mkdir lib"
fi

if [[ "$has_cpp" == "1" ]]; then
    # C++ path: compile each .cpp file into the shared library
    lib_upper="${lib_name^^}"

    objfiles=()
    for f in src/"$lib_name"/*.cpp; do
        [[ ! -f "$f" ]] && continue
        if [[ -n ${exclude_files[0]} ]]; then
            c=0
            for ef in "${exclude_files[@]}"; do
                if [[ "$f" == "src/$lib_name/$ef" ]]; then
                    c=1
                fi
            done
            if [[ "$c" == "1" ]]; then
                continue
            fi
        fi
        run_command "$CXX $include_flags $warnings -fPIC -c $f -o ${f/.cpp/.o}"
        objfiles+=("${f/.cpp/.o}")
    done

    if [[ ${#objfiles[@]} -gt 0 ]]; then
        run_command "$CXX -shared -fPIC ${objfiles[*]} -o lib/$output_name"
        run_command "rm -f ${objfiles[*]}"
    fi
else
    # C path: compile each .c file into the shared library
    build_c_file() {
        run_command "$CC $include_flags $warnings -fPIC -c $1 -o ${1/.c/.o}"
    }

    objfiles=()

    # Compile dependency .c files
    for dep in "${dep_libs[@]}"; do
        for f in src/"$dep"/*.c; do
            [[ ! -f "$f" ]] && continue
            build_c_file "$f"
            objfiles+=("${f/.c/.o}")
        done
    done

    # Compile the library's own .c files
    for f in src/"$lib_name"/*.c; do
        [[ ! -f "$f" ]] && continue
        if [[ "$f" == *"test.c" ]]; then
            continue
        fi
        if [[ -n ${exclude_files[0]} ]]; then
            c=0
            for ef in "${exclude_files[@]}"; do
                if [[ "$f" == "src/$lib_name/$ef" ]]; then
                    c=1
                fi
            done
            if [[ "$c" == "1" ]]; then
                continue
            fi
        fi
        build_c_file "$f"
        objfiles+=("${f/.c/.o}")
    done

    if [[ ${#objfiles[@]} -gt 0 ]]; then
        run_command "$CC -shared -fPIC ${objfiles[*]} -o lib/$output_name"
    fi

    # Clean up object files
    for dep in "${dep_libs[@]}"; do
        run_command "rm -f src/${dep}/*.o"
    done
    run_command "rm -f src/${lib_name}/*.o"
fi

echo "Built shared library: lib/$output_name"
