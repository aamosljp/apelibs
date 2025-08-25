#!/usr/bin/env bash

cd "$(dirname "$0")/.." || exit

private_code_replacement_identifier="#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY"

exec_name="$0"

lib_name="$1"

source "tools/lib/option_parser"

add_option "api-file" "string" "Filename containing api definitions (default: <lib_name>_api.h)"
add_option "private-file" "string" "Filename containing private definitions (default: <lib_name>_internal.h"
add_option "outfile" "string" "Filename to write the output to (default: <lib_name>.h)"
add_option "exclude-files" "string" "Exclude all files specified by pattern from release"

if [[ -z "$lib_name" ]]; then
    print_usage "$exec_name"
    exit 1
fi

parse_options "${@:2}"

api_file="${lib_name}_api.h"
private_file="${lib_name}_internal.h"
outfile="${lib_name}.h"
IFS=',' read -ra exclude_files <<< "${options[exclude-files]}"

if [[ -n "${options[api-file]}" ]]; then
    api_file="${options[api-file]}"
fi

if [[ -n "${options[private-file]}" ]]; then
    private_file="${options[private-file]}"
fi

if [[ -n "${options[outfile]}" ]]; then
    outfile="${options[outfile]}"
fi

if [[ ! "$lib_name" =~ ^[0-9a-zA-Z_-]+$ ]]; then
    echo "Library name should contain only alphanumeric characters, underscores and dashes"
    exit 1
fi

if [[ ! "$api_file" =~ ^[0-9a-zA-Z_-]+\.h$ ]]; then
    echo "Api file name should contain only alphanumeric characters, underscores and dashes and it should end with '.h'"
    exit 1
fi

if [[ ! "$private_file" =~ ^[0-9a-zA-Z_-]+\.h$ ]]; then
    echo "Private file name should contain only alphanumeric characters, underscores and dashes and it should end with '.h'"
    exit 1
fi

if [[ ! -d "src/$lib_name" ]]; then
    echo "Source for library $lib_name does not exist, create it first"
fi

# declare -A private_includes
# private_includes=()

generate_private() {
    local pconcat=""
    local private_includes
    declare -A private_includes
    for f in src/"$lib_name"/**.c; do
        if [[ "$f" == "src/$lib_name/test.c" ]]; then
            continue
        fi
        if [[ -n "${exclude_files[*]}" ]]; then
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
        local fcontent
        fcontent=$(printf "/* BEGIN %s */\n" "$(basename "$f")")
        local raw
        raw=$(cat "$f")
        IFS=$'\n' 
        for line in "${raw[@]}"; do
            if [[ "$line" =~ ^#include ]]; then
                if [[ "${line:9}" != "\"$private_file\"" ]]; then
                    private_includes[${line:9}]=1
                fi
            else
                fcontent=$(printf "%s\n%s" "$fcontent" "$line")
            fi
        done
        fcontent=$(printf "%s\n/* END %s */\n" "$fcontent" "$(basename "$f")")
        pconcat=$(printf "%s\n\n%s\n" "$pconcat" "$fcontent")
    done
    for key in "${!private_includes[@]}"; do
        pconcat=$(printf "#include %s\n%s" "$key" "$pconcat")
    done
    echo "$pconcat"
}

private=$(generate_private)

if [[ ! -d "include" ]]; then
    mkdir "include"
fi
pfile_content=$(cat "src/$lib_name/$private_file")
pfile_content="${pfile_content/"#include \"$api_file\""/}"
afile_content=$(cat "src/$lib_name/$api_file")
apfile_contents=$(printf "%s\n\n#ifdef %s_IMPLEMENTATION\n%s\n\n#endif" "$afile_content" "${lib_name^^}" "$pfile_content")
full_contents="${apfile_contents/"$private_code_replacement_identifier"/$private}"

printf "%s\n" "$full_contents" > "include/$outfile"
# concatenate_source_files

# csrc_files=$(concatenate_source_files)
#
# pfile_contents=$(cat $lib_name/$private_file)
# afile_contents=""
# # for key in "${!private_includes[@]}"; do
# #     echo "$key"
# #     afile_contents=$(printf "$afile_contents\n#include $key")
# # done
# afile_contents=$(printf "$(cat $lib_name/$api_file)")
# pfile_contents="${pfile_contents/"#include \"$api_file\""/}"
# apfile_contents=$(printf "$afile_contents\n\n#ifdef ${lib_name^^}_IMPLEMENTATION\n$pfile_contents\n\n#endif")
# full_contents="${apfile_contents/"$private_code_replacement_identifier"/$csrc_files}"
# echo "$full_contents" > "$outfile"
#
