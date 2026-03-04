/*
 * This is free and unencumbered software released into the public domain.
 * 
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 * 
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 * 
 * For more information, please refer to <https://unlicense.org>
 */

/*
 * ape_args.h - APE Argument Parsing Library
 * -----------------------------------
 * Author: aamosljp (https://github.com/aamosljp)
 * Original Release Date: 2025-08-24
 * Current Version: 0.1.0
 *
 * License:
 *   This library is released into the public domain.
 *   You may use it for any purpose, commercial or private, without restriction.
 *
 * Description:
 *   ape_args is a lightweight (2 functions) argument parsing library.
 *   It supports multiple togglable syntaxes and also positional arguments
 *
 * Usage Example:
 *   #define APE_ARGS_IMPLEMENTATION
 *   #include "ape_args.h"
 *
 *   int main()
 *   {
 *       ape_args_parsed_args parsed = { 0 };
 *       int result = ape_args_parse_args(
 *       	&(ape_args_parse_opts){
 *       		.argc = &argc,
 *       		.argv = &argv,
 *       		.stop_at = "--",
 *       		.ignore_first_arg = 1,
 *       		.mode = APE_ARGS_ALLOW_POSITIONAL | APE_ARGS_ALLOW_DASH | APE_ARGS_ALLOW_DASH_VAL | APE_ARGS_ALLOW_DASH_EQ,
 *       	},
 *       	&parsed);
 *       if (result == -1) {
 *       	fprintf(stderr, "Encountered parsing error\n");
 *       	return 1;
 *       }
 *       for (int i = 0; i < parsed.positional_count; i++) {
 *       	printf("positional[%d]: %s\n", i, parsed.positional[i]);
 *       }
 *       for (int i = 0; i < parsed.map.key_count; i++) {
 *       	printf("%s => %s\n", parsed.map.iterable[i].key, parsed.map.array[parsed.map.iterable[i].index].value);
 *       }
 *       return 0;
 *   }
 *
 * Optional Macros (define before APE_LINE_IMPLEMENTATION):
 *   APE_ARGS_MALLOC              // Custom malloc replacement
 *   APE_ARGS_REALLOC             // Custom realloc replacement
 *   APE_ARGS_FREE                // Custom free replacement
 *   APE_ARGS_STRIP_PREFIX        // Strip prefix from exported symbols
 *   APE_ARGS_HASHMAP_MAX_LEN     // Internal array length for hashmaps
 *   APE_ARGS_HASHMAP             // Your own custom hashmap implementation (look at the builtin implementation for details)
 *   APE_ARGS_HASHMAP_SET_KEY     // Set key function/macro for your custom impl
 *   APE_ARGS_HASHMAP_GET_KEY     // Get key function/macro for your custom impl
 *
 * API Reference:
 *   char *ape_args_shift_args(int *argc, char ***argv);
 *      - shift args and return the first argument
 *   int ape_args_parse_args(const ape_args_parse_opts *opts, ape_args_parsed_args *parsed);
 *      - parse arguments based on options in opts
 *
 * Version History:
 *   0.1.0 2025-08-24 - Initial release.
 */
#ifndef APE_ARGS_INCLUDED
#define APE_ARGS_INCLUDED

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define APE_ARGS_WINDOWS
#if defined(_WIN64)
#define APE_ARGS_WINDOWS_X64
#endif
#elif defined(__linux__) || defined(__unix__)
#define APE_ARGS_LINUX
#elif defined(__APPLE__)
#define APE_ARGS_APPLE
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdio.h>

#ifndef APE_ARGS_MALLOC
#if defined(APE_ARGS_REALLOC) || defined(APE_ARGS_FREE)
#pragma GCC error "Need to define MALLOC, REALLOC and FREE or none of them"
#else
#define APE_ARGS_MALLOC malloc
#define APE_ARGS_REALLOC realloc
#define APE_ARGS_FREE free
#endif
#else
#if !defined(APE_ARGS_REALLOC) || !defined(APE_ARGS_FREE)
#pragma GCC error "Need to define MALLOC, REALLOC and FREE or none of them"
#endif
#endif

#ifndef APE_ARGS_HASHMAP_MAX_LEN
#define APE_ARGS_HASHMAP_MAX_LEN 65536
#endif

