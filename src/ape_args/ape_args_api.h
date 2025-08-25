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

#define APE_ARGS_HASHMAP_SET_KEY(hm, k, v)                                                                     \
	do {                                                                                                   \
		if ((hm)->key_count >= APE_ARGS_HASHMAP_MAX_LEN) {                                             \
			fprintf(stderr, "ERROR: Too many elements in hashmap\n");                              \
		}                                                                                              \
		unsigned int index = APE_ARGS_HASH_FUNCTION(k);                                                \
		if ((hm)->array[index].is_set && memcmp((hm)->array[index].key, (k), strlen(k)) != 0) {        \
			fprintf(stderr, "ERROR: Conflicting keys, not assigning\n");                           \
		} else if ((hm)->array[index].is_set && memcmp((hm)->array[index].key, (k), strlen(k)) == 0) { \
			free((hm)->array[index].value);                                                        \
			(hm)->array[index].value = malloc(strlen(v) + 1);                                      \
			memcpy((hm)->array[index].value, (v), strlen(v));                                      \
			(hm)->array[index].value[strlen(v)] = 0;                                               \
		} else {                                                                                       \
			(hm)->array[index].key = malloc(strlen(k) + 1);                                        \
			(hm)->array[index].value = malloc(strlen(v) + 1);                                      \
			memcpy((hm)->array[index].key, (k), strlen(k));                                        \
			memcpy((hm)->array[index].value, (v), strlen(v));                                      \
			(hm)->array[index].key[strlen(k)] = 0;                                                 \
			(hm)->array[index].value[strlen(v)] = 0;                                               \
			(hm)->array[index].is_set = 1;                                                         \
			(hm)->iterable[(hm)->key_count].key = malloc(strlen(k) + 1);                           \
			memcpy((hm)->iterable[(hm)->key_count].key, (k), strlen(k));                           \
			(hm)->iterable[(hm)->key_count].key[strlen(k)] = 0;                                    \
			(hm)->iterable[(hm)->key_count].index = index;                                         \
			(hm)->key_count++;                                                                     \
		}                                                                                              \
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
	APE_ARGS_ALLOW_DASH = 1 << 0, // --key
	APE_ARGS_ALLOW_DASH_EQ = 1 << 1, // --key=value
	APE_ARGS_ALLOW_DASH_VAL = 1 << 2, // --key value
	APE_ARGS_ALLOW_EQ = 1 << 3, // key=value
	APE_ARGS_ALLOW_POSITIONAL = 1 << 4, // value (stored in positional array)
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
