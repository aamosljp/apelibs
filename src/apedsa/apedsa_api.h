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