#ifndef APE_ARGS_HASH_FUNCTION
#define APE_ARGS_HASH_FUNCTION ape_args_hash_builtin
#endif

#ifndef APE_ARGS_HASHMAP
#if defined(APE_ARGS_HASHMAP_SET_KEY)
#pragma GCC error "APE_ARGS_HASHMAP_SET_KEY should not be defined"
#endif
#if defined(APE_ARGS_HASHMAP_GET_KEY)
#pragma GCC error "APE_ARGS_HASHMAP_GET_KEY should not be defined"
#endif

#define APE_ARGS_HASHMAP(kt, vt)                      \
	struct {                                      \
		unsigned int key_count;               \
		struct {                              \
			kt key;                       \
			unsigned int index;           \
		} iterable[APE_ARGS_HASHMAP_MAX_LEN]; \
		struct {                              \
			kt key;                       \
			vt value;                     \
			int is_set;                   \
		} array[APE_ARGS_HASHMAP_MAX_LEN];    \
	}

#define APE_ARGS_HASHMAP_SET_KEY(hm, k, v)                                                                                     \
	do {                                                                                                                   \
		if ((hm)->key_count >= APE_ARGS_HASHMAP_MAX_LEN) { fprintf(stderr, "ERROR: Too many elements in hashmap\n"); } \
		unsigned int index = APE_ARGS_HASH_FUNCTION(k);                                                                \
		if ((hm)->array[index].is_set && memcmp((hm)->array[index].key, (k), strlen(k)) != 0) {                        \
			fprintf(stderr, "ERROR: Conflicting keys, not assigning\n");                                           \
		} else if ((hm)->array[index].is_set && memcmp((hm)->array[index].key, (k), strlen(k)) == 0) {                 \
			free((hm)->array[index].value);                                                                        \
			(hm)->array[index].value = malloc(strlen(v) + 1);                                                      \
			memcpy((hm)->array[index].value, (v), strlen(v));                                                      \
			(hm)->array[index].value[strlen(v)] = 0;                                                               \
		} else {                                                                                                       \
			(hm)->array[index].key = malloc(strlen(k) + 1);                                                        \
			(hm)->array[index].value = malloc(strlen(v) + 1);                                                      \
			memcpy((hm)->array[index].key, (k), strlen(k));                                                        \
			memcpy((hm)->array[index].value, (v), strlen(v));                                                      \
			(hm)->array[index].key[strlen(k)] = 0;                                                                 \
			(hm)->array[index].value[strlen(v)] = 0;                                                               \
			(hm)->array[index].is_set = 1;                                                                         \
			(hm)->iterable[(hm)->key_count].key = malloc(strlen(k) + 1);                                           \
			memcpy((hm)->iterable[(hm)->key_count].key, (k), strlen(k));                                           \
			(hm)->iterable[(hm)->key_count].key[strlen(k)] = 0;                                                    \
			(hm)->iterable[(hm)->key_count].index = index;                                                         \
			(hm)->key_count++;                                                                                     \
		}                                                                                                              \
	} while (0)

#define APE_ARGS_HASHMAP_GET_KEY(hm, k)                                                                                           \
	(hm)->array[APE_ARGS_HASH_FUNCTION(k)].is_set &&memcmp((hm)->array[APE_ARGS_HASH_FUNCTION(k)].key, (k), strlen(k)) == 0 ? \
		(hm)->array[APE_ARGS_HASH_FUNCTION(k)].value :                                                                    \
		NULL

#else
#if !defined(APE_ARGS_HASHMAP_SET_KEY)
#pragma GCC error "APE_ARGS_HASHMAP_SET_KEY should be defined"
#endif
#if !defined(APE_ARGS_HASHMAP_GET_KEY)
#pragma GCC error "APE_ARGS_HASHMAP_GET_KEY should be defined"
#endif
#endif

#if defined(__cplusplus)
}
#endif

#include <string.h>

typedef enum {
	APE_ARGS_ALLOW_DASH = 1 << 0,	     // --key
	APE_ARGS_ALLOW_DASH_EQ = 1 << 1,     // --key=value
	APE_ARGS_ALLOW_DASH_VAL = 1 << 2,    // --key value
	APE_ARGS_ALLOW_EQ = 1 << 3,	     // key=value
	APE_ARGS_ALLOW_POSITIONAL = 1 << 4,  // value (stored in positional array)
	APE_ARGS_ALLOW_SINGLE_DASH = 1 << 5, // any single '-' is interpreted as '--'
} ape_args_parse_mode;

