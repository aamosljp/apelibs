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
 * apedsa.h - v0.2
 *
 * 
 *   #define APEDSA_IMPLEMENTATION
 *   #include "apedsa.h"
 * 
 * **** Dynamic Array ****
 * 
 * Sample code for dynamic array usage:
 * 
 *   int main(void) {
 *       int *arr = NULL;
 *       apedsa_da_push(arr, 42);
 *       apedsa_da_push(arr, 1337);
 *       apedsa_da_push(arr, 420);
 *       for (int i = 0; i < apedsa_da_count(arr); i++) {
 *           printf("%d\n", arr[i]);
 *       }
 *   }
 * 
 * Dynamic array usage:
 *   apedsa_da_count - Returns the number of elements in da as an unsigned integer
 *   apedsa_da_cap - Returns the capacity of da
 *   apedsa_da_push - Pushes an element onto da
 *   apedsa_da_addn - Adds n uninitialized values to da
 *   apedsa_da_last - Returns the last element in da
 *   apedsa_da_insert - Inserts an element at index i
 *   apedsa_da_delete - Deletes an element at index i
 *   apedsa_da_delete_swap - Deletes an element at index i and replaces it with the last element
 *   apedsa_da_reserve - Reserves space for n elements in da
 * 
 * **** Hashmap ****
 * 
 * Sample code for hashmaps:
 * 
 *   int main(void) {
 *       struct kv { int key; int value; };
 *       struct kv *hm = NULL;
 *       apedsa_hm_put(hm, 111, 42);
 *       apedsa_hm_put(hm, 814, 11);
 *       apedsa_hm_put(hm, 420, 1337);
 *       printf("%d\n", apedsa_hm_get(hm, 420)); // should print "1337"
 *       for (int i = 0; i < apedsa_hm_len(hm), i++) {
 *           printf("%d: %d\n", hm[i].key, hm[i].value); // Can be indexed directly!
 *       } // This loop should print the following:
 *       // 111: 42
 *       // 814: 11
 *       // 420: 1337
 *       apedsa_hm_del(hm, 111);
 *       for (int i = 0; i < apedsa_hm_len(hm), i++) {
 *           printf("%d: %d\n", hm[i].key, hm[i].value);
 *       } // This loop should print the following:
 *       // 420: 1337
 *       // 814: 11
 *       // NOTE: Deletions don't preserve order
 *   }
 * 
 * Hashmap usage:
 *   apedsa_hm_len - Returns the number of elements in hm as an unsigned integer
 *   apedsa_hm_put - Insert value at key in hashmap
 *   apedsa_hm_get - Get value at key in hashmap
 *   apedsa_hm_del - Delete value at key in hashmap
 *   apedsa_hm_puts - Insert key-value pair (format: (struct pair){.key=key, .value=value})
 *   apedsa_hm_geti - Get index of key
 *   apedsa_hm_getp - Get pointer to key-value pair
 *   apedsa_hm_gets - Get key-value pair
 *   apedsa_hm_with_cap - Initialize hashmap with capacity (slightly reduces allocations on large hashmaps)
 *   apedsa_hm_put_batch - Batch insert key-value pairs from array into hashmap
 * 
 * For string-like keys (null-terminated), we provide a separate set of functions:
 *   apedsa_shm_len
 *   apedsa_shm_put
 *   apedsa_shm_get
 *   apedsa_shm_del
 *   apedsa_shm_puts
 *   apedsa_shm_geti
 *   apedsa_shm_getp
 *   apedsa_shm_gets
 *   apedsa_shm_with_cap
 *   apedsa_shm_put_batch
 * 
 */

#ifndef APEDSA_INCLUDED
#define APEDSA_INCLUDED

#define APEDSA_VERSION_MAJOR 0
#define APEDSA_VERSION_MINOR 2

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define APEDSA_WINDOWS
#if defined(_WIN64)
#define APEDSA_WINDOWS_X64
#endif
#elif defined(__linux__) || defined(__unix__)
#define APEDSA_LINUX
#elif defined(__APPLE__)
#define APEDSA_APPLE
#endif

#ifndef APEDSA_MALLOC
#if defined(APEDSA_REALLOC) || defined(APEDSA_FREE)
#pragma GCC error "Need to define MALLOC, REALLOC and FREE or none of them"
#else
#include <stdlib.h>
#define APEDSA_MALLOC malloc
#define APEDSA_REALLOC realloc
#define APEDSA_FREE(x) (void)((x) ? free(x) : (void)0)
#endif
#else
#if !defined(APEDSA_REALLOC) || !defined(APEDSA_FREE)
#pragma GCC error "Need to define MALLOC, REALLOC and FREE or none of them"
#endif
#endif

#ifndef APEDSA_ASSERT
#ifdef APEDSA_USE_STDLIB_ASSERT
#include <assert.h>
#define APEDSA_ASSERT(c) assert(c)
#else
#include <stdio.h>
#include <stdlib.h>
#define APEDSA_ASSERT(c) (void)(!(c) ? fprintf(stderr, "%s:%d Assertion '%s' failed\n", __FILE__, __LINE__, #c), exit(1) : (void)0);
#endif
#endif

#define APEDSA_UNUSED(x) (void)(x)

#include <stddef.h>
#include <stdbool.h>

#if defined(__cplusplus)
extern "C" {
#endif

/// Set random seed for hash
extern void apedsa_rand_seed(size_t seed);

// Hash function used internally, returns 128-bit hash into out
extern void apedsa_da_murmurhash3_128(const void *key, size_t len, size_t seed, void *out);
/// This one just calls murmurhash and discards the unused bits
extern size_t apedsa_hash_bytes(void *p, size_t len, size_t seed);
/// Hash a null-terminated string, also discards unused bits
extern size_t apedsa_hash_string(char *str, size_t seed);

// Simple string arena implementation
typedef struct ApedsaStringArena ApedsaStringArena;
extern char *apedsa_string_arena_alloc(ApedsaStringArena *arena, char *str);
extern void apedsa_string_arena_reset(ApedsaStringArena *arena);

// if you want to use custom hash functions
typedef size_t (*ApedsaHashBytesFn)(void *key, size_t key_size, size_t seed);
typedef size_t (*ApedsaHashStringFn)(char *key, size_t seed);
extern void apedsa_hashmap_set_hash_fns(void *a, ApedsaHashBytesFn bytes_fn, ApedsaHashStringFn string_fn);

////////
// Private implementation functions, should only be used internally
////////

extern void *__apedsa_hashmap_put_internal(void *a, void *key, size_t key_size, size_t kv_size, int mode);
extern void *__apedsa_hashmap_get_internal(void *a, void *key, size_t key_size, size_t kv_size, int mode);
extern void *__apedsa_hashmap_del_internal(void *a, void *key, size_t key_size, size_t kv_size, size_t koff, int mode);
extern void *__apedsa_hashmap_put_internal_batch(void *a, size_t count, void *pairs, size_t key_size, size_t kv_size, int mode);

extern void *__apedsa_da_growf(void *da, size_t esz, size_t growby, size_t min_cap);
extern void *__apedsa_hashmap_reserve_internal(void *a, size_t count, size_t kv_size);

#if defined(__cplusplus)
}
#endif

