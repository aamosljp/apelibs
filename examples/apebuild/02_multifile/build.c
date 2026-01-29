/*
 * build.c - Build script for multi-file calculator example
 *
 * This demonstrates building a project with multiple source files.
 * The build system automatically compiles each source file to an object
 * file, then links them together.
 *
 * Usage:
 *   cc -o build build.c -I../../../include -DAPEBUILD_IMPLEMENTATION
 *   ./build
 *   ./build_output/calculator
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

	/* Create build context */
	ApeBuildCtx ctx;
	ape_ctx_init(&ctx);
	ape_ctx_set_output_dir(&ctx, "build_output");
	ape_ctx_set_parallel(&ctx, 4); /* Compile up to 4 files in parallel */

	/* Create the calculator executable */
	ApeBuilderHandle calc = ape_builder_new("calculator");
	ape_builder_set_type(calc, APE_TARGET_EXECUTABLE);
	ape_builder_set_toolchain(calc, ctx.toolchain);
	ape_builder_set_output_dir(calc, ctx.output_dir);

	/* Add all source files */
	ape_builder_add_source(calc, "main.c");
	ape_builder_add_source(calc, "calc.c");

	/* Add current directory to include path (for calc.h) */
	ape_builder_add_include(calc, ".");

	/* Compiler flags */
	ape_builder_add_cflag(calc, "-Wall");
	ape_builder_add_cflag(calc, "-Wextra");
	ape_builder_add_cflag(calc, "-O2");

	/* Build */
	int result = ape_builder_build(calc);

	if (result) {
		ape_log_success("Build succeeded!");
		char *output = ape_builder_output_path(calc);
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
