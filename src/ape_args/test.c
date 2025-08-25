#include "ape_args_api.h"

int main(int argc, char **argv)
{
	ape_args_parsed_args parsed = { 0 };
	int result = ape_args_parse_args(
		&(ape_args_parse_opts){
			.argc = &argc,
			.argv = &argv,
			.stop_at = "--",
			.ignore_first_arg = 1,
			.mode = APE_ARGS_ALLOW_POSITIONAL | APE_ARGS_ALLOW_DASH | APE_ARGS_ALLOW_DASH_VAL | APE_ARGS_ALLOW_DASH_EQ,
		},
		&parsed);
	if (result == -1) {
		fprintf(stderr, "Encountered parsing error\n");
		return 1;
	}
	for (int i = 0; i < parsed.positional_count; i++) {
		printf("positional[%d]: %s\n", i, parsed.positional[i]);
	}
	for (int i = 0; i < parsed.map.key_count; i++) {
		printf("%s => %s\n", parsed.map.iterable[i].key, parsed.map.array[parsed.map.iterable[i].index].value);
	}
	return 0;
}
