#ifndef APE_GLOB_INCLUDED
#define APE_GLOB_INCLUDED

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define APE_GLOB_WINDOWS
#if defined(_WIN64)
#define APE_GLOB_WINDOWS_X64
#endif
#elif defined(__linux__) || defined(__unix__)
#define APE_GLOB_LINUX
#elif defined(__APPLE__)
#define APE_GLOB_APPLE
#endif

#ifndef APE_GLOB_MALLOC
#if defined(APE_GLOB_REALLOC) || defined(APE_GLOB_FREE)
#pragma GCC error "Need to define MALLOC, REALLOC and FREE or none of them"
#else
#include <stdlib.h>
#define APE_GLOB_MALLOC malloc
#define APE_GLOB_REALLOC realloc
#define APE_GLOB_FREE free
#endif
#else
#if !defined(APE_GLOB_REALLOC) || !defined(APE_GLOB_FREE)
#pragma GCC error "Need to define MALLOC, REALLOC and FREE or none of them"
#endif
#endif

#ifndef APE_GLOB_ASSERT
#ifdef APE_GLOB_USE_STDLIB_ASSERT
#include <assert.h>
#define APE_GLOB_ASSERT(c) assert(c)
#else
#include <stdio.h>
#include <stdlib.h>
#define APE_GLOB_ASSERT(c)                                                                 \
	if (!(c)) {                                                                        \
		fprintf(stderr, "%s:%d Assertion '%s' failed\n", __FILE__, __LINE__, ##c); \
		exit(1);                                                                   \
	}
#endif
#endif

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum {
	APEGLOB_FLAG_CASE_INSENSITIVE = 1u << 0,
	APEGLOB_FLAG_DOT_LITERAL = 1u << 1,
	APEGLOB_FLAG_GLOBSTAR = 1u << 2,
	APEGLOB_FLAG_FOLLOW_SYMLINKS = 1u << 3,
} apeglob_glob_flags;

typedef bool (*apeglob_match_fn)(const char *pattern, const char *text, unsigned flags);

typedef struct {
	char **items;
	unsigned int count;
	unsigned int capacity;
} apeglob_list;

int apeglob_glob(const char *pattern, unsigned flags, apeglob_match_fn match, apeglob_list *out);
void apeglob_free(apeglob_list *list);

// Simple built-in matcher (* ? []), byte-wise, case sensitive by default
bool apeglob_match_simple(const char *pat, const char *txt, unsigned flags);

#if defined(__cplusplus)
}
#endif

#if defined(APE_GLOB_STRIP_PREFIX)

#endif

#endif

#ifdef APE_GLOB_IMPLEMENTATION

#ifndef APE_GLOB_IMPLEMENTATION_INCLUDED
#define APE_GLOB_IMPLEMENTATION_INCLUDED

/* User can define a custom function prefix (eg. static) */
#ifndef APE_GLOB_DEF
#define APE_GLOB_DEF
#endif

/* Can also define custom private function prefix */
#ifndef APE_GLOB_PRIVATE
#define APE_GLOB_PRIVATE static
#endif

#ifndef APE_GLOB_TRUE
#define APE_GLOB_TRUE (1)
#define APE_GLOB_FALSE (0)
#else
#if !defined(APE_GLOB_FALSE)
#pragma GCC error "Need to define both APE_GLOB_TRUE and APE_GLOB_FALSE or neither"
#endif
#endif

/* BEGIN ape_glob_main.c */
/* END ape_glob_main.c */

#endif

#endif
