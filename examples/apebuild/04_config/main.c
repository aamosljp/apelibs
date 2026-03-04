#include <stdio.h>
#include "../../../include/ape_config.h"

int main() {
	printf("Config example program\n");

	/* Create a simple config */
	ApeConfig config;
	ape_config_init(&config);

	ape_config_set_string(&config, "message", "Hello from config!");
	ape_config_set_int(&config, "code", 42);

	/* Get values */
	const char *message = ape_config_get_string(&config, "message", "Default message");
	int64_t code = ape_config_get_int(&config, "code", 0);

	printf("%s (code: %lld)\n", message, (long long)code);

	/* Write as JSON */
	char *json = ape_config_write(&config);
	printf("Config as JSON: %s\n", json);
	APE_CONFIG_FREE(json);

	ape_config_cleanup(&config);

	return 0;
}