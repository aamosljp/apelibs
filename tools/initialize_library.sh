#!/usr/bin/env bash

cd $(dirname $0)/..

private_code_replacement_identifier="#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY"

exec_name="$0"

lib_name="$1"

source "tools/lib/option_parser"

add_option "api-file" "string" "Filename containing api definitions (default: <lib_name>_api.h)"
add_option "private-file" "string" "Filename containing private definitions (default: <lib_name>_internal.h"

if [[ -z "$lib_name" ]]; then
    print_usage "$exec_name"
    exit 1
fi

parse_options "${@:2}"

api_file=""$lib_name"_api.h"
private_file=""$lib_name"_internal.h"

if [[ ! -z "${options[api-file]}" ]]; then
    api_file="${options[api-file]}"
fi

if [[ ! -z "${options[private-file]}" ]]; then
    private_file="${options[private-file]}"
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

if [[ -d "src/$lib_name" ]]; then
    echo "Library with name $lib_name already exists"
    exit 1
fi

mkdir "src/$lib_name"
afc=$(cat "template/template_api.h")
pfc=$(cat "template/template_internal.h")
tfc=$(cat "template/test.c")
pfc="${pfc//"template"/${lib_name,,}}"
pfc="${pfc//"TEMPLATE"/${lib_name^^}}"
afc="${afc//"template"/${lib_name,,}}"
afc="${afc//"TEMPLATE"/${lib_name^^}}"
tfc="${tfc//"template"/${lib_name,,}}"
printf "%s\n" "$pfc" > "src/$lib_name/$private_file"
printf "%s\n" "$afc" > "src/$lib_name/$api_file"
printf "%s\n" "$tfc" > "src/$lib_name/test.c"