#if defined(__GNUC__) || defined(__clang__)
#define __APEDSA_HASH_TYPEOF
#ifdef __cplusplus
#define __APEDSA_HAS_LITERAL_ARRAY
#endif
#endif

#if !defined(__cplusplus)
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define __APEDSA_HAS_LITERAL_ARRAY
#endif
#endif

#if defined(__APEDSA_HAS_LITERAL_ARRAY) && defined(__APEDSA_HASH_TYPEOF)
#define APEDSA_ADDRESSOF(tv, v) ((typeof(tv)[1]){ v })
#else
#define APEDSA_ADDRESSOF(tv, v) &(v)
#endif

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define __APEDSA_HASH_GENERIC
#endif

#if !defined(APEDSA_HASHMAP_QUADRATIC_PROBING) && !defined(APEDSA_HASHMAP_LINEAR_PROBING) && !defined(APEDSA_HASHMAP_DOUBLE_HASHING)
#define APEDSA_HASHMAP_QUADRATIC_PROBING
#endif

#define APEDSA_OFFSETOF(v, f) ((char *)&(v)->f - (char *)(v))

#define APEDSA_CACHE_LINE_SIZE 64

#define apedsa_da_temp(da) (apedsa_da_header(da)->temp)
#define apedsa_da_header(da) ((ApedsaDaHeader *)(da) - 1)
#define apedsa_da_cap(da) ((da) ? apedsa_da_header(da)->capacity : 0)
#define apedsa_da_count(da) ((da) ? apedsa_da_header(da)->count : 0)
#define apedsa_da_put(da, x) (apedsa_da_reserve(da, 1), (da)[apedsa_da_header(da)->count++] = (x))
#define apedsa_da_push apedsa_da_put
#define apedsa_da_last(da) ((da)[apedsa_da_header(da)->count - 1])
#define apedsa_da_deleten(da, i, n) \
	(memmove(&(da)[i], &(da)[(i) + (n)], sizeof(*(da)) * (apedsa_da_header(da)->count - (n) - (i))), apedsa_da_header(da)->count -= (n))
#define apedsa_da_delete(da, i) apedsa_da_deleten(da, i, 1)
#define apedsa_da_delete_swap(da, i) ((da)[i] = apedsa_da_last(da), --apedsa_da_header(da)->count)
#define apedsa_da_addn(da, n) (apedsa_da_reserve(da, n), apedsa_da_header(da)->count += (n))
#define apedsa_da_insertn(da, i, n) \
	(apedsa_da_addn((da), (n)), memmove(&(da)[(i) + (n)], &(da)[i], sizeof(*(da)) * (apedsa_da_header(da)->count - (n) - (i))))
#define apedsa_da_insert(da, i, x) (apedsa_da_insertn(da, i, 1), (da)[i] = (x))
#define apedsa_da_grow(da, n, min_cap) ((da) = __apedsa_da_growf_wrapper((da), sizeof(*(da)), (n), (min_cap)))
#define apedsa_da_reserve(da, n) \
	((!(da) || apedsa_da_header(da)->count + (n) > apedsa_da_header(da)->capacity) ? (apedsa_da_grow(da, n, 0), 0) : 0)

#define apedsa_hm_put(t, k, v)                                                                                             \
	((t) = __apedsa_hashmap_put_internal_wrapper((t), APEDSA_ADDRESSOF((t)->key, (k)), sizeof((t)->key), sizeof(*(t)), \
						     APEDSA_HASHMAP_MODE_BINARY),                                          \
	 (t)[apedsa_da_temp((t))].key = (k), (t)[apedsa_da_temp((t))].value = (v))

#define apedsa_hm_puts(t, s)                                                                                                    \
	((t) = __apedsa_hashmap_put_internal_wrapper((t), &(s).key, sizeof((s).key), sizeof(*(t)), APEDSA_HASHMAP_MODE_BINARY), \
	 (t)[apedsa_da_temp(t)] = (s))

#define apedsa_hm_geti(t, k)                                                                                                       \
	((t) = __apedsa_hashmap_get_internal_wrapper((t), (void *)APEDSA_ADDRESSOF((t)->key, (k)), sizeof((t)->key), sizeof(*(t)), \
						     APEDSA_HASHMAP_MODE_BINARY),                                                  \
	 apedsa_da_temp(t))

#define apedsa_hm_getp(t, k) ((void)apedsa_hm_geti(t, k), &(t)[apedsa_da_temp(t)])

#define apedsa_hm_del(t, k)                                                                                                  \
	(__apedsa_hashmap_del_internal_wrapper((t), (void *)APEDSA_ADDRESSOF((t)->key, (k)), sizeof((t)->key), sizeof(*(t)), \
					       APEDSA_OFFSETOF((t), key), APEDSA_HASHMAP_MODE_BINARY))

#define apedsa_hm_gets(t, k) (*apedsa_hm_getp(t, k))
#define apedsa_hm_get(t, k) (apedsa_hm_getp(t, k)->value)
#define apedsa_hm_len(t) (apedsa_da_count(t))

#define apedsa_hm_with_cap(a, n) __apedsa_hashmap_reserve_internal(a, n, sizeof(*(a)))

#define apedsa_hm_put_batch(t, v, n) \
	((t) = __apedsa_hashmap_put_internal_batch_wrapper((t), (n), (v), sizeof((t)->key), sizeof(*(t)), APEDSA_HASHMAP_MODE_BINARY))

#define apedsa_shm_len(t) (apedsa_da_count(t))

#define apedsa_shm_put(t, k, v)                                                                                                \
	((t) = __apedsa_hashmap_put_internal_wrapper((t), (void *)(k), sizeof((k)), sizeof(*(t)), APEDSA_HASHMAP_MODE_STRING), \
	 (t)[apedsa_da_temp(t)].value = (v))

#define apedsa_shm_puts(t, s)                                                                                                  \
	((t) = __apedsa_hashmap_put_internal_wrapper((t), (s).key, sizeof((s).key), sizeof(*(t)), APEDSA_HASHMAP_MODE_STRING), \
	 (t)[apedsa_da_temp].value = (v))

