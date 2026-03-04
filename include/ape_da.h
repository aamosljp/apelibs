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

#ifndef APE_DA_INCLUDED
#define APE_DA_INCLUDED

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define APE_DA_WINDOWS
#if defined(_WIN64)
#define APE_DA_WINDOWS_X64
#endif
#elif defined(__linux__) || defined(__unix__)
#define APE_DA_LINUX
#elif defined(__APPLE__)
#define APE_DA_APPLE
#endif

#ifndef APE_DA_MALLOC
#if defined(APE_DA_REALLOC) || defined(APE_DA_FREE)
#pragma GCC error "Need to define MALLOC, REALLOC and FREE or none of them"
#else
#include <stdlib.h>
#define APE_DA_MALLOC malloc
#define APE_DA_REALLOC realloc
#define APE_DA_FREE free
#endif
#else
#if !defined(APE_DA_REALLOC) || !defined(APE_DA_FREE)
#pragma GCC error "Need to define MALLOC, REALLOC and FREE or none of them"
#endif
#endif

#ifndef APE_DA_ASSERT
#ifdef APE_DA_USE_STDLIB_ASSERT
#include <assert.h>
#define APE_DA_ASSERT(c) assert(c)
#else
#include <stdio.h>
#include <stdlib.h>
#define APE_DA_ASSERT(c)                                                                   \
	if (!(c)) {                                                                        \
		fprintf(stderr, "%s:%d Assertion '%s' failed\n", __FILE__, __LINE__, ##c); \
		exit(1);                                                                   \
	}
#endif
#endif

#if defined(__cplusplus)
template <class T> static T *ape_da_growf_wrapper(T *da, size_t n, size_t growby, size_t min_cap) {
	return (T *)ape_da_growf((void *)da, n, growby, min_cap);
}
extern "C" {
#endif

extern void *ape_da_growf(void *da, size_t n, size_t growby, size_t min_cap);

#include <stddef.h>
#include <string.h>

#define APE_DA_INIT_CAP 256

typedef struct {
	size_t capacity;
	size_t count;
} ApeDaHeader;

#define ape_da_header(da) ((ApeDaHeader *)(da) - 1)

#define ape_da_grow(da, n, min_cap) (ape_da_growf_wrapper(da, sizeof(*(da)), n, min_cap))
#define ape_da_maybegrow(da, n) ((!(da) || ape_da_header(da)->count + (n) > ape_da_header(da)->capacity) ? (ape_da_grow(da, n, 0), 0) : 0)
#define ape_da_setcap(da, n) (ape_da_grow(da, 0, n))
#define ape_da_setlen(da, n) ((ape_da_cap(da) < n ? ape_da_setcap(da, n), 0 : 0), (da) ? ape_da_header(da)->count = (n) : 0)
#define ape_da_cap(da) ((da) ? ape_da_header(da)->capacity : 0)
#define ape_da_len(da) ((da) ? (ptrdiff_t)ape_da_header(da)->count : 0)
#define ape_da_push(da, x) (ape_da_maybegrow(da, 1), (da)[ape_da_header(da)->count++] = (x))
#define ape_da_addn(da, n) (ape_da_maybegrow(da, n), ape_da_header(da)->count += (n))
#define ape_da_last(da) ((da)[ape_da_len(da) - 1])
#define ape_da_free(da) ((void)((da) ? APE_DA_REALLOC(ape_da_header(da), 0) : 0), (da) = NULL)
#define ape_da_deln(da, i, n) \
	(memmove(&(a)[i], &(da)[(i) + (n)], sizeof(*(da)) * (ape_da_header(da)->count - (n) - (i))), ape_da_header(da)->count -= (n))
#define ape_da_del(da, i) ape_da_deln(da, i, 1)
#define ape_da_delswap(da, i) ((da)[i] = ape_da_last(da), ape_da_header(da)->count--)
#define ape_da_insn(da, i, n) \
	(ape_da_addn((da), (n)), memmove(&(da)[(i) + (n)], &(da)[i], sizeof(*(da)) * (ape_da_header(da)->count - (n) - (i))))
#define ape_da_ins(da, i, v) (ape_da_insn((a), (i), 1), (da)[i] = (v))

#if defined(__cplusplus)
}
#endif

#if defined(APE_DA_STRIP_PREFIX)

#endif

#endif

#ifdef APE_DA_IMPLEMENTATION

#ifndef APE_DA_IMPLEMENTATION_INCLUDED
#define APE_DA_IMPLEMENTATION_INCLUDED

/* User can define a custom function prefix (eg. static) */
#ifndef APE_DA_DEF
#define APE_DA_DEF
#endif

/* Can also define custom private function prefix */
#ifndef APE_DA_PRIVATE
#define APE_DA_PRIVATE static
#endif

#ifndef APE_DA_TRUE
#define APE_DA_TRUE (1)
#define APE_DA_FALSE (0)
#else
#if !defined(APE_DA_FALSE)
#pragma GCC error "Need to define both APE_DA_TRUE and APE_DA_FALSE or neither"
#endif
#endif

/* BEGIN ape_da_da.c */

void *ape_da_growf(void *da, size_t n, size_t growby, size_t min_cap) {
	size_t min_len = ape_da_len(da) + n;
	if (min_len > min_cap) min_cap = min_len;
	if (min_cap <= min_len) return da;
	if (min_cap < 2 * ape_da_cap(da))
		min_cap = 2 * ape_da_cap(da);
	else if (min_cap < 4)
		min_cap = 4;
	void *new_da = APE_DA_REALLOC((da) ? ape_da_header(da) : 0, n * min_cap + sizeof(ApeDaHeader));
	new_da = (char *)new_da + sizeof(ApeDaHeader);
	if (da == NULL) { ape_da_header(new_da)->count = 0; }
	ape_da_header(new_da)->capacity = min_cap;
	return new_da;
}
/* END ape_da_da.c */

#endif

#endif
