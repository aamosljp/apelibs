#include "ape_args_api.h"
#include <stdlib.h>

#include "ape_args_internal.h"
#include <string.h>

APE_ARGS_DEF inline char *ape_args_shift_args(int *argc, char ***argv)
{
	if ((*argc) <= 0)
		return NULL;
	(*argc)--;
	char *result = (*argv)[0];
	(*argv)++;
	return result;
}

APE_ARGS_DEF int ape_args_parse_args(const ape_args_parse_opts *opts, ape_args_parsed_args *parsed)
{
	ape_args_parse_mode mode = opts->mode;
	if (mode == 0) {
		mode = APE_ARGS_ALLOW_DASH | APE_ARGS_ALLOW_DASH_EQ | APE_ARGS_ALLOW_DASH_VAL | APE_ARGS_ALLOW_SINGLE_DASH |
		       APE_ARGS_ALLOW_POSITIONAL | APE_ARGS_ALLOW_EQ;
	}
	int positional_capacity = 2;
	parsed->positional = (char **)malloc(positional_capacity * sizeof(char *));
	int allow_positional = 1;
	char *arg;
	if (opts->ignore_first_arg)
		arg = ape_args_shift_args(opts->argc, opts->argv);
	while ((arg = ape_args_shift_args(opts->argc, opts->argv))) {
		if (opts->stop_at && strcmp(opts->stop_at, arg) == 0)
			break;
		char *eq;
		int ap = 0;
		if (!ap && (mode & (APE_ARGS_ALLOW_DASH | APE_ARGS_ALLOW_DASH_VAL | APE_ARGS_ALLOW_DASH_EQ))) {
			if (strncmp(arg, "-", 1) == 0) {
				int ddash = strncmp(arg, "--", 2) == 0;
				if (!(mode & APE_ARGS_ALLOW_SINGLE_DASH) & !ddash) {
					fprintf(stderr, "Need '--' instead of a single '-'\n");
					return -1;
				}
				char *act = arg + (ddash ? 2 : 1);
				if (!ap && mode & APE_ARGS_ALLOW_DASH_EQ) {
					if (strchr(act, '=') != NULL) {
						if (strchr(act, '=') != act) {
							int n = strlen(act);
							char *key = strtok(act, "=");
							char *val = strtok(NULL, "=");
							if (!val) {
								fprintf(stderr, "Need value after '='\n");
								return -1;
							}
							if (strlen(key) + strlen(val) + 1 != n) {
								fprintf(stderr, "Cannot have more than one '=' in arg\n");
								return -1;
							}
							APE_ARGS_HASHMAP_SET_KEY(&parsed->map, key, val);
							allow_positional = 0;
							ap = 1;
						} else {
							fprintf(stderr, "Need key before '='\n");
							return -1;
						}
					}
				}
				if (!ap && mode & APE_ARGS_ALLOW_DASH) {
					if (!strchr(act, '=')) {
						if (mode & APE_ARGS_ALLOW_DASH_VAL) {
							int old_argc = *(opts->argc);
							char **old_argv = *(opts->argv);
							char *narg = ape_args_shift_args(opts->argc, opts->argv);
							if (!narg || strncmp(narg, "-", 1) == 0) {
								*(opts->argc) = old_argc;
								*(opts->argv) = old_argv;
							} else {
								APE_ARGS_HASHMAP_SET_KEY(&parsed->map, act, narg);
								allow_positional = 0;
								ap = 1;
							}
						}
						if (!ap) {
							APE_ARGS_HASHMAP_SET_KEY(&parsed->map, act, "true");
							allow_positional = 0;
							ap = 1;
						}
					} else {
						fprintf(stderr, "Cannot have equal-sign in argument");
						return -1;
					}
				}
			}
		}
		if (!ap && (mode & (APE_ARGS_ALLOW_EQ | APE_ARGS_ALLOW_POSITIONAL))) {
			if (strncmp(arg, "-", 1) == 0) {
				fprintf(stderr, "Cannot start with '-'\n");
				return -1;
			}
			if (!ap && (mode & APE_ARGS_ALLOW_EQ)) {
				if (strchr(arg, '=') != NULL) {
					int n = strlen(arg);
					if (strchr(arg, '=') == arg) {
						fprintf(stderr, "Need key before '='\n");
						return -1;
					}
					char *key = strtok(arg, "=");
					char *val = strtok(NULL, "=");
					if (!val) {
						fprintf(stderr, "Need value after '='\n");
						return -1;
					}
					if (strlen(key) + strlen(val) + 1 != n) {
						fprintf(stderr, "Cannot have more than one '='\n");
						return -1;
					}
					APE_ARGS_HASHMAP_SET_KEY(&parsed->map, key, val);
					allow_positional = 0;
					ap = 1;
				}
			}
			if (!ap && (mode & APE_ARGS_ALLOW_POSITIONAL)) {
				if (allow_positional || opts->allow_positional_anywhere) {
					if (strchr(arg, '=') != NULL) {
						fprintf(stderr, "Arg cannot contain '='\n");
						return -1;
					}
					if (parsed->positional_count + 1 >= positional_capacity) {
						positional_capacity *= 2;
						parsed->positional =
							(char **)realloc(parsed->positional, positional_capacity * sizeof(char *));
					}
					parsed->positional[parsed->positional_count++] = strdup(arg);
					ap = 1;
				} else {
					fprintf(stderr, "Positional arguments should be put before other arguments\n");
					return -1;
				}
			}
		}
		if (!ap) {
			fprintf(stderr, "Couldn't parse argument '%s'\n", arg);
			return -1;
		}
	}
	return *opts->argc;
}
