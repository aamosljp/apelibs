#!/usr/bin/env bash

cd $(dirname $0)/..

exec_name="$0"

lib_name="$1"

possible_option_types=("boolean" "number" "string")

declare -A available_options
declare -A option_descriptions
declare -A options

add_option() {
    local option_name="$1"
    local option_type="$2"
    local description="$3"
    if [[ -z "$option_type" ]]; then
        option_type="boolean"
    fi
    if [[ -z "$description" ]]; then
        description="No description"
    fi
    for ao in "${available_options[@]}"; do
        if [[ "$ao" == "$option_name" ]]; then
            echo "ERROR: Redefined option name "$ao""
            exit 1
        fi
    done
    local is_ot=0
    for ot in "${possible_option_types[@]}"; do
        if [[ "$ot" == "$option_type" ]]; then
            is_ot=1
            break
        fi
    done
    if [[ "$is_ot" == "0" ]]; then
        echo "ERROR: Invalid option type "$ot""
        echo "Possible types are: ${possible_option_types[@]}"
    fi
    available_options["$option_name"]="$option_type"
    option_descriptions["$option_name"]="$description"
}

parse_options() {
    while [[ "$1" =~ ^- && ! "$1" == "--" ]]; do
        local valid=0
        for akey in "${!available_options[@]}"; do
            case "${available_options["$akey"]}" in
                "boolean" )
                    if [[ "$1" == "--$akey" || "$1" == "--$akey=yes" ]]; then
                        options["$akey"]="yes"
                        valid=1
                    elif [[ "$1" == "--$akey=no" ]]; then
                        options["$akey"]="no"
                        valid=1
                    fi
                    ;;
                "string" )
                    if [[ "$1" =~ "--$akey=" ]]; then
                        local s="${1:((${#akey}+3))}"
                        if [[ -z "$s" ]]; then
                            echo "Argument after --$akey= cannot be empty string"
                            exit 1
                        fi
                        options["$akey"]="$s"
                        valid=1
                    elif [[ "$1" == "--$akey" ]]; then
                        shift
                        local s="$1"
                        if [[ -z "$s" ]]; then
                            echo "Argument after --$akey cannot be empty string"
                            exit 1
                        fi
                        options["$akey"]="$s"
                        valid=1
                    fi
                    ;;
                "number" )
                    if [[ "$1" =~ "--$akey=" ]]; then
                        local s="${1:((${#akey}+3))}"
                        if [[ -z "$s" || ! "$s" =~ ^[0-9]+$ ]]; then
                            echo "Argument after --$akey= has to be a number"
                            exit 1
                        fi
                        options["$akey"]="$s"
                        valid=1
                    elif [[ "$1" == "--$akey" ]]; then
                        shift
                        local s="$1"
                        if [[ -z "$s" || ! "$s" =~ ^[0-9]\d+$ ]]; then
                            echo "Argument after --$akey has to be a number"
                            exit 1
                        fi
                        options["$akey"]="$s"
                        valid=1
                    fi
                    ;;
            esac
        done
        if [[ "$valid" == "0" ]]; then
            echo "Invalid option $1"
            echo ""
            print_usage "$exec_name"
        fi
        shift
    done
}

print_usage() {
    echo "Usage: $1 <lib_name> [OPTIONS...]"
    for key in "${!available_options[@]}"; do
        if [[ "${available_options["$key"]}" == "boolean" ]]; then
            echo "--$key|--$key=<yes|no>                ${option_descriptions["$key"]}"
        elif [[ "${available_options["$key"]}" == "number" ]]; then
            echo "--$key=<number>|--$key <number>       ${option_descriptions["$key"]}"
        elif [[ "${available_options["$key"]}" == "string" ]]; then
            echo "--$key=<string>|--$key <string>       ${option_descriptions["$key"]}"
        fi
    done
}

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

if [[ -z "$lib_name" || ! -d "$lib_name" ]]; then
    printf "ERROR: %s: No such directory\n" $lib_name
    exit 1
fi

objfiles=()
for f in $lib_name/**.c; do
    if [[ ! -z "$exclude_files" ]]; then
        local c=0
        for ef in "${exclude_files[@]}"; do
            if [[ "$f" == "$lib_name/$ef" ]]; then
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

run_command "mkdir bin"
run_command ""$CC" "${objfiles[@]}" -o bin/${lib}_test"
run_command "rm -rf ${lib}/*.o"