typedef struct {
	// argc and argv passed to the parse function
	// these are usually just pointers to whatever was passed to your main function
	int *argc;
	char ***argv;

	char *stop_at; // stop parsing when this string is given as argument (default: NULL)
	// this is usually "--" on unix

	int ignore_first_arg; // you should turn this on if you don't want the executable name to be parsed

	int allow_positional_anywhere; // allow (-|--).* args between positional args

	ape_args_parse_mode mode; // if 0, everything is allowed
} ape_args_parse_opts;

typedef struct {
	unsigned int positional_count;
	char **positional;
	APE_ARGS_HASHMAP(char *, char *) map;
} ape_args_parsed_args;

int ape_args_parse_args(const ape_args_parse_opts *opts, ape_args_parsed_args *parsed);
char *ape_args_shift_args(int *argc, char ***argv);

#if defined(APE_ARGS_STRIP_PREFIX)

#define args_parse_mode ape_args_parse_mode
#define args_parse_opts ape_args_parse_opts
#define args_parsed_args ape_args_parsed_args
#define args_parse_args ape_args_parse_args
#define args_shift_args ape_args_shift_args

#endif

#endif

#ifdef APE_ARGS_IMPLEMENTATION

#ifndef APE_ARGS_IMPLEMENTATION_INCLUDED
#define APE_ARGS_IMPLEMENTATION_INCLUDED

/* User can define a custom function prefix (eg. static) */
#ifndef APE_ARGS_DEF
#define APE_ARGS_DEF
#endif

/* Can also define custom private function prefix */
#ifndef APE_ARGS_PRIVATE
#define APE_ARGS_PRIVATE static
#endif

#ifndef APE_ARGS_TRUE
#define APE_ARGS_TRUE (1)
#define APE_ARGS_FALSE (0)
#else
#if !defined(APE_ARGS_FALSE)
#pragma GCC error "Need to define both APE_ARGS_TRUE and APE_ARGS_FALSE or neither"
#endif
#endif

APE_ARGS_PRIVATE int ape_args_hash_builtin(char *key) {
	int h = 0;
	char *c = key;
#define _T(i) ((((i * 1252225 + 12854 / 12535) << 4 / 294) >> 1) - 6) % APE_ARGS_HASHMAP_MAX_LEN
	while (*c) {
		h = _T(h ^ *c);
		c++;
	}
#undef _T
	return h;
}

#include <string.h>
#include <stdlib.h>
#include "ape_args_api.h"

/* BEGIN ape_args_main.c */

APE_ARGS_DEF inline char *ape_args_shift_args(int *argc, char ***argv) {
	if ((*argc) <= 0) return NULL;
	(*argc)--;
	char *result = (*argv)[0];
	(*argv)++;
	return result;
}

APE_ARGS_DEF int ape_args_parse_args(const ape_args_parse_opts *opts, ape_args_parsed_args *parsed) {
	ape_args_parse_mode mode = opts->mode;
	if (mode == 0) {
		mode = APE_ARGS_ALLOW_DASH | APE_ARGS_ALLOW_DASH_EQ | APE_ARGS_ALLOW_DASH_VAL | APE_ARGS_ALLOW_SINGLE_DASH |
		       APE_ARGS_ALLOW_POSITIONAL | APE_ARGS_ALLOW_EQ;
	}
	int positional_capacity = 2;
	parsed->positional = (char **)APE_ARGS_MALLOC(positional_capacity * sizeof(char *));
	int allow_positional = 1;
	char *arg;
	if (opts->ignore_first_arg) arg = ape_args_shift_args(opts->argc, opts->argv);
	while ((arg = ape_args_shift_args(opts->argc, opts->argv))) {
		if (opts->stop_at && strcmp(opts->stop_at, arg) == 0) break;
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
							(char **)APE_ARGS_REALLOC(parsed->positional, positional_capacity * sizeof(char *));
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
/* END ape_args_main.c */

#endif

#endif