#define apedsa_shm_geti(t, k)                                                                                                       \
	((t) = __apedsa_hashmap_get_internal_wrapper((t), (void *)(k), sizeof((t)->key), sizeof(*(t)), APEDSA_HASHMAP_MODE_STRING), \
	 apedsa_da_temp(t))
#define apedsa_shm_getp(t, k) ((void)apedsa_shm_geti(t, k), &(t)[apedsa_da_temp(t)])
#define apedsa_shm_gets(t, k) (*apedsa_shm_getp(t, k))
#define apedsa_shm_get(t, k) (apedsa_shm_getp(t, k)->value)
#define apedsa_shm_del(t, k)                                                                                                      \
	((t) = __apedsa_hashmap_del_internal_wrapper((t), (void *)(k), sizeof((t)->key), sizeof(*(t)), APEDSA_OFFSETOF((t), key), \
						     APEDSA_HASHMAP_MODE_STRING))

#define apedsa_shm_with_cap apedsa_hm_with_cap

#define apedsa_shm_put_batch(t, v, n) \
	((t) = __apedsa_hashmap_put_internal_batch_wrapper((t), (n), (v), sizeof((t)->key), sizeof(*(t)), APEDSA_HASHMAP_MODE_STRING))

typedef struct {
	size_t capacity;
	size_t count;
	void *aux;	// a pointer to either a hashmap or a btree (depending on type)
	ptrdiff_t temp; // stores temporary values for hashmap and btree
} ApedsaDaHeader;

typedef struct ApedsaStringBlock {
	struct ApedsaStringBlock *next;
	char data[8];
} ApedsaStringBlock;

struct ApedsaStringArena {
	ApedsaStringBlock *blocks;
	size_t remaining;
	unsigned char block;
};

enum {
	APEDSA_HASHMAP_MODE_BINARY,
	APEDSA_HASHMAP_MODE_STRING,
};

// These wrappers allow us to work in C++ as well
#ifdef __cplusplus
template <typename T> static T *__apedsa_da_growf_wrapper(T *da, size_t esz, size_t growby, size_t min_cap)
{
	return (T *)__apedsa_da_growf((void *)da, esz, growby, min_cap);
}
template <typename T> static T *__apedsa_hashmap_put_internal_wrapper(T *hashmap, void *key, size_t key_size, size_t kv_size, int mode)
{
	return (T *)__apedsa_hashmap_put_internal((void *)hashmap, key, key_size, kv_size, mode);
}
template <typename T> static T *__apedsa_hashmap_get_internal_wrapper(T *hashmap, void *key, size_t key_size, size_t kv_size, int mode)
{
	return (T *)__apedsa_hashmap_get_internal((void *)hashmap, key, key_size, kv_size, mode);
}
template <typename T>
static T *__apedsa_hashmap_del_internal_wrapper(T *hashmap, void *key, size_t key_size, size_t kv_size, size_t koff, int mode)
{
	return (T *)__apedsa_hashmap_del_internal((void *)hashmap, key, key_size, kv_size, koff, mode);
}
template <typename T>
static T *__apedsa_hashmap_put_internal_batch_wrapper(T *hashmap, size_t count, void *pairs, size_t key_size, size_t kv_size, int mode)
{
	return (T *)__apedsa_hashmap_put_internal_batch((void *)hashmap, count, pairs, key_size, kv_size, mode);
}
#else
#define __apedsa_da_growf_wrapper __apedsa_da_growf
#define __apedsa_hashmap_put_internal_wrapper __apedsa_hashmap_put_internal
#define __apedsa_hashmap_get_internal_wrapper __apedsa_hashmap_get_internal
#define __apedsa_hashmap_del_internal_wrapper __apedsa_hashmap_del_internal
#define __apedsa_hashmap_put_internal_batch_wrapper __apedsa_hashmap_put_internal_batch
#endif

#if defined(APEDSA_STRIP_PREFIX)

#define da_count apedsa_da_count
#define da_len da_count
#define da_put apedsa_da_put
#define da_push apedsa_da_push
#define da_last apedsa_da_last
#define da_deleten apedsa_da_deleten
#define da_delete apedsa_da_delete
#define da_delete_swap apedsa_da_delete_swap
#define da_insertn apedsa_da_insertn
#define da_insert apedsa_da_insert
#define da_grow apedsa_da_grow
#define da_reserve apedsa_da_reserve

#define hm_len apedsa_hm_len
#define hm_put apedsa_hm_put
#define hm_puts apedsa_hm_puts
#define hm_geti apedsa_hm_geti
#define hm_getp apedsa_hm_getp
#define hm_gets apedsa_hm_gets
#define hm_get apedsa_hm_get
#define hm_del apedsa_hm_del

#define hm_with_cap apedsa_hm_with_cap

#define hm_put_batch apedsa_hm_put_batch

#define shm_len apedsa_shm_len
#define shm_put apedsa_shm_put
#define shm_puts apedsa_shm_puts
#define shm_geti apedsa_shm_geti
#define shm_getp apedsa_shm_getp
#define shm_gets apedsa_shm_gets
#define shm_get apedsa_shm_get
#define shm_del apedsa_shm_del

#define shm_with_cap apedsa_shm_with_cap

#define shm_put_batch apedsa_shm_put_batch

#endif

#endif

#ifdef APEDSA_IMPLEMENTATION

#ifndef APEDSA_IMPLEMENTATION_INCLUDED
#define APEDSA_IMPLEMENTATION_INCLUDED

#include <stdint.h>
#include <string.h>

/* User can define a custom function prefix (eg. static) */
#ifndef APEDSA_DEF
#define APEDSA_DEF
#endif

/* Can also define custom private function prefix */
#ifndef APEDSA_PRIVATE
#define APEDSA_PRIVATE static
#endif

#ifndef APEDSA_TRUE
#define APEDSA_TRUE (1)
#define APEDSA_FALSE (0)
#else
#if !defined(APEDSA_FALSE)
#pragma GCC error "Need to define both APEDSA_TRUE and APEDSA_FALSE or neither"
#endif
#endif

#include <stdint.h>

/* BEGIN da.c */

void *__apedsa_da_growf(void *da, size_t esz, size_t growby, size_t min_cap)
{
	size_t min_len = apedsa_da_count(da) + growby;
	if (min_len > min_cap)
		min_cap = min_len;
	if (min_cap <= apedsa_da_cap(da))
		return da;
	if (min_cap < 2 * apedsa_da_cap(da))
		min_cap = 2 * apedsa_da_cap(da);
	else if (min_cap < 4)
		min_cap = 4;
	void *new_da = APEDSA_REALLOC((da) ? apedsa_da_header(da) : 0, min_cap * esz + sizeof(ApedsaDaHeader));
	new_da = (char *)new_da + sizeof(ApedsaDaHeader);
	if (da == NULL) {
		apedsa_da_header(new_da)->count = 0;
		apedsa_da_header(new_da)->aux = NULL;
	}
	apedsa_da_header(new_da)->capacity = min_cap;
	return new_da;
}
/* END da.c */

