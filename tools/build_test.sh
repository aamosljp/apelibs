#!/usr/bin/env bash

cd "$(dirname "$0")/.." || exit

exec_name="$0"

lib_name="$1"

source "tools/lib/option_parser"

if [[ -z "$lib_name" ]]; then
    print_usage "$exec_name"
    exit 1
fi

run_command() {
    echo "CMD: $*"
    eval "$*"
}

add_option "cc" "string" "C compiler to use (default: gcc)"
add_option "cxx" "string" "C++ compiler to use (default: g++)"
add_option "exclude-files" "string" "comma-separated list of files to ignore"

parse_options "${@:2}"

if [[ -n "${options[cc]}" ]]; then
    CC="${options[cc]}"
fi

if [[ -z "$CXX" ]]; then
    CXX="g++"
fi
if [[ -n "${options[cxx]}" ]]; then
    CXX="${options[cxx]}"
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

# Reads deps.txt for a given library and returns all transitive dep lib names
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
        # Recurse into the dependency's own deps first
        collect_deps "$dep"
    done < "$deps_file"
}

collect_deps "$lib_name"
dep_libs=("${!visited_deps[@]}")

# ============================================================================
# Build all source files (deps first, then the library itself)
# ============================================================================

# Build include paths: -I for each dep so its headers are found
include_flags="-Itest"
for dep in "${dep_libs[@]}"; do
    include_flags="$include_flags -Isrc/$dep"
done
include_flags="$include_flags -Isrc/$lib_name"

# Check if the library has any C++ source files
has_cpp=0
for f in src/"$lib_name"/*.cpp; do
    [[ -f "$f" ]] && has_cpp=1 && break
done

if [[ ! -d "bin" ]]; then
    run_command "mkdir bin"
fi

if [[ "$has_cpp" == "1" ]]; then
    # ----------------------------------------------------------------
    # C++ path: generate the single header and compile test against it.
    # This avoids C/C++ linkage mismatches since all library code ends
    # up in the same translation unit as the test.
    # ----------------------------------------------------------------
    lib_upper="${lib_name^^}"

    run_command "./tools/generate_from_source.sh $lib_name"

    cpp_include_flags="-Itest -Iinclude -Isrc/$lib_name"

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
        run_command "$CXX $cpp_include_flags -D${lib_upper}_IMPLEMENTATION -include include/${lib_name}.h -c $f -o ${f/.cpp/.o}"
        objfiles+=("${f/.cpp/.o}")
    done

    run_command "$CXX ${objfiles[*]} -o bin/${lib_name}_test"
    run_command "rm -rf src/${lib_name}/*.o"
else
    # ----------------------------------------------------------------
    # C-only path: compile each .c file individually and link.
    # ----------------------------------------------------------------
    build_c_file() {
        run_command "$CC $include_flags -c $1 -o ${1/.c/.o}"
    }

    objfiles=()

    # Compile dependency .c files (excluding test.c)
    for dep in "${dep_libs[@]}"; do
        for f in src/"$dep"/*.c; do
            [[ ! -f "$f" ]] && continue
            if [[ "$f" == "src/$dep/test.c" ]]; then
                continue
            fi
            build_c_file "$f"
            objfiles+=("${f/.c/.o}")
        done
    done

    # Compile the library's own .c files
    for f in src/"$lib_name"/*.c; do
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
        build_c_file "$f"
        objfiles+=("${f/.c/.o}")
    done

    run_command "$CC ${objfiles[*]} -o bin/${lib_name}_test"

    # Clean up object files from all compiled directories
    run_command "rm -rf src/${lib_name}/*.o"
    for dep in "${dep_libs[@]}"; do
        run_command "rm -rf src/${dep}/*.o"
    done
fi
