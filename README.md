# Apelibs
A collection of public-odmain, single-header C libraries and tiny tools to help build and ship them.

Almost everything in this repo is released into the public domain, check each header for details


## Libraries

Each library will have its own instructions

- ape_line.h - line input/editor + termios-wrapper
- ape_args.h - command-line argument parser

I will be adding more libraries overtime. PRs and issues are also welcome.

## Tools
- tools/initialize_library.sh - copy template and initialize a new library
- tools/generate_from_source.sh - generate a header from collection of source files
- tools/build_test.sh - build a test program for library (if it includes test.c)

All of the scripts are written in bash and use some unix commands

### `tools/initialize_library.sh`
Copy files from template directory into a new directory specified by the first parameter
Replaces all occurrences of template with the new library name

Other optional parameters are:
    `--api-file=<filename> | --api-file <filename>`             Filename containing api definitions (default: <lib_name>_api.h)
    `--private-file=<filename> | --private-file <filename>`     Filename containing private definitions (default: <lib_name>_internal.h)

Example:
```bash
./tools/initialize_library.sh ape_example
```
Output will look like this:
```
ape_line/
    ape_line_api.h
    ape_line_internal.h
    test.c
```

### `tools/generate_from_source.sh`
Build the library specified by the first argument.
Combines the api and private files and appends any source files.
test.c is never included in builds

Optional parameters:
    `--api-file=<filename> | --api-file <filename>`             Filename containing api definitions (default: <lib_name>_api.h)
    `--private-file=<filename> | --private-file <filename>`     Filename containing private definitions (default: <lib_name>_internal.h)
    `--outfile=<filename> | --outfile <filename>`               Output filename (default: <lib_name>.h)
    `--exclude-files=<list> | --exclude-files <list>`           Ignore files specified in list (should be comma(,)-separated)

Example:
```bash
./tools/generate_from_source.sh ape_example
```

It will combine the following into ape_line.h
```
ape_line/
    ape_line_api.h
    ape_line_internal.h
    ape_line_editor.c
    ape_line_history.c
    ape_line_error.c
    ape_line_main.c
```

### `tools/build_test.sh`
NOTE: This requires a C compiler

Will build the test file for specified library.

Optional parameters:
    `--exclude-files=<list> | --exclude-files <list>`   Ignore files specified in list (should be comma(,)-separated)
    `--cc=<command> | --cc <command>`                   Specify C compiler to use

Example:
```bash
./tools/build_test.sh ape_line
```
This should output `bin/ape_line_test` which can then be executed

## License
Public domain. Anyone can use, modify and redistribute these files for any purpose, commercial or private, without restriction.
