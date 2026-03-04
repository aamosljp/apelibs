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

# ============================================================================
# License and version handling
# ============================================================================

lib_upper="${lib_name^^}"

# Read LICENSE file and format as C comment
if [[ ! -f "LICENSE" ]]; then
    echo "ERROR: LICENSE file not found in repository root" >&2
    exit 1
fi

license_comment="/*
$(sed 's/^/ * /' LICENSE)
 */"

# Extract version macros from api.h
version_major=""
version_minor=""
api_path="src/$lib_name/$api_file"

if [[ -f "$api_path" ]]; then
    # Look for #define <LIB>_VERSION_MAJOR <number>
    version_major=$(grep -E "^#define ${lib_upper}_VERSION_MAJOR [0-9]+" "$api_path" | awk '{print $3}')
    version_minor=$(grep -E "^#define ${lib_upper}_VERSION_MINOR [0-9]+" "$api_path" | awk '{print $3}')
fi

# Warn if version macros are missing
if [[ -z "$version_major" ]]; then
    echo "WARNING: $api_file is missing version macro (${lib_upper}_VERSION_MAJOR)" >&2
fi
if [[ -z "$version_minor" ]]; then
    echo "WARNING: $api_file is missing version macro (${lib_upper}_VERSION_MINOR)" >&2
fi

# Read optional usage.txt
usage_content=""
usage_path="src/$lib_name/usage.txt"
if [[ -f "$usage_path" ]]; then
    usage_content="$(sed 's/^/ * /' "$usage_path")"
fi

# Generate version comment if both macros are present
version_comment=""
if [[ -n "$version_major" && -n "$version_minor" ]]; then
    if [[ -n "$usage_content" ]]; then
        version_comment="/*
 * ${lib_name}.h - v${version_major}.${version_minor}
 *
$usage_content
 */"
    else
        version_comment="/*
 * ${lib_name}.h - v${version_major}.${version_minor}
 */"
    fi
fi

# ============================================================================
# Dependency handling
# ============================================================================

# Read deps.txt if present, storing dep names in an array
deps=()
if [[ -f "src/$lib_name/deps.txt" ]]; then
    while IFS= read -r dep || [[ -n "$dep" ]]; do
        dep="$(echo "$dep" | xargs)" # trim whitespace
        [[ -z "$dep" || "$dep" == \#* ]] && continue  # skip empty lines and comments
        deps+=("$dep")
    done < "src/$lib_name/deps.txt"
fi

# Generate the dependency preamble for the single-header output.
# For each dep, we emit a preprocessor block:
#   #ifndef <LIB>_EXTERNAL_<DEP>
#     /* --- embedded <dep>.h begin --- */
#     <contents of include/<dep>.h>
#     /* --- embedded <dep>.h end --- */
#   #else
#     #include "<dep>.h"
#   #endif
generate_dep_preamble() {
    local lib_upper="${lib_name^^}"
    for dep in "${deps[@]}"; do
        local dep_upper="${dep^^}"
        local dep_header="include/${dep}.h"
        if [[ ! -f "$dep_header" ]]; then
            echo "ERROR: dependency '$dep' header not found at $dep_header" >&2
            echo "       Run ./tools/generate_from_source.sh $dep first" >&2
            exit 1
        fi
        local dep_impl_guard="${dep_upper}_IMPLEMENTATION"
        echo "#ifndef ${lib_upper}_EXTERNAL_${dep_upper}"
        echo "/* --- embedded ${dep}.h begin --- */"
        # Emit the dependency header, but strip any trailing #endif that closes
        # the include guard — we need to also force-define the IMPLEMENTATION
        # guard so the dep's implementation code is included.
        echo "#ifndef ${dep_impl_guard}"
        echo "#define ${dep_impl_guard}"
        echo "#endif"
        cat "$dep_header"
        echo ""
        echo "/* --- embedded ${dep}.h end --- */"
        echo "#else"
        echo "#include \"${dep}.h\""
        echo "#endif /* ${lib_upper}_EXTERNAL_${dep_upper} */"
        echo ""
    done
}

dep_preamble=""
if [[ ${#deps[@]} -gt 0 ]]; then
    dep_preamble="$(generate_dep_preamble)
"
fi

# ============================================================================
# Private code generation (concatenate .c files)
# ============================================================================

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
        fcontent="/* BEGIN $(basename "$f") */"
        while IFS= read -r line || [[ -n "$line" ]]; do
            if [[ "$line" =~ ^#include ]]; then
                if [[ "${line:9}" != "\"$private_file\"" ]]; then
                    # Skip includes of dependency api headers — they're already
                    # available from the embedded/external dep preamble.
                    local skip=0
                    for dep in "${deps[@]}"; do
                        if [[ "${line:9}" == "\"${dep}_api.h\"" ]]; then
                            skip=1
                            break
                        fi
                    done
                    if [[ "$skip" == "0" ]]; then
                        private_includes[${line:9}]=1
                    fi
                fi
            else
                fcontent="$fcontent
$line"
            fi
        done < "$f"
        fcontent="$fcontent
/* END $(basename "$f") */"
        pconcat="$pconcat

$fcontent
"
    done
    for key in "${!private_includes[@]}"; do
        pconcat="#include $key
$pconcat"
    done
    echo "$pconcat"
}

private=$(generate_private)

if [[ ! -d "include" ]]; then
    mkdir "include"
fi
pfile_content=$(cat "src/$lib_name/$private_file")
pfile_content="${pfile_content/"#include \"$api_file\""/}"

# Strip #include of dependency api headers from the internal header
for dep in "${deps[@]}"; do
    pfile_content="${pfile_content//"#include \"${dep}_api.h\""/}"
done

afile_content=$(cat "src/$lib_name/$api_file")

# Strip #include of dependency api headers from the api header
for dep in "${deps[@]}"; do
    afile_content="${afile_content//"#include \"${dep}_api.h\""/}"
done

# Build the header preamble (license + version comment)
header_preamble="$license_comment"
if [[ -n "$version_comment" ]]; then
    header_preamble="$header_preamble

$version_comment"
fi

apfile_contents="${header_preamble}

${dep_preamble}${afile_content}

#ifdef ${lib_upper}_IMPLEMENTATION
$pfile_content

#endif"
# Escape special characters in the replacement string for bash parameter substitution
# & refers to the matched pattern, \ is interpreted as escape
private_escaped="${private//\\/\\\\}"  # First escape backslashes
private_escaped="${private_escaped//&/\\&}"  # Then escape ampersands
full_contents="${apfile_contents/"$private_code_replacement_identifier"/$private_escaped}"

printf '%s\n' "$full_contents" > "include/$outfile"
