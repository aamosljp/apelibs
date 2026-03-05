/*
 * build.c - Build script for WebAssembly example using Emscripten
 *
 * This demonstrates using the emcc toolchain in apebuild to compile
 * a C program to WebAssembly. The output is an HTML page that runs
 * the WASM module in the browser.
 *
 * Prerequisites:
 *   - Emscripten SDK (emcc) must be installed and in PATH
 *
 * Usage:
 *   1. Compile this build script (with a native compiler, NOT emcc):
 *      cc -o build build.c -I../../../include -DAPEBUILD_IMPLEMENTATION
 *
 *   2. Run the build:
 *      ./build
 *
 *   3. Serve and open the result in a browser:
 *      cd build_wasm && python3 -m http.server 8080
 *      # Open http://localhost:8080/hello_wasm.html
 *
 * Features demonstrated:
 *   - ape_toolchain_emcc() factory for Emscripten toolchain
 *   - APE_TARGET_WASM target type
 *   - Default embedded shell HTML file
 *   - Custom shell file override
 *   - Exported functions
 *   - Memory configuration
 *   - Preload/embed files
 *   - Additional emcc flags via ape_builder_add_cflag/add_ldflag
 */

#ifndef APEBUILD_IMPLEMENTATION
#define APEBUILD_IMPLEMENTATION
#endif
#include "apebuild.h"

int main(int argc, char **argv)
{
	APE_REBUILD(argc, argv);

	ape_log_init();
	ape_log_set_colors(1);

	/* ----------------------------------------------------------------
	 * Example 1: Basic WASM build with default shell
	 * ---------------------------------------------------------------- */

	/* Create the Emscripten toolchain */
	ApeToolchainHandle emcc = ape_toolchain_emcc();

	/* Optionally add default flags to the toolchain.
	 * These will apply to ALL builders using this toolchain. */
	ape_toolchain_add_cflag(emcc, "-O2");

	/* Create a builder targeting WASM */
	ApeBuilderHandle hello = ape_builder_new("hello_wasm");
	ape_builder_set_type(hello, APE_TARGET_WASM);
	ape_builder_set_toolchain(hello, emcc);
	ape_builder_set_output_dir(hello, "build_wasm");

	/* Add source files */
	ape_builder_add_source(hello, "main.c");

	/* Add compiler flags (emcc accepts most gcc/clang flags) */
	ape_builder_add_cflag(hello, "-Wall");
	ape_builder_add_cflag(hello, "-Wextra");

	/* Export functions callable from JavaScript.
	 * Format: underscore-prefixed C function names. */
	ape_builder_add_exported_function(hello, "_main");
	ape_builder_add_exported_function(hello, "_add");

	/* Configure WASM memory (in megabytes).
	 * initial = starting heap size, max = growth limit.
	 * Setting max > 0 automatically enables ALLOW_MEMORY_GROWTH. */
	ape_builder_set_wasm_memory(hello, 16, 64);

	/* Additional emcc-specific linker flags can be added directly.
	 * These are passed as-is to the emcc link command. */
	ape_builder_add_ldflag(hello, "-sEXPORTED_RUNTIME_METHODS=ccall,cwrap");

	/* Build it! The default embedded shell HTML will be used
	 * since we didn't call ape_builder_set_shell_file(). */
	int result = ape_builder_build(hello);

	if (result) {
		char *output_path = ape_builder_output_path(hello);
		ape_log_success("Build succeeded: %s", output_path);
		ape_log_info("Serve with: cd build_wasm && python3 -m http.server 8080");
		APEBUILD_FREE(output_path);
	} else {
		ape_log_failure("Build failed");
	}

	/* ----------------------------------------------------------------
	 * Example 2 (commented out): Using a custom shell file
	 *
	 * To use your own HTML template instead of the built-in one:
	 *
	 *   ape_builder_set_shell_file(hello, "my_shell.html");
	 *
	 * Your shell file must contain the {{{ SCRIPT }}} placeholder
	 * where emcc will inject the JavaScript glue code.
	 * ---------------------------------------------------------------- */

	/* ----------------------------------------------------------------
	 * Example 3 (commented out): JS-only output (no HTML shell)
	 *
	 * For library-style WASM modules without an HTML wrapper:
	 *
	 *   ape_builder_set_wasm_output_mode(hello, 0);
	 *
	 * This produces .js + .wasm files that you load from your
	 * own HTML/JS code.
	 * ---------------------------------------------------------------- */

	/* ----------------------------------------------------------------
	 * Example 4 (commented out): Embedding/preloading data files
	 *
	 * Embed files directly into the WASM binary:
	 *   ape_builder_add_embed_file(hello, "config.json");
	 *
	 * Preload files (downloaded at startup, for larger assets):
	 *   ape_builder_add_preload_file(hello, "assets/texture.png");
	 * ---------------------------------------------------------------- */

	/* Cleanup */
	ape_build_shutdown();
	ape_log_shutdown();

	return result ? 0 : 1;
}