/* BEGIN hashmap.c */

static size_t __apedsa_hash_seed = 0x31415926;

APEDSA_DEF void apedsa_rand_seed(size_t seed)
{
	__apedsa_hash_seed = seed;
}

#define __APEDSA_SIZE_T_BITS (sizeof(size_t) == 8 ? 64 : 32)
#define __APEDSA_ROTL32(x, n) ((x) << (n) | (x >> (32 - (n))))
#define __APEDSA_ROTR32(x, n) ((x) >> (n) | (x << (32 - (n))))
#define __APEDSA_ROTL64(x, n) ((x) << (n) | (x >> (64 - (n))))
#define __APEDSA_ROTR64(x, n) ((x) >> (n) | (x << (64 - (n))))

APEDSA_PRIVATE inline uint32_t __apedsa_fmix32(uint32_t h)
{
	h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;
	return h;
}

#if defined(__cplusplus) && __cplusplus >= 201703L
#define __APEDSA_FALLTHROUGH [[fallthrough]]
#elif defined(__GNUC__) && __GNUC__ >= 7 || defined(__clang__)
#define __APEDSA_FALLTHROUGH __attribute__((fallthrough))
#else
#define __APEDSA_FALLTHROUGH
#endif

APEDSA_DEF void apedsa_da_murmurhash3_128(const void *key, size_t len, size_t seed, void *out)
{
	const unsigned char *data = (const unsigned char *)key;
	const int nblocks = len / 16;

	uint32_t h1 = seed;
	uint32_t h2 = seed;
	uint32_t h3 = seed;
	uint32_t h4 = seed;

	const uint32_t c1 = 0x239b961b;
	const uint32_t c2 = 0xab0e9789;
	const uint32_t c3 = 0x38b34ae5;
	const uint32_t c4 = 0xa1e38b93;

	const uint32_t *blocks = (const uint32_t *)(data + nblocks * 16);

	for (int i = -nblocks; i; i++) {
		// clang-format off
        uint32_t k1 = blocks[i*4+0];uint32_t k2 = blocks[i*4+1];uint32_t k3 = blocks[i*4+2];uint32_t k4 = blocks[i*4+3];
        k1 *= c1; k1 = __APEDSA_ROTL32(k1, 15); k1 *= c2; h1 ^= k1; h1 = __APEDSA_ROTL32(h1, 19); h1 += h2; h1 = h1 * 5 + 0x561ccd1b;
        k2 *= c2; k2 = __APEDSA_ROTL32(k2, 16); k2 *= c3; h2 ^= k2; h2 = __APEDSA_ROTL32(h2, 17); h2 += h3; h2 = h2 * 5 + 0x0bcaa747;
        k3 *= c3; k3 = __APEDSA_ROTL32(k3, 17); k3 *= c4; h3 ^= k3; h3 = __APEDSA_ROTL32(h3, 15); h3 += h4; h3 = h3 * 5 + 0x96cd1c35;
        k4 *= c4; k4 = __APEDSA_ROTL32(k4, 18); k4 *= c1; h4 ^= k4; h4 = __APEDSA_ROTL32(h4, 13); h4 += h1; h4 = h4 * 5 + 0x32ac3b17;
		// clang-format on
	}

	const unsigned char *tail = (const unsigned char *)(data + nblocks * 16);

	uint32_t k1 = 0;
	uint32_t k2 = 0;
	uint32_t k3 = 0;
	uint32_t k4 = 0;
	switch (len & 15) {
			// clang-format off
        // fall through
        case 15: k4 ^= (uint32_t)tail[14] << 16;
        // fall through
        case 14: k4 ^= (uint32_t)tail[13] << 8;
        // fall through
        case 13: k4 ^= (uint32_t)tail[12] << 0;
                 k4 *= c4; k4 = __APEDSA_ROTL32(k4, 18); k4 *= c1; h4 ^= k4;
        // fall through
        case 12: k3 ^= (uint32_t)tail[11] << 24;
        // fall through
        case 11: k3 ^= (uint32_t)tail[10] << 16;
        // fall through
        case 10: k3 ^= (uint32_t)tail[9] << 8;
        // fall through
        case 9: k3 ^= (uint32_t)tail[8] << 0;
                k3 *= c3; k3 = __APEDSA_ROTL32(k3, 17); k3 *= c4; h3 ^= k3;
        // fall through
        case 8: k2 ^= (uint32_t)tail[7] << 24;
        // fall through
        case 7: k2 ^= (uint32_t)tail[6] << 16;
        // fall through
        case 6: k2 ^= (uint32_t)tail[5] << 8;
        // fall through
        case 5: k2 ^= (uint32_t)tail[4] << 0;
                k2 *= c2; k2 = __APEDSA_ROTL32(k2, 18); k2 *= c3; h2 ^= k2;
        // fall through
        case 4: k1 ^= (uint32_t)tail[3] << 24;
        // fall through
        case 3: k1 ^= (uint32_t)tail[2] << 16;
        // fall through
        case 2: k1 ^= (uint32_t)tail[1] << 8;
        // fall through
        case 1: k1 ^= (uint32_t)tail[0] << 0; 
                k1 *= c1; k1 = __APEDSA_ROTL32(k1, 31); k1 *= c2; h1 ^= k1;
			// clang-format on
	}

	// clang-format off
	h1 ^= len; h2 ^= len; h3 ^= len; h4 ^= len;
	h1 += h2;  h1 += h3;  h1 += h4;
	h2 += h1;  h3 += h1;  h4 += h1;
    h1 = __apedsa_fmix32(h1);
    h2 = __apedsa_fmix32(h2);
    h3 = __apedsa_fmix32(h3);
    h4 = __apedsa_fmix32(h4);
    h1 += h2; h1 += h3; h1 += h4;
    h2 += h1; h3 += h1; h4 += h1;
	// clang-format on
	((uint32_t *)out)[0] = h1;
	((uint32_t *)out)[1] = h2;
	((uint32_t *)out)[2] = h3;
	((uint32_t *)out)[3] = h4;
}

APEDSA_DEF size_t apedsa_hash_bytes(void *p, size_t len, size_t seed)
{
	char hash[16];
	apedsa_da_murmurhash3_128(p, len, seed, hash);
	return *(size_t *)hash; // Just discard unused bits
}

