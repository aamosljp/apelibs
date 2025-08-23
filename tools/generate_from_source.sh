private_code_replacement_identifier="#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY"

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

add_option "api-file" "string" "Filename containing api definitions (default: ${lib_name}_api.h)"
add_option "private-file" "string" "Filename containing private definitions (default: ${lib_name}_internal.h"
add_option "outfile" "string" "Filename to write the output to (default: ${lib_name}.h)"

parse_options "${@:2}"

api_file=""$lib_name"_api.h"
private_file=""$lib_name"_internal.h"
outfile=""$lib_name".h"

if [[ ! -z "${options[api-file]}" ]]; then
    api_file="${options[api-file]}"
fi

if [[ ! -z "${options[private-file]}" ]]; then
    private_file="${options[private-file]}"
fi

if [[ ! -z "${options[outfile]}" ]]; then
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

if [[ ! -d "$lib_name" ]]; then
    echo "Source for library $lib_name does not exist, create it first"
fi

# declare -A private_includes
# private_includes=()

out="${lib_name}.h"

generate_private() {
    local pconcat=""
    local private_includes
    declare -A private_includes
    for f in $lib_name/**.c; do
        if [[ "$f" == "$lib_name/test.c" ]]; then
            continue
        fi
        local fcontent
        fcontent=$(printf "/* BEGIN %s */\n" "$(basename $f)")
        local raw=$(cat "$f")
        IFS=$'\n' 
        for line in ${raw[@]}; do
            if [[ "$line" =~ ^#include ]]; then
                if [[ "${line:9}" != "\"$private_file\"" ]]; then
                    private_includes[${line:9}]=1
                fi
            else
                fcontent=$(printf "%s\n%s" "$fcontent" "$line")
            fi
        done
        fcontent=$(printf "%s\n/* END %s */\n" "$fcontent" "$(basename $f)")
        pconcat=$(printf "%s\n\n%s\n" "$pconcat" "$fcontent")
    done
    for key in "${!private_includes[@]}"; do
        pconcat=$(printf "#include %s\n%s" "$key" "$pconcat")
    done
    echo "$pconcat"
}

private=$(generate_private)

pfile_content=$(cat $lib_name/$private_file)
pfile_content="${pfile_content/"#include \"$api_file\""/}"
afile_content=$(cat $lib_name/$api_file)
apfile_contents=$(printf "%s\n\n#ifdef %s_IMPLEMENTATION\n%s\n\n#endif" "$afile_content" "${lib_name^^}" "$pfile_content")
full_contents="${apfile_contents/"$private_code_replacement_identifier"/%s}"
printf "$full_contents\n" "$private" > "$outfile"
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
