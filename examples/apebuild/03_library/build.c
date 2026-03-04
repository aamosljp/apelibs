/*
 * build.c - Build script for library + executable example
 *
 * This demonstrates building a static library and an executable that
 * links against it. The build system handles dependencies automatically -
 * the library is built before the executable that depends on it.
 *
 * Project structure:
 *   libmath/           - Static library sources
 *     mathlib.h        - Public header
 *     basic.c          - Basic math operations
 *     advanced.c       - Advanced math operations
 *   app/               - Application sources
 *     main.c           - Main entry point
 *   build.c            - This build script
 *
 * Usage:
 *   cc -o build build.c -I../../../include -DAPEBUILD_IMPLEMENTATION
 *   ./build
 *   ./build_output/mathapp
 */

#ifndef APEBUILD_IMPLEMENTATION
#define APEBUILD_IMPLEMENTATION
#endif
#include "apebuild.h"

int main(int argc, char **argv) {
	APE_REBUILD(argc, argv);

	ape_log_init();
	ape_log_set_colors(1);

	/* Create build context */
	ApeBuildCtx ctx;
	ape_ctx_init(&ctx);
	ape_ctx_set_output_dir(&ctx, "build_output");
	ape_ctx_set_parallel(&ctx, 4);
	ape_ctx_set_verbosity(&ctx, APE_VERBOSE_VERBOSE); /* Show commands */

	/* ========================================
	 * Build the math library (static)
	 * ======================================== */
	ApeBuilderHandle mathlib = ape_builder_new("mathlib");
	ape_builder_set_type(mathlib, APE_TARGET_STATIC_LIB);
	ape_builder_set_toolchain(mathlib, ctx.toolchain);
	ape_builder_set_output_dir(mathlib, ctx.output_dir);

	/* Add library sources */
	ape_builder_add_source(mathlib, "libmath/basic.c");
	ape_builder_add_source(mathlib, "libmath/advanced.c");

	/* Include path for library's own header */
	ape_builder_add_include(mathlib, "libmath");

	/* Compiler flags */
	ape_builder_add_cflag(mathlib, "-Wall");
	ape_builder_add_cflag(mathlib, "-Wextra");
	ape_builder_add_cflag(mathlib, "-O2");

	/* ========================================
	 * Build the application executable
	 * ======================================== */
	ApeBuilderHandle app = ape_builder_new("mathapp");
	ape_builder_set_type(app, APE_TARGET_EXECUTABLE);
	ape_builder_set_toolchain(app, ctx.toolchain);
	ape_builder_set_output_dir(app, ctx.output_dir);

	/* Add application sources */
	ape_builder_add_source(app, "app/main.c");

	/* Include path for the library header */
	ape_builder_add_include(app, "libmath");

	/* Compiler flags */
	ape_builder_add_cflag(app, "-Wall");
	ape_builder_add_cflag(app, "-Wextra");
	ape_builder_add_cflag(app, "-O2");

	/* Link with the math library
	 * This automatically:
	 * 1. Adds mathlib as a dependency (builds before app)
	 * 2. Adds the library's output directory to lib search path
	 * 3. Links with the library
	 */
	ape_builder_link_with(app, mathlib);

	/* ========================================
	 * Build everything
	 * ======================================== */
	ape_log_info("Building math library...");
	int result = ape_builder_build(mathlib);

	if (result) {
		ape_log_info("Building application...");
		result = ape_builder_build(app);
	}

	if (result) {
		ape_log_success("Build succeeded!");
		char *output = ape_builder_output_path(app);
		ape_log_info("Output: %s", output);
		APEBUILD_FREE(output);
	} else {
		ape_log_failure("Build failed");
	}

	ape_ctx_cleanup(&ctx);
	ape_build_shutdown();
	ape_log_shutdown();

	return result ? 0 : 1;
}
