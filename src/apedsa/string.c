#include "apedsa_internal.h"

#ifndef APEDSA_STRING_ARENA_BLOCKSIZE_MIN
#define APEDSA_STRING_ARENA_BLOCKSIZE_MIN 512
#endif
#ifndef APEDSA_STRING_ARENA_BLOCKSIZE_MAX
#define APEDSA_STRING_ARENA_BLOCKSIZE_MAX 1 << 20
#endif

APEDSA_DEF char *apedsa_string_arena_alloc(ApedsaStringArena *arena, char *str) {
	char *p;
	size_t len = strlen(str) + 1;
	if (len > arena->remaining) {
		size_t blocksize = arena->block;
		blocksize = (size_t)(APEDSA_STRING_ARENA_BLOCKSIZE_MIN) << (blocksize >> 1);
		if (blocksize < (size_t)(APEDSA_STRING_ARENA_BLOCKSIZE_MAX)) ++arena->block;
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

APEDSA_DEF void apedsa_string_arena_reset(ApedsaStringArena *arena) {
	ApedsaStringBlock *x, *y;
	x = arena->blocks;
	while (x) {
		y = x->next;
		APEDSA_FREE(x);
		x = y;
	}
	memset(arena, 0, sizeof(*arena));
}