APEDSA_DEF size_t apedsa_hash_string(char *str, size_t seed)
{
	char hash[16];
	apedsa_da_murmurhash3_128(str, strlen(str), seed, hash);
	return *(size_t *)hash;
}

#define APEDSA_HASHMAP_HASH_EMPTY 0
#define APEDSA_HASHMAP_HASH_DELETED 1

#define APEDSA_HASHMAP_INDEX_EMPTY -1
#define APEDSA_HASHMAP_INDEX_DELETED -2
#define APEDSA_HASHMAP_INDEX_IN_USE(x) ((x) >= 0)

#define APEDSA_HASHMAP_BUCKET_SIZE 8
#define APEDSA_HASHMAP_BUCKET_SHIFT 3
#define APEDSA_HASHMAP_BUCKET_MASK (APEDSA_HASHMAP_BUCKET_SIZE - 1)
#define APEDSA_HASHMAP_DOUBLE_HASH_PRIME 7

typedef struct {
	size_t hash;
	ptrdiff_t index;
} ApedsaHashBucketSlot;

typedef struct {
	// size_t hash[APEDSA_HASHMAP_BUCKET_SIZE];
	// ptrdiff_t index[APEDSA_HASHMAP_BUCKET_SIZE];
	ApedsaHashBucketSlot slots[APEDSA_HASHMAP_BUCKET_SIZE]; // 128 bytes = 2 cache-line
} ApedsaHashBucket;

typedef struct {
	size_t slot_count;
	size_t used_count;
	size_t used_count_threshold;
	size_t used_count_shrink_threshold;
	size_t tombstone_count;
	size_t tombstone_count_threshold;
	size_t seed;
	ApedsaHashBytesFn hash_bytes_fn;
	ApedsaHashStringFn hash_string_fn;
	ApedsaStringArena string;
	ApedsaHashBucket *buckets; // This is not actually a separate allocation
} ApedsaHashIndex;

#define __apedsa_load_32_or_64(var, temp, v32, v64_hi, v64_lo)                                           \
	temp = v64_lo ^ v32, temp <<= 16, temp <<= 16, temp >>= 16, temp >>= 16, /* discard if 32-bit */ \
		var = v64_hi, var <<= 16, var <<= 16,				 /* discard if 32-bit */ \
		var ^= temp ^ v32

APEDSA_PRIVATE size_t __apedsa_hashmap_probe_position(size_t hash, size_t slot_count)
{
	size_t pos = hash & (slot_count - 1);
	return pos;
}

ApedsaHashIndex *__apedsa_hashmap_rehash(size_t slot_count, ApedsaHashIndex *old)
{
	ApedsaHashIndex *table = (ApedsaHashIndex *)APEDSA_MALLOC(sizeof(ApedsaHashIndex) +
								  sizeof(ApedsaHashBucket) * (slot_count >> APEDSA_HASHMAP_BUCKET_SHIFT) +
								  APEDSA_CACHE_LINE_SIZE - 1);
	table->slot_count = slot_count;
	table->used_count = 0;
	table->used_count_threshold = slot_count * 12 / 16;
	table->tombstone_count_threshold = slot_count * 2 / 16;
	table->used_count_shrink_threshold = slot_count * 4 / 16;
	table->tombstone_count = 0;
	table->hash_bytes_fn = NULL;
	table->hash_string_fn = NULL;
	// make sure the buckets start on a cache line
	table->buckets = (ApedsaHashBucket *)(((uintptr_t)(table + 1) + APEDSA_CACHE_LINE_SIZE - 1) & ~(APEDSA_CACHE_LINE_SIZE - 1));
	if (slot_count <= APEDSA_HASHMAP_BUCKET_SIZE)
		table->used_count_shrink_threshold = 0;
	APEDSA_ASSERT(table->used_count_threshold + table->tombstone_count_threshold < slot_count);
	if (old) {
		table->string = old->string;
		table->seed = old->seed;
		table->hash_bytes_fn = old->hash_bytes_fn;
		table->hash_string_fn = old->hash_string_fn;
	} else {
		memset(&table->string, 0, sizeof(table->string));
		table->seed = __apedsa_hash_seed;
		size_t a, b, temp;
		__apedsa_load_32_or_64(a, temp, 2147001325, 0x27bb2ee6, 0x87b0b0fd);
		__apedsa_load_32_or_64(b, temp, 715136305, 0, 0xb504f32d);
		__apedsa_hash_seed = __apedsa_hash_seed * a + b;
	}
	for (size_t i = 0; i < slot_count >> APEDSA_HASHMAP_BUCKET_SHIFT; i++) {
		ApedsaHashBucket *bucket = &table->buckets[i];
		// memset is probably more optimized than anything I could write
		for (size_t j = 0; j < APEDSA_HASHMAP_BUCKET_SIZE; j++) {
			bucket->slots[j].hash = APEDSA_HASHMAP_HASH_EMPTY;
			bucket->slots[j].index = APEDSA_HASHMAP_INDEX_EMPTY;
		}
	}

	if (old) {
		table->used_count = old->used_count;
		for (size_t i = 0; i < old->slot_count >> APEDSA_HASHMAP_BUCKET_SHIFT; i++) {
			for (size_t j = 0; j < APEDSA_HASHMAP_BUCKET_SIZE; j++) {
				ApedsaHashBucket *ob = old->buckets + i;
				if (APEDSA_HASHMAP_INDEX_IN_USE(ob->slots[j].index)) {
					size_t hash = ob->slots[j].hash;
					size_t pos = __apedsa_hashmap_probe_position(hash, slot_count);
#if defined(APEDSA_HASHMAP_QUADRATIC_PROBING) || defined(APEDSA_HASHMAP_LINEAR_PROBING)
					size_t step = APEDSA_HASHMAP_BUCKET_SIZE;
#elif defined(APEDSA_HASHMAP_DOUBLE_HASHING)
					size_t step = APEDSA_HASHMAP_DOUBLE_HASH_PRIME - (hash % APEDSA_HASHMAP_DOUBLE_HASH_PRIME);
#endif
					ApedsaHashBucket *bucket;
					for (;;) {
						bucket = &table->buckets[pos >> APEDSA_HASHMAP_BUCKET_SHIFT];
						for (size_t k = pos & APEDSA_HASHMAP_BUCKET_MASK; k < APEDSA_HASHMAP_BUCKET_SIZE; k++) {
							if (bucket->slots[k].hash == APEDSA_HASHMAP_HASH_EMPTY) {
								bucket->slots[k].hash = hash;
								bucket->slots[k].index = ob->slots[j].index;
								goto done;
							}
						}
						size_t limit = pos & APEDSA_HASHMAP_BUCKET_MASK;
						for (size_t k = 0; k < limit; k++) {
							if (bucket->slots[k].hash == APEDSA_HASHMAP_HASH_EMPTY) {
								bucket->slots[k].hash = hash;
								bucket->slots[k].index = ob->slots[j].index;
								goto done;
							}
						}
						pos += step;
#if defined(APEDSA_HASHMAP_QUADRATIC_PROBING)
						step += APEDSA_HASHMAP_BUCKET_SIZE;
#endif
						pos &= (table->slot_count - 1);
						// __builtin_prefetch(&table->buckets[pos >> APEDSA_HASHMAP_BUCKET_SHIFT], 0, 3);
					}
				}
			done:;
			}
		}
	}
	return table;
}

