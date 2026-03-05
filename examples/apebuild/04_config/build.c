#include "../../../include/apebuild.h"
#include "../../../include/ape_config.h"
#include <stdio.h>

int main(int argc, char **argv)
{
	APE_REBUILD(argc, argv);

	/* Initialize build context */
	ApeBuildCtx ctx;
	ape_ctx_init(&ctx);
	ape_ctx_set_output_dir(&ctx, "build");

	/* Create a builder */
	ApeBuilderHandle builder = ape_builder_new("config_example");
	ape_builder_set_type(builder, APE_TARGET_EXECUTABLE);
	ape_builder_add_source(builder, "main.c");

	/* Build */
	if (!ape_ctx_build(&ctx, builder)) {
		printf("Build failed!\n");
		return 1;
	}

	/* Test config module */
	ApeConfig config;
	ape_config_init(&config);

	/* Set some config values */
	ape_config_set_string(&config, "project.name", "MyProject");
	ape_config_set_int(&config, "project.version", 1);
	ape_config_set_bool(&config, "project.debug", 1);

	/* Write config to JSON */
	char *json = ape_config_write(&config);
	printf("Config JSON:\n%s\n", json);
	APE_CONFIG_FREE(json);

	/* Parse JSON back */
	ApeConfig config2;
	ape_config_init(&config2);
	if (ape_config_parse(&config2, json)) {
		const char *name = ape_config_get_string(&config2, "project.name", "Unknown");
		int64_t version = ape_config_get_int(&config2, "project.version", 0);
		int debug = ape_config_get_bool(&config2, "project.debug", 0);

		printf("Parsed config:\n");
		printf("  Name: %s\n", name);
		printf("  Version: %lld\n", (long long)version);
		printf("  Debug: %d\n", debug);
	}

	ape_config_cleanup(&config2);
	ape_config_cleanup(&config);

	/* Cleanup */
	ape_ctx_cleanup(&ctx);

	return 0;
}