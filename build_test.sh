#!/usr/bin/env bash

lib="$1"

run_command() {
    echo "CMD: $@"
    `$@`
}

build_file() {
    run_command ""$CC" -c "$1" -o "${1/.c/.o}""
}

objfiles=()
for f in $lib/**.c; do
    build_file "$f"
    objfiles+=("${f/.c/.o}")
done

run_command "mkdir bin"
run_command ""$CC" "${objfiles[@]}" -o bin/${lib}_test"
run_command "rm -rf ${lib}/*.o"