#define MASK(b) (((size_t)0x1 << (b)) - 1)
#define KEY_SIZE_MASK(n) MASK((size_t)(n) * 8)

APEDSA_PRIVATE int __apedsa_is_key_equal(void *a, void *key, size_t key_size, size_t kv_size, size_t i, int mode)
{
	// clang-format off
    size_t len = mode >= APEDSA_HASHMAP_MODE_STRING ? strlen((char *)key) : key_size;
    if (len <= 8) { // compare bytes directly for small keys
        size_t key_masked = len == 8 ? *(size_t*)key : ((size_t)KEY_SIZE_MASK(len) & *(size_t*)key);
        char *key2 = mode >= APEDSA_HASHMAP_MODE_STRING ? (char *)*(char **)((char *)a + i * kv_size)
            : (char *)a + i * kv_size;
        size_t key2_masked = len == 8 ? *(size_t*)key2 : ((size_t)KEY_SIZE_MASK(len) & *(size_t*)key2);
        return key_masked == key2_masked;
    }
	// clang-format on
	if (mode >= APEDSA_HASHMAP_MODE_STRING)
		return strcmp(*(char **)((char *)a + i * kv_size), (char *)key) == 0;
	return memcmp((char *)a + i * kv_size, key, key_size) == 0;
}

void *__apedsa_hashmap_put_internal(void *a, void *key, size_t key_size, size_t kv_size, int mode)
{
	if (a == NULL) {
		a = __apedsa_da_growf(a, kv_size, 1, 0);
		memset(a, 0, kv_size);
	}
	ApedsaHashIndex *table = (ApedsaHashIndex *)apedsa_da_header(a)->aux;
	if (table == NULL || table->used_count >= table->used_count_threshold) {
		size_t slot_count = (table == NULL) ? APEDSA_HASHMAP_BUCKET_SIZE : table->slot_count * 2;
		ApedsaHashIndex *new_table = __apedsa_hashmap_rehash(slot_count, table);
		if (table) {
			APEDSA_FREE(table);
		}
		apedsa_da_header(a)->aux = table = new_table;
	}

	size_t hash = mode >= APEDSA_HASHMAP_MODE_STRING ? table->hash_string_fn ? table->hash_string_fn((char *)key, table->seed) :
										   apedsa_hash_string((char *)key, table->seed) :
		      table->hash_bytes_fn		 ? table->hash_bytes_fn(key, key_size, table->seed) :
							   apedsa_hash_bytes(key, key_size, table->seed);
#if defined(APEDSA_HASHMAP_QUADRATIC_PROBING) || defined(APEDSA_HASHMAP_LINEAR_PROBING)
	size_t step = APEDSA_HASHMAP_BUCKET_SIZE;
#elif defined(APEDSA_HASHMAP_DOUBLE_HASHING)
	size_t step = APEDSA_HASHMAP_DOUBLE_HASH_PRIME - (hash % APEDSA_HASHMAP_DOUBLE_HASH_PRIME);
#endif
	ptrdiff_t tombstone = -1;
	if (hash < 2)
		hash += 2;
	size_t pos = __apedsa_hashmap_probe_position(hash, table->slot_count);
	ApedsaHashBucket *bucket;

	for (;;) {
		bucket = &table->buckets[pos >> APEDSA_HASHMAP_BUCKET_SHIFT];

		for (size_t i = pos & APEDSA_HASHMAP_BUCKET_MASK; i < APEDSA_HASHMAP_BUCKET_SIZE; i++) {
			if (bucket->slots[i].hash == hash &&
			    __apedsa_is_key_equal(a, key, key_size, kv_size, bucket->slots[i].index, mode)) {
				apedsa_da_header(a)->temp = bucket->slots[i].index;
				return a;
			} else if (bucket->slots[i].hash == APEDSA_HASHMAP_HASH_EMPTY) {
				pos = (pos & ~APEDSA_HASHMAP_BUCKET_MASK) + i;
				goto found_empty_slot;
			} else if (tombstone < 0 && bucket->slots[i].index == APEDSA_HASHMAP_INDEX_DELETED) {
				tombstone = (ptrdiff_t)((pos & ~APEDSA_HASHMAP_BUCKET_MASK) + i);
			}
		}
		size_t limit = pos & APEDSA_HASHMAP_BUCKET_MASK;
		for (size_t i = 0; i < limit; i++) {
			if (bucket->slots[i].hash == hash &&
			    __apedsa_is_key_equal(a, key, key_size, kv_size, bucket->slots[i].index, mode)) {
				apedsa_da_header(a)->temp = bucket->slots[i].index;
				return a;
			} else if (bucket->slots[i].hash == APEDSA_HASHMAP_HASH_EMPTY) {
				pos = (pos & ~APEDSA_HASHMAP_BUCKET_MASK) + i;
				goto found_empty_slot;
			} else if (tombstone < 0 && bucket->slots[i].index == APEDSA_HASHMAP_INDEX_DELETED) {
				tombstone = (ptrdiff_t)((pos & ~APEDSA_HASHMAP_BUCKET_MASK) + i);
			}
		}

		pos += step;
#if defined(APEDSA_HASHMAP_QUADRATIC_PROBING)
		step += APEDSA_HASHMAP_BUCKET_SIZE;
#endif
		pos &= (table->slot_count - 1);
		// __builtin_prefetch(&table->buckets[pos >> APEDSA_HASHMAP_BUCKET_SHIFT], 0, 3);
	}
found_empty_slot:
	if (tombstone >= 0) {
		pos = tombstone;
		table->tombstone_count--;
	}
	table->used_count++;

	ptrdiff_t i = (ptrdiff_t)apedsa_da_count(a);
	if ((size_t)i + 1 > apedsa_da_cap(a))
		*(void **)&a = __apedsa_da_growf(a, kv_size, 1, 0);
	APEDSA_ASSERT((size_t)i + 1 <= apedsa_da_cap(a));
	apedsa_da_header(a)->count++;
	bucket = &table->buckets[pos >> APEDSA_HASHMAP_BUCKET_SHIFT];
	bucket->slots[pos & APEDSA_HASHMAP_BUCKET_MASK].hash = hash;
	bucket->slots[pos & APEDSA_HASHMAP_BUCKET_MASK].index = i;
	apedsa_da_header(a)->temp = i;
	if (mode >= APEDSA_HASHMAP_MODE_STRING)
		*(char **)((char *)a + i * kv_size) = apedsa_string_arena_alloc(&table->string, (char *)key);
	return a;
}

