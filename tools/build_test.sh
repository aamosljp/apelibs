#!/usr/bin/env bash

cd $(dirname $0)/..

exec_name="$0"

lib_name="$1"

source "tools/lib/option_parser"

if [[ -z "$lib_name" ]]; then
    print_usage "$exec_name"
    exit 1
fi

run_command() {
    echo "CMD: $@"
    `$@`
}

build_file() {
    run_command ""$CC" -c "$1" -o "${1/.c/.o}""
}

add_option "cc" "string" "C compiler to use (default: gcc)"
add_option "exclude-files" "string" "comma-separated list of files to ignore"

parse_options "${@:2}"

if [[ ! -z "${options[cc]}" ]]; then
    CC="${options[cc]}"
fi

exclude_files=()
if [[ ! -z "${options[exclude-files]}" ]]; then
    IFS='.' read -ra exclude_files <<< "${options[exclude-files]}"
fi

if [[ -z "$lib_name" || ! -d "src/$lib_name" ]]; then
    printf "ERROR: %s: No such directory\n" $lib_name
    exit 1
fi

objfiles=()
for f in src/$lib_name/**.c; do
    if [[ ! -z "$exclude_files" ]]; then
        local c=0
        for ef in "${exclude_files[@]}"; do
            if [[ "$f" == "src/$lib_name/$ef" ]]; then
                c=1
            fi
        done
        if [[ "$c" == "1" ]]; then
            continue
        fi
    fi
    build_file "$f"
    objfiles+=("${f/.c/.o}")
done

if [[ ! -d "bin" ]]; then
    run_command "mkdir bin"
fi
run_command ""$CC" "${objfiles[@]}" -o bin/${lib_name}_test"
run_command "rm -rf src/${lib_name}/*.o"
