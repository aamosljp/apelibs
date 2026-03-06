#include "apedsa_internal.h"
#include <stdint.h>

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
        case 15: k4 ^= tail[14] << 16;
        case 14: k4 ^= tail[13] << 8;
        case 13: k4 ^= tail[12] << 0;
                 k4 *= c4; k4 = __APEDSA_ROTL32(k4, 18); k4 *= c1; h4 ^= k4;
        case 12: k3 ^= tail[11] << 24;
        case 11: k3 ^= tail[10] << 16;
        case 10: k3 ^= tail[9] << 8;
        case 9: k3 ^= tail[8] << 0;
                k3 *= c3; k3 = __APEDSA_ROTL32(k3, 17); k3 *= c4; h3 ^= k3;
        case 8: k2 ^= tail[7] << 24;
        case 7: k2 ^= tail[6] << 16;
        case 6: k2 ^= tail[5] << 8;
        case 5: k2 ^= tail[4] << 0;
                k2 *= c2; k2 = __APEDSA_ROTL32(k2, 18); k2 *= c3; h2 ^= k2;
        case 4: k1 ^= tail[3] << 24;
        case 3: k1 ^= tail[2] << 16;
        case 2: k1 ^= tail[1] << 8;
        case 1: k1 ^= tail[0] << 0; 
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
	table->used_count_threshold = slot_count - (slot_count >> 2);
	table->tombstone_count_threshold = (slot_count >> 3) + (slot_count >> 4);
	table->used_count_shrink_threshold = slot_count >> 2;
	table->tombstone_count = 0;
	table->hash_bytes_fn = NULL;
	table->hash_string_fn = NULL;
	// make sure the buckets start on a cache line
	table->buckets = (ApedsaHashBucket *)(((uintptr_t)(table + 1) + APEDSA_CACHE_LINE_SIZE - 1) & ~(APEDSA_CACHE_LINE_SIZE - 1));
	if (slot_count <= APEDSA_HASHMAP_BUCKET_SIZE)
		table->used_count_shrink_threshold = 0;
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