void *__apedsa_hashmap_put_internal_batch(void *a, size_t count, void *pairs, size_t key_size, size_t kv_size, int mode)
{
	if (a == NULL) {
		a = __apedsa_da_growf(a, kv_size, count, 0);
		memset(a, 0, kv_size * count);
	}
	ApedsaHashIndex *table = (ApedsaHashIndex *)apedsa_da_header(a)->aux;
	size_t existiing = table ? table->used_count : 0;
	size_t needed = count + existiing;
	size_t new_slot_count = table ? table->slot_count : APEDSA_HASHMAP_BUCKET_SIZE;
	while (new_slot_count < count || (needed > new_slot_count - (new_slot_count >> 2))) {
		new_slot_count *= 2;
	}
	if (table == NULL || new_slot_count > table->slot_count) {
		table = __apedsa_hashmap_rehash(new_slot_count, table);
		apedsa_da_header(a)->aux = table;
	}
	size_t old_threshold = table->used_count_threshold;
	table->used_count_threshold = SIZE_MAX;
	// size_t old_count = apedsa_da_count(a);
	// if (mode >= APEDSA_HASHMAP_MODE_STRING) { pairs = *(char **)pairs; }
	for (size_t i = 0; i < count; i++) {
		void *pair = ((char *)pairs + i * kv_size);
		char *key = mode >= APEDSA_HASHMAP_MODE_STRING ? *(char **)(char *)pair : (char *)pair;
		// char *value = ((char *)pairs + i * kv_size + key_size);
		a = __apedsa_hashmap_put_internal(a, key, key_size, kv_size, mode);
		memcpy((char *)a + (apedsa_da_count(a) - 1) * kv_size, pair, kv_size);
		table = (ApedsaHashIndex *)apedsa_da_header(a)->aux;
	}
	if (table->used_count > old_threshold) {
		table = __apedsa_hashmap_rehash(table->slot_count * 2, table);
		apedsa_da_header(a)->aux = table;
	}
	table->used_count_threshold = old_threshold;
	return a;
}

APEDSA_PRIVATE ptrdiff_t __apedsa_hashmap_find_slot(void *a, void *key, size_t key_size, size_t kv_size, int mode)
{
	ApedsaHashIndex *table = (ApedsaHashIndex *)apedsa_da_header(a)->aux;
	size_t hash = mode >= APEDSA_HASHMAP_MODE_STRING ? table->hash_string_fn ? table->hash_string_fn((char *)key, table->seed) :
										   apedsa_hash_string((char *)key, table->seed) :
		      table->hash_bytes_fn		 ? table->hash_bytes_fn(key, key_size, table->seed) :
							   apedsa_hash_bytes(key, key_size, table->seed);
	if (hash < 2)
		hash += 2;
#if defined(APEDSA_HASHMAP_QUADRATIC_PROBING) || defined(APEDSA_HASHMAP_LINEAR_PROBING)
	size_t step = APEDSA_HASHMAP_BUCKET_SIZE;
#elif defined(APEDSA_HASHMAP_DOUBLE_HASHING)
	size_t step = APEDSA_HASHMAP_DOUBLE_HASH_PRIME - (hash % APEDSA_HASHMAP_DOUBLE_HASH_PRIME);
#endif
	size_t pos = __apedsa_hashmap_probe_position(hash, table->slot_count);
	ApedsaHashBucket *bucket;

#define __APEDSA_HASHMAP_FIND_PROBE_LOOP_I(x)                                                                                  \
	if (bucket->slots[x].hash == hash && __apedsa_is_key_equal(a, key, key_size, kv_size, bucket->slots[x].index, mode)) { \
		return (ptrdiff_t)((pos & ~APEDSA_HASHMAP_BUCKET_MASK) + x);                                                   \
	} else if (bucket->slots[x].hash == APEDSA_HASHMAP_HASH_EMPTY) {                                                       \
		return APEDSA_HASHMAP_INDEX_EMPTY;                                                                             \
	}

	for (;;) {
		bucket = &table->buckets[pos >> APEDSA_HASHMAP_BUCKET_SHIFT];

		for (size_t i = pos & APEDSA_HASHMAP_BUCKET_MASK; i < APEDSA_HASHMAP_BUCKET_SIZE; i++) {
			if (bucket->slots[i].hash == hash &&
			    __apedsa_is_key_equal(a, key, key_size, kv_size, bucket->slots[i].index, mode)) {
				return (ptrdiff_t)((pos & ~APEDSA_HASHMAP_BUCKET_MASK) + i);
			} else if (bucket->slots[i].hash == APEDSA_HASHMAP_HASH_EMPTY) {
				return APEDSA_HASHMAP_INDEX_EMPTY;
			}
		}
		size_t limit = pos & APEDSA_HASHMAP_BUCKET_MASK;
		for (size_t i = 0; i < limit; i++) {
			if (bucket->slots[i].hash == hash &&
			    __apedsa_is_key_equal(a, key, key_size, kv_size, bucket->slots[i].index, mode)) {
				return (ptrdiff_t)((pos & ~APEDSA_HASHMAP_BUCKET_MASK) + i);
			} else if (bucket->slots[i].hash == APEDSA_HASHMAP_HASH_EMPTY) {
				return APEDSA_HASHMAP_INDEX_EMPTY;
			}
		}

		pos += step;
#if defined(APEDSA_HASHMAP_QUADRATIC_PROBING)
		step += APEDSA_HASHMAP_BUCKET_SIZE;
#endif
		pos &= (table->slot_count - 1);
		// __builtin_prefetch(&table->buckets[pos >> APEDSA_HASHMAP_BUCKET_SHIFT], 0, 3);
	}
	return APEDSA_HASHMAP_INDEX_EMPTY;
}

