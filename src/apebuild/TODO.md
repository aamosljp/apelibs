# Apebuild TODO

Apebuild is a modular build system for C. Each module can be used independently or together.

## Module Overview

### High Priority (Core Functionality)

| Module | File | Description |
|--------|------|-------------|
| Strings | [TODO_str.md](TODO_str.md) | String builder, string list, dynamic arrays, string utilities |
| Filesystem | [TODO_fs.md](TODO_fs.md) | File I/O, directory iteration, path manipulation |
| Command | [TODO_cmd.md](TODO_cmd.md) | Command building, sync/async execution, process pools |
| Logging | [TODO_log.md](TODO_log.md) | Configurable logging with levels, colors, timestamps |
| Arguments | [TODO_args.md](TODO_args.md) | Command-line argument parsing |
| Core | (this file) | Build system logic, toolchains, builders |

### Low Priority (Future Enhancements)

| Module | File | Description |
|--------|------|-------------|
| Hash | [TODO_hash.md](TODO_hash.md) | File hashing (MD5, SHA256) for content-based rebuild |
| Config | [TODO_config.md](TODO_config.md) | Read/write config files (INI, JSON, key-value) |
| Download | [TODO_dl.md](TODO_dl.md) | Download files from URLs, dependency fetching |
| Archive | [TODO_archive.md](TODO_archive.md) | Create/extract tar, zip archives |
| Platform | [TODO_platform.md](TODO_platform.md) | Platform detection, cross-compilation support |

**Current Focus:** Linux local builds with GCC/Clang

---

# Core Module TODO

The core module contains all build-system-specific functionality.

## Toolchain Configuration

Per-builder toolchain settings (no more global macros):

- [ ] ApeToolchain - Toolchain configuration structure
  - [ ] compiler - Compiler command (gcc, clang, cl.exe, etc.)
  - [ ] linker - Linker command
  - [ ] archiver - Static library archiver (ar)
  - [ ] src_extension - Source file extension (.c, .cpp, etc.)
  - [ ] obj_extension - Object file extension (.o, .obj)
  - [ ] exe_extension - Executable extension ("", .exe)
  - [ ] lib_prefix - Library prefix (lib, "")
  - [ ] static_lib_suffix - Static library suffix (.a, .lib)
  - [ ] shared_lib_suffix - Shared library suffix (.so, .dll, .dylib)
- [ ] ape_toolchain_default - Get default toolchain for current platform
- [ ] ape_toolchain_gcc - Preset for GCC
- [ ] ape_toolchain_clang - Preset for Clang
- [ ] ape_toolchain_msvc - Preset for MSVC
- [ ] ape_toolchain_clone - Clone a toolchain for modification

## Compiler/Linker Flags

- [ ] ApeFlags - Flag collection structure
- [ ] ape_flags_new - Create new flag collection
- [ ] ape_flags_add - Add flag
- [ ] ape_flags_add_include - Add include directory (-I)
- [ ] ape_flags_add_define - Add preprocessor define (-D)
- [ ] ape_flags_add_libdir - Add library search directory (-L)
- [ ] ape_flags_add_lib - Add library (-l)
- [ ] ape_flags_add_framework - Add framework (macOS)
- [ ] ape_flags_optimize - Add optimization level (-O0, -O2, -O3, -Os)
- [ ] ape_flags_debug - Add debug flags (-g)
- [ ] ape_flags_warnings - Add warning flags (-Wall, -Wextra, etc.)
- [ ] ape_flags_std - Set language standard (-std=c11, etc.)
- [ ] ape_flags_merge - Merge two flag collections

## Builder Structure

- [ ] ApeBuilder - Build target configuration
  - [ ] name - Target name
  - [ ] type - Build type enum (executable, static_lib, shared_lib, object)
  - [ ] toolchain - Pointer to toolchain config
  - [ ] sources - List of source files
  - [ ] compile_flags - Compiler flags
  - [ ] link_flags - Linker flags
  - [ ] output_dir - Output directory
  - [ ] dependencies - List of dependent builders

## Builder Creation

- [ ] ape_builder_new - Create new builder with name
- [ ] ape_builder_free - Free builder resources
- [ ] ape_builder_set_toolchain - Set toolchain for builder
- [ ] ape_builder_set_type - Set build type (exe, static, shared)
- [ ] ape_builder_set_output_dir - Set output directory

## Source Management

- [ ] ape_builder_add_source - Add single source file
- [ ] ape_builder_add_sources - Add multiple source files
- [ ] ape_builder_add_dir - Add all source files from directory
- [ ] ape_builder_add_dir_recursive - Add source files recursively
- [ ] ape_builder_add_glob - Add files matching glob pattern
- [ ] ape_builder_exclude - Exclude file from build

## Flag Management

- [ ] ape_builder_add_include - Add include directory
- [ ] ape_builder_add_define - Add preprocessor define
- [ ] ape_builder_add_cflag - Add compiler flag
- [ ] ape_builder_add_ldflag - Add linker flag
- [ ] ape_builder_add_lib - Add library dependency
- [ ] ape_builder_add_libdir - Add library search directory

## Dependencies

- [ ] ape_builder_depends_on - Add dependency on another builder
- [ ] ape_builder_link_with - Link with output of another builder

## Build Execution

- [ ] ape_builder_build - Build the target (returns success/failure)
- [ ] ape_builder_clean - Clean build artifacts
- [ ] ape_builder_rebuild - Force rebuild all
- [ ] ape_builder_needs_rebuild - Check if rebuild is needed

## Command Generation

- [ ] ape_builder_gen_compile_cmd - Generate compile command for source
- [ ] ape_builder_gen_link_cmd - Generate link command
- [ ] ape_builder_gen_all_cmds - Generate all commands for build

## Build Context

- [ ] ApeBuildContext - Global build context
  - [ ] builders - List of all builders
  - [ ] default_toolchain - Default toolchain
  - [ ] output_dir - Global output directory
  - [ ] parallel_jobs - Max parallel compilation jobs
  - [ ] verbose - Verbose output flag
- [ ] ape_context_new - Create build context
- [ ] ape_context_free - Free build context
- [ ] ape_context_add_builder - Register builder with context
- [ ] ape_context_build_all - Build all registered builders
- [ ] ape_context_build_target - Build specific target by name
- [ ] ape_context_clean_all - Clean all build artifacts

## Auto-Rebuild

- [ ] ape_self_rebuild - Check and rebuild build script if source changed
- [ ] APEBUILD_MAIN - Main function macro with auto-rebuild

## DSL Macros (Optional Convenience Layer)

- [ ] APE_BUILDER - Begin builder definition block
- [ ] APE_SOURCE - Add source file
- [ ] APE_SOURCE_DIR - Add source directory
- [ ] APE_INCLUDE - Add include directory
- [ ] APE_DEFINE - Add preprocessor define
- [ ] APE_LINK - Add library to link
- [ ] APE_DEPENDS - Add dependency

## Build Types

- [ ] APE_BUILD_EXECUTABLE - Build executable
- [ ] APE_BUILD_STATIC_LIB - Build static library
- [ ] APE_BUILD_SHARED_LIB - Build shared library
- [ ] APE_BUILD_OBJECT - Build object file only (no link)

## Utility Functions

- [ ] ape_objfile_path - Generate object file path from source path
- [ ] ape_output_path - Generate output path for target
