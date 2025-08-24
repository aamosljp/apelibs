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
 *
 * Roadmap (roughly in order of importance):
 *
 * Usage Example:
 *   #define APE_ARGS_IMPLEMENTATION
 *   #include "ape_args.h"
 *
 *   int main()
 *   {
 *   }
 *
 * Optional Macros (define before APE_LINE_IMPLEMENTATION):
 *   APE_ARGS_MALLOC              // Custom malloc replacement
 *   APE_ARGS_REALLOC             // Custom realloc replacement
 *   APE_ARGS_FREE                // Custom free replacement
 *   APE_ARGS_STRIP_PREFIX        // Strip prefix from exported symbols
 *
 * API Reference:
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

#define APE_ARGS_HASHMAP_SET_KEY(hm, k, v)                                                              \
	do {                                                                                            \
		if ((hm)->key_count >= APE_ARGS_HASHMAP_MAX_LEN) {                                      \
			fprintf(stderr, "ERROR: Too many elements in hashmap\n");                       \
		}                                                                                       \
		unsigned int index = APE_ARGS_HASH_FUNCTION(k);                                         \
		if ((hm)->array[index].is_set && memcmp((hm)->array[index].key, (k), sizeof(k)) != 0) { \
			fprinf(stderr, "ERROR: Conflicting keys. Not assigning\n");                     \
		} else {                                                                                \
			(hm)->array[index].key = malloc(sizeof(k));                                     \
			(hm)->array[index].value = malloc(sizeof(v));                                   \
			memcpy((hm)->array[index].key, (k), sizeof(k));                                 \
			memcpy((hm)->array[index].value, (v), sizeof(v));                               \
			(hm)->array[index].is_set = 1;                                                  \
			(hm)->iterable[(hm)->key_count].key = malloc(sizeof(k));                        \
			memcpy((hm)->iterable[(hm)->key_count].key, (k), sizeof(k));                    \
			(hm)->iterable[(hm)->key_count].index = index;                                  \
			(hm)->key_count++;                                                              \
		}                                                                                       \
	} while (0)

#define APE_ARGS_HASHMAP_GET_KEY(hm, k)                                                                                           \
	(hm)->array[APE_ARGS_HASH_FUNCTION(k)].is_set &&memcmp((hm)->array[APE_ARGS_HASH_FUNCTION(k)].key, (k), sizeof(k)) == 0 ? \
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

typedef struct {
	// argc and argv passed to the parse function
	// these are usually just pointers to whatever was passed to your main function
	int *argc;
	char ***argv;

	char *stop_at; // stop parsing when this string is given as argument (default: NULL)
	// this is usually "--" on unix

	char *default_key; // store values in this list if no key is present (default: "default")
} ape_args_parse_opts;

typedef struct {
	unsigned int positional_count;
	char **positional;
	APE_ARGS_HASHMAP(char *, char *) map;
} ape_args_parsed_args;

int ape_args_parse_args(const ape_args_parse_opts *opts);
char *ape_args_shift_args(int *argc, char ***argv);

#if defined(APE_ARGS_STRIP_PREFIX)

#define parse_opts ape_args_parse_opts
#define parse_args ape_args_parse_args
#define shift_args ape_args_shift_args

#endif

#endif