#define __APEDSA_HASHMAP_PROBE_LOOP_I(x)                          \
	if (bucket->slots[x].hash == APEDSA_HASHMAP_HASH_EMPTY) { \
		bucket->slots[x].hash = hash;                     \
		bucket->slots[x].index = ob->slots[j].index;      \
		goto done;                                        \
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

						size_t k = pos & APEDSA_HASHMAP_BUCKET_MASK;
						__APEDSA_HASHMAP_PROBE_LOOP_I(k);
						k = (k + 1) % APEDSA_HASHMAP_BUCKET_SIZE;
						__APEDSA_HASHMAP_PROBE_LOOP_I(k);
						k = (k + 1) % APEDSA_HASHMAP_BUCKET_SIZE;
						__APEDSA_HASHMAP_PROBE_LOOP_I(k);
						k = (k + 1) % APEDSA_HASHMAP_BUCKET_SIZE;
						__APEDSA_HASHMAP_PROBE_LOOP_I(k);
						k = (k + 1) % APEDSA_HASHMAP_BUCKET_SIZE;
						__APEDSA_HASHMAP_PROBE_LOOP_I(k);
						k = (k + 1) % APEDSA_HASHMAP_BUCKET_SIZE;
						__APEDSA_HASHMAP_PROBE_LOOP_I(k);
						k = (k + 1) % APEDSA_HASHMAP_BUCKET_SIZE;
						__APEDSA_HASHMAP_PROBE_LOOP_I(k);
						k = (k + 1) % APEDSA_HASHMAP_BUCKET_SIZE;
						__APEDSA_HASHMAP_PROBE_LOOP_I(k);

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
#undef __APEDSA_HASHMAP_PROBE_LOOP_I
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

#define __APEDSA_HASHMAP_PUT_PROBE_LOOP_I(x)                                                                                   \
	if (bucket->slots[x].hash == hash && __apedsa_is_key_equal(a, key, key_size, kv_size, bucket->slots[x].index, mode)) { \
		apedsa_da_header(a)->temp = bucket->slots[x].index;                                                            \
		return a;                                                                                                      \
	} else if (bucket->slots[x].hash == APEDSA_HASHMAP_HASH_EMPTY) {                                                       \
		pos = (pos & ~APEDSA_HASHMAP_BUCKET_MASK) + x;                                                                 \
		goto found_empty_slot;                                                                                         \
	} else if (tombstone < 0 && bucket->slots[x].index == APEDSA_HASHMAP_INDEX_DELETED) {                                  \
		tombstone = (ptrdiff_t)((pos & ~APEDSA_HASHMAP_BUCKET_MASK) + x);                                              \
	}

	for (;;) {
		bucket = &table->buckets[pos >> APEDSA_HASHMAP_BUCKET_SHIFT];

		size_t i = pos & APEDSA_HASHMAP_BUCKET_MASK;
		__APEDSA_HASHMAP_PUT_PROBE_LOOP_I(i);
		i = (i + 1) % APEDSA_HASHMAP_BUCKET_SIZE;
		__APEDSA_HASHMAP_PUT_PROBE_LOOP_I(i);
		i = (i + 1) % APEDSA_HASHMAP_BUCKET_SIZE;
		__APEDSA_HASHMAP_PUT_PROBE_LOOP_I(i);
		i = (i + 1) % APEDSA_HASHMAP_BUCKET_SIZE;
		__APEDSA_HASHMAP_PUT_PROBE_LOOP_I(i);
		i = (i + 1) % APEDSA_HASHMAP_BUCKET_SIZE;
		__APEDSA_HASHMAP_PUT_PROBE_LOOP_I(i);
		i = (i + 1) % APEDSA_HASHMAP_BUCKET_SIZE;
		__APEDSA_HASHMAP_PUT_PROBE_LOOP_I(i);
		i = (i + 1) % APEDSA_HASHMAP_BUCKET_SIZE;
		__APEDSA_HASHMAP_PUT_PROBE_LOOP_I(i);
		i = (i + 1) % APEDSA_HASHMAP_BUCKET_SIZE;
		__APEDSA_HASHMAP_PUT_PROBE_LOOP_I(i);

		pos += step;
#if defined(APEDSA_HASHMAP_QUADRATIC_PROBING)
		step += APEDSA_HASHMAP_BUCKET_SIZE;
#endif
		pos &= (table->slot_count - 1);
		// __builtin_prefetch(&table->buckets[pos >> APEDSA_HASHMAP_BUCKET_SHIFT], 0, 3);
	}
#undef __APEDSA_HASHMAP_PUT_PROBE_LOOP_I
found_empty_slot:
	if (tombstone >= 0) {
		pos = tombstone;
		table->tombstone_count--;
	} else {
		table->used_count++;
	}
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
	size_t new_slot_count = table ? table->slot_count : APEDSA_HASHMAP_BUCKET_SIZE;
	while (new_slot_count < count) {
		new_slot_count *= 2;
	}
	if (table == NULL || new_slot_count > table->slot_count) {
		table = __apedsa_hashmap_rehash(new_slot_count, table);
		apedsa_da_header(a)->aux = table;
	}
	size_t old_threshold = table->used_count_threshold;
	table->used_count_threshold = SIZE_MAX;
	size_t old_count = apedsa_da_count(a);
	// if (mode >= APEDSA_HASHMAP_MODE_STRING) { pairs = *(char **)pairs; }
	for (int i = 0; i < count; i++) {
		void *pair = ((char *)pairs + i * kv_size);
		char *key = mode >= APEDSA_HASHMAP_MODE_STRING ? *(char **)(char *)pair : (char *)pair;
		char *value = ((char *)pairs + i * kv_size + key_size);
		a = __apedsa_hashmap_put_internal(a, key, key_size, kv_size, mode);
		memcpy((char *)a + (apedsa_da_count(a) - 1) * kv_size, pair, kv_size);
		table = (ApedsaHashIndex *)apedsa_da_header(a)->aux;
	}
	if (table->used_count > old_threshold) {
		a = __apedsa_hashmap_rehash(table->slot_count * 2, table);
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

		size_t i = pos & APEDSA_HASHMAP_BUCKET_MASK;
		__APEDSA_HASHMAP_FIND_PROBE_LOOP_I(i);
		i = (i + 1) % APEDSA_HASHMAP_BUCKET_SIZE;
		__APEDSA_HASHMAP_FIND_PROBE_LOOP_I(i);
		i = (i + 1) % APEDSA_HASHMAP_BUCKET_SIZE;
		__APEDSA_HASHMAP_FIND_PROBE_LOOP_I(i);
		i = (i + 1) % APEDSA_HASHMAP_BUCKET_SIZE;
		__APEDSA_HASHMAP_FIND_PROBE_LOOP_I(i);
		i = (i + 1) % APEDSA_HASHMAP_BUCKET_SIZE;
		__APEDSA_HASHMAP_FIND_PROBE_LOOP_I(i);
		i = (i + 1) % APEDSA_HASHMAP_BUCKET_SIZE;
		__APEDSA_HASHMAP_FIND_PROBE_LOOP_I(i);
		i = (i + 1) % APEDSA_HASHMAP_BUCKET_SIZE;
		__APEDSA_HASHMAP_FIND_PROBE_LOOP_I(i);
		i = (i + 1) % APEDSA_HASHMAP_BUCKET_SIZE;
		__APEDSA_HASHMAP_FIND_PROBE_LOOP_I(i);

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
	APEDSA_ASSERT(table->used_count >= 0);
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
	// else if (table->tombstone_count > table->tombstone_count_threshold)
	// 	apedsa_da_header(a)->aux = __apedsa_hashmap_rehash(table->slot_count, table);

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
