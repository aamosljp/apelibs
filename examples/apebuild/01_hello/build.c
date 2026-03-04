/*
 * build.c - Build script for hello world example
 *
 * This demonstrates the basic usage of apebuild to compile a single-file
 * C program. The build script itself is a C program that uses the apebuild
 * library.
 *
 * Usage:
 *   1. First, compile this build script:
 *      cc -o build build.c -I../../../include -DAPEBUILD_IMPLEMENTATION
 *
 *   2. Run the build:
 *      ./build
 *
 *   3. Run the result:
 *      ./build_output/hello
 *
 * Or with the self-rebuild feature (compile once, then edit and run):
 *   cc -o build build.c -I../../../include -DAPEBUILD_IMPLEMENTATION
 *   ./build              # builds hello.c
 *   # edit build.c...
 *   ./build              # automatically rebuilds itself, then builds hello.c
 */

#ifndef APEBUILD_IMPLEMENTATION
#define APEBUILD_IMPLEMENTATION
#endif
#include "apebuild.h"

int main(int argc, char **argv) {
	/* Auto-rebuild this build script if it changes */
	APE_REBUILD(argc, argv);

	/* Initialize logging */
	ape_log_init();
	ape_log_set_colors(1);

	/* Create build context with default gcc toolchain */
	ApeBuildCtx ctx;
	ape_ctx_init(&ctx);
	ape_ctx_set_output_dir(&ctx, "build_output");
	ape_ctx_set_verbosity(&ctx, APE_VERBOSE_NORMAL);

	/* Create a builder for our executable */
	ApeBuilderHandle hello = ape_builder_new("hello");
	ape_builder_set_type(hello, APE_TARGET_EXECUTABLE);
	ape_builder_set_toolchain(hello, ctx.toolchain);
	ape_builder_set_output_dir(hello, ctx.output_dir);
	ape_builder_add_source(hello, "hello.c");

	/* Add some compiler flags */
	ape_builder_add_cflag(hello, "-Wall");
	ape_builder_add_cflag(hello, "-Wextra");

	/* Build it */
	int result = ape_builder_build(hello);

	if (result) {
		char *output_path = ape_builder_output_path(hello);
		ape_log_success("Build succeeded: %s", output_path);
		APEBUILD_FREE(output_path);
	} else {
		ape_log_failure("Build failed");
	}

	/* Cleanup */
	ape_ctx_cleanup(&ctx);
	ape_build_shutdown();
	ape_log_shutdown();

	return result ? 0 : 1; /* Return 0 on success for shell convention */
}
