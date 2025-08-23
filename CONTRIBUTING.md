# Contributing to Apelibs

## Development environment
- A standard UNIX shell and a C compiler are required.
- The repository provides a `shell.nix` for nix users, you should make sure it doesn't do anything suspicious.

## Coding style
- Source files are formatted with `clang-format` using the settings in `.clang-format` (tabs with width 8 and a 140-column limit). Run `clang-format **.c **.h -i` before commiting.

## Working on libraries
- Each library must be self‑contained and depend only on the C standard library.
- Public declarations live in `<lib>_api.h`; private declarations go in `<lib>_internal.h` or source files; implementations belong in `.c` files.

### Creating a new library
1. Run `./tools/initialize_library.sh <lib_name>` to copy the template and replace names.
2. Add your public API definitions to `<lib_name>_api.h` and optional private definitions to `<lib_name>_internal.h`.
3. Implement functions in source files and, if desired, create `test.c`.

### Generating headers and building tests
- Generate the single-header version before committing: `./tools/generate_from_source.sh <lib_name>`.
- If a test program exists, build it with `./tools/build_test.sh <lib_name>`.
- Before pushing, run `./tools/generate_all.sh` to ensure header generation has no unexpected side effects.

## Submitting changes
1. Ensure all generated headers are up to date and tests pass.
2. Commit your changes with clear messages and open a pull request.
3. By contributing, you agree to release your changes into the public domain.