void *__apedsa_hashmap_get_internal(void *a, void *key, size_t key_size, size_t kv_size, int mode)
{
	if (a == NULL) {
		a = __apedsa_da_growf(a, kv_size, 0, 1);
		memset(a, 0, kv_size);
		apedsa_da_temp(a) = APEDSA_HASHMAP_INDEX_EMPTY;
		return a;
	}
	ApedsaHashIndex *table = (ApedsaHashIndex *)apedsa_da_header(a)->aux;
	if (table == NULL)
		apedsa_da_temp(a) = APEDSA_HASHMAP_INDEX_EMPTY;
	else {
		ptrdiff_t slot = __apedsa_hashmap_find_slot(a, key, key_size, kv_size, mode);
		if (slot < 0) {
			apedsa_da_temp(a) = APEDSA_HASHMAP_INDEX_EMPTY;
		} else {
			ApedsaHashBucket *b = &table->buckets[slot >> APEDSA_HASHMAP_BUCKET_SHIFT];
			apedsa_da_temp(a) = b->slots[slot & APEDSA_HASHMAP_BUCKET_MASK].index;
		}
	}
	return a;
}

void *__apedsa_hashmap_del_internal(void *a, void *key, size_t key_size, size_t kv_size, size_t koff, int mode)
{
	if (a == NULL)
		return 0;
	ApedsaHashIndex *table = (ApedsaHashIndex *)apedsa_da_header(a)->aux;
	if (table == NULL)
		return a;
	ptrdiff_t slot = __apedsa_hashmap_find_slot(a, key, key_size, kv_size, mode);
	if (slot < 0)
		return a;
	ApedsaHashBucket *b = &table->buckets[slot >> APEDSA_HASHMAP_BUCKET_SHIFT];
	int i = slot & APEDSA_HASHMAP_BUCKET_MASK;
	ptrdiff_t old_index = b->slots[i].index;
	ptrdiff_t final_index = (ptrdiff_t)apedsa_da_count(a) - 1;
	APEDSA_ASSERT(slot < (ptrdiff_t)table->slot_count);
	table->used_count--;
	table->tombstone_count++;
	apedsa_da_temp(a) = 1;
	// APEDSA_ASSERT(table->used_count >= 0);
	b->slots[i].hash = APEDSA_HASHMAP_HASH_DELETED;
	b->slots[i].index = APEDSA_HASHMAP_INDEX_DELETED;
	if (old_index != final_index) {
		memmove((char *)a + kv_size * old_index, (char *)a + kv_size * final_index, kv_size);
		if (mode >= APEDSA_HASHMAP_MODE_STRING)
			slot = __apedsa_hashmap_find_slot(a, *(char **)((char *)a + kv_size * old_index + koff), key_size, kv_size, mode);
		else
			slot = __apedsa_hashmap_find_slot(a, (char *)a + kv_size * old_index + koff, key_size, kv_size, mode);
		APEDSA_ASSERT(slot >= 0);
		b = &table->buckets[slot >> APEDSA_HASHMAP_BUCKET_SHIFT];
		i = slot & APEDSA_HASHMAP_BUCKET_MASK;
		APEDSA_ASSERT(b->slots[i].index == final_index);
		b->slots[i].index = old_index;
	}
	apedsa_da_header(a)->count--;
	if (table->used_count < table->used_count_shrink_threshold && table->slot_count > APEDSA_HASHMAP_BUCKET_SIZE)
		apedsa_da_header(a)->aux = __apedsa_hashmap_rehash(table->slot_count >> 1, table);
	else if (table->tombstone_count > table->tombstone_count_threshold)
		apedsa_da_header(a)->aux = __apedsa_hashmap_rehash(table->slot_count, table);

	return a;
}

void *__apedsa_hashmap_reserve_internal(void *a, size_t count, size_t kv_size)
{
	if (a == NULL) {
		a = __apedsa_da_growf(a, kv_size, count, 0);
		memset(a, 0, kv_size * count);
	}
	ApedsaHashIndex *table = (ApedsaHashIndex *)apedsa_da_header(a)->aux;
	if (table == NULL || table->used_count >= table->used_count_threshold || table->slot_count < count) {
		size_t slot_count = (table == NULL) ? APEDSA_HASHMAP_BUCKET_SIZE : table->slot_count * 2;
		while (slot_count < count)
			slot_count *= 2;
		ApedsaHashIndex *new_table = __apedsa_hashmap_rehash(slot_count, table);
		if (table) {
			APEDSA_FREE(table);
		}
		apedsa_da_header(a)->aux = table = new_table;
	}
	return a;
}

void apedsa_hashmap_set_hash_fns(void *a, ApedsaHashBytesFn bytes_fn, ApedsaHashStringFn string_fn)
{
	if (a == NULL)
		return;
	ApedsaHashIndex *table = (ApedsaHashIndex *)apedsa_da_header(a)->aux;
	if (table == NULL)
		return;
	table->hash_bytes_fn = bytes_fn;
	table->hash_string_fn = string_fn;
}
/* END hashmap.c */

/* BEGIN string.c */

#ifndef APEDSA_STRING_ARENA_BLOCKSIZE_MIN
#define APEDSA_STRING_ARENA_BLOCKSIZE_MIN 512
#endif
#ifndef APEDSA_STRING_ARENA_BLOCKSIZE_MAX
#define APEDSA_STRING_ARENA_BLOCKSIZE_MAX 1 << 20
#endif

APEDSA_DEF char *apedsa_string_arena_alloc(ApedsaStringArena *arena, char *str)
{
	char *p;
	size_t len = strlen(str) + 1;
	if (len > arena->remaining) {
		size_t blocksize = arena->block;
		blocksize = (size_t)(APEDSA_STRING_ARENA_BLOCKSIZE_MIN) << (blocksize >> 1);
		if (blocksize < (size_t)(APEDSA_STRING_ARENA_BLOCKSIZE_MAX))
			++arena->block;
		if (len > blocksize) {
			// Just allocate the full size
			ApedsaStringBlock *block = (ApedsaStringBlock *)APEDSA_MALLOC(sizeof(*block) - 8 + len);
			memcpy(block->data, str, len);
			if (arena->blocks) {
				block->next = arena->blocks->next;
				arena->blocks->next = block;
			} else {
				block->next = 0;
				arena->blocks = block;
				arena->remaining = 0;
			}
			return block->data;
		} else {
			ApedsaStringBlock *block = (ApedsaStringBlock *)APEDSA_MALLOC(sizeof(*block) - 8 + blocksize);
			block->next = arena->blocks;
			arena->blocks = block;
			arena->remaining = blocksize;
		}
	}
	APEDSA_ASSERT(len <= arena->remaining);
	p = arena->blocks->data + arena->remaining - len;
	arena->remaining -= len;
	memcpy(p, str, len);
	return p;
}

APEDSA_DEF void apedsa_string_arena_reset(ApedsaStringArena *arena)
{
	ApedsaStringBlock *x, *y;
	x = arena->blocks;
	while (x) {
		y = x->next;
		APEDSA_FREE(x);
		x = y;
	}
	memset(arena, 0, sizeof(*arena));
}
/* END string.c */

#endif

#endif
