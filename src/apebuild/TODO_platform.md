# Platform Module TODO

Module for platform detection, cross-compilation support, and platform-specific utilities.

**Priority: Low** - Focus is on Linux builds for now.

## Platform Detection

- [ ] ApePlatform - Platform enum
  - [ ] APE_PLATFORM_LINUX
  - [ ] APE_PLATFORM_MACOS
  - [ ] APE_PLATFORM_WINDOWS
  - [ ] APE_PLATFORM_BSD
  - [ ] APE_PLATFORM_UNKNOWN

- [ ] ApeArch - Architecture enum
  - [ ] APE_ARCH_X86
  - [ ] APE_ARCH_X86_64
  - [ ] APE_ARCH_ARM
  - [ ] APE_ARCH_ARM64
  - [ ] APE_ARCH_RISCV64
  - [ ] APE_ARCH_UNKNOWN

- [ ] ape_platform_current - Get current platform
- [ ] ape_arch_current - Get current architecture
- [ ] ape_platform_name - Get platform name string
- [ ] ape_arch_name - Get architecture name string

## Platform Checks (Compile-time macros)

- [ ] APE_LINUX - Defined if Linux
- [ ] APE_MACOS - Defined if macOS
- [ ] APE_WINDOWS - Defined if Windows
- [ ] APE_BSD - Defined if BSD
- [ ] APE_POSIX - Defined if POSIX-compatible
- [ ] APE_X86 - Defined if x86
- [ ] APE_X86_64 - Defined if x86_64
- [ ] APE_ARM - Defined if ARM
- [ ] APE_ARM64 - Defined if ARM64

## Path Conventions

- [ ] APE_PATH_SEP - Path separator character ('/' or '\\')
- [ ] APE_PATH_SEP_STR - Path separator string
- [ ] APE_PATH_LIST_SEP - Path list separator (':' or ';')
- [ ] ape_platform_normalize_path - Normalize path for current platform
- [ ] ape_platform_to_native_path - Convert to native path format
- [ ] ape_platform_from_native_path - Convert from native path format

## File Extensions

- [ ] ape_platform_exe_ext - Get executable extension ("", ".exe")
- [ ] ape_platform_obj_ext - Get object file extension (".o", ".obj")
- [ ] ape_platform_static_lib_ext - Get static lib extension (".a", ".lib")
- [ ] ape_platform_shared_lib_ext - Get shared lib extension (".so", ".dll", ".dylib")
- [ ] ape_platform_lib_prefix - Get library prefix ("lib", "")

## Environment

- [ ] ape_platform_home_dir - Get user home directory
- [ ] ape_platform_temp_dir - Get temp directory
- [ ] ape_platform_config_dir - Get user config directory
- [ ] ape_platform_cache_dir - Get user cache directory
- [ ] ape_platform_exe_path - Get path to current executable
- [ ] ape_platform_num_cpus - Get number of CPU cores

## Cross-Compilation

- [ ] ApeCrossTarget - Cross-compilation target
  - [ ] platform - Target platform
  - [ ] arch - Target architecture
  - [ ] toolchain_prefix - Toolchain prefix (e.g., "arm-linux-gnueabihf-")
  - [ ] sysroot - Sysroot path
- [ ] ape_cross_target_new - Create cross-compilation target
- [ ] ape_cross_target_from_triple - Create from target triple (e.g., "x86_64-linux-gnu")
- [ ] ape_cross_target_to_triple - Get target triple string

## Toolchain Discovery

- [ ] ape_platform_find_compiler - Find default C compiler
- [ ] ape_platform_find_cxx_compiler - Find default C++ compiler
- [ ] ape_platform_find_linker - Find default linker
- [ ] ape_platform_find_ar - Find archiver
- [ ] ape_platform_find_tool - Find tool by name in PATH

## System Information

- [ ] ape_platform_os_version - Get OS version string
- [ ] ape_platform_kernel_version - Get kernel version
- [ ] ape_platform_hostname - Get hostname
- [ ] ape_platform_username - Get current username
- [ ] ape_platform_total_memory - Get total system memory
- [ ] ape_platform_available_memory - Get available memory

## Feature Detection

- [ ] ape_platform_has_command - Check if command exists in PATH
- [ ] ape_platform_has_library - Check if library is available
- [ ] ape_platform_has_header - Check if header is available

## Windows-Specific (future)

- [ ] ape_platform_find_msvc - Find MSVC installation
- [ ] ape_platform_find_windows_sdk - Find Windows SDK
- [ ] ape_platform_setup_msvc_env - Setup MSVC environment

## macOS-Specific (future)

- [ ] ape_platform_find_xcode - Find Xcode installation
- [ ] ape_platform_find_sdk - Find macOS SDK
- [ ] ape_platform_min_version - Set minimum deployment target
