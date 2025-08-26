#ifndef TEMPLATE_INCLUDED
#define TEMPLATE_INCLUDED

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define TEMPLATE_WINDOWS
#if defined(_WIN64)
#define TEMPLATE_WINDOWS_X64
#endif
#elif defined(__linux__) || defined(__unix__)
#define TEMPLATE_LINUX
#elif defined(__APPLE__)
#define TEMPLATE_APPLE
#endif

#ifndef TEMPLATE_MALLOC
#if defined(TEMPLATE_REALLOC) || defined(TEMPLATE_FREE)
#pragma GCC error "Need to define MALLOC, REALLOC and FREE or none of them"
#else
#include <stdlib.h>
#define TEMPLATE_MALLOC malloc
#define TEMPLATE_REALLOC realloc
#define TEMPLATE_FREE free
#endif
#else
#if !defined(TEMPLATE_REALLOC) || !defined(TEMPLATE_FREE)
#pragma GCC error "Need to define MALLOC, REALLOC and FREE or none of them"
#endif
#endif

#ifndef TEMPLATE_ASSERT
#ifdef TEMPLATE_USE_STDLIB_ASSERT
#include <assert.h>
#define TEMPLATE_ASSERT(c) assert(c)
#else
#include <stdio.h>
#include <stdlib.h>
#define TEMPLATE_ASSERT(c)                                                                 \
	if (!(c)) {                                                                        \
		fprintf(stderr, "%s:%d Assertion '%s' failed\n", __FILE__, __LINE__, ##c); \
		exit(1);                                                                   \
	}
#endif
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(__cplusplus)
}
#endif

#if defined(TEMPLATE_STRIP_PREFIX)

#endif

#endif
