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
 * apealloc.h - v0.1
 */

#ifndef APEALLOC_INCLUDED
#define APEALLOC_INCLUDED

#define APEALLOC_VERSION_MAJOR 0
#define APEALLOC_VERSION_MINOR 1

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define APEALLOC_WINDOWS
#if defined(_WIN64)
#define APEALLOC_WINDOWS_X64
#endif
#elif defined(__linux__) || defined(__unix__)
#define APEALLOC_LINUX
#elif defined(__APPLE__)
#define APEALLOC_APPLE
#endif

#ifndef APEALLOC_MALLOC
#if defined(APEALLOC_REALLOC) || defined(APEALLOC_FREE)
#pragma GCC error "Need to define MALLOC, REALLOC and FREE or none of them"
#else
#include <stdlib.h>
#define APEALLOC_MALLOC malloc
#define APEALLOC_REALLOC realloc
#define APEALLOC_FREE free
#endif
#else
#if !defined(APEALLOC_REALLOC) || !defined(APEALLOC_FREE)
#pragma GCC error "Need to define MALLOC, REALLOC and FREE or none of them"
#endif
#endif

#ifndef APEALLOC_ASSERT
#ifdef APEALLOC_USE_STDLIB_ASSERT
#include <assert.h>
#define APEALLOC_ASSERT(c) assert(c)
#else
#include <stdio.h>
#include <stdlib.h>
#define APEALLOC_ASSERT(c)                                                                 \
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

#if defined(APEALLOC_STRIP_PREFIX)

#endif

#endif

#ifdef APEALLOC_IMPLEMENTATION

#ifndef APEALLOC_IMPLEMENTATION_INCLUDED
#define APEALLOC_IMPLEMENTATION_INCLUDED

#include <string.h>

/* User can define a custom function prefix (eg. static) */
#ifndef APEALLOC_DEF
#define APEALLOC_DEF
#endif

/* Can also define custom private function prefix */
#ifndef APEALLOC_PRIVATE
#define APEALLOC_PRIVATE static
#endif

#ifndef APEALLOC_TRUE
#define APEALLOC_TRUE (1)
#define APEALLOC_FALSE (0)
#else
#if !defined(APEALLOC_FALSE)
#pragma GCC error "Need to define both APEALLOC_TRUE and APEALLOC_FALSE or neither"
#endif
#endif

/* BEGIN pool.c */

typedef struct ApeallocSubblockHeader {
	int is_free;
	size_t index;
	size_t size;
	struct ApeallocSubblockHeader *next;
	void *data;
} ApeallocSubblockHeader;

typedef struct ApeallocPoolBlockHeader {
	size_t size; // fixed size, defined at creation
	size_t subblock_count;
	size_t used_subblock_count;
	struct ApeallocPoolBlockHeader *next; // we have to do this so pointers remain valid
	ApeallocSubblockHeader *first_free;
	ApeallocSubblockHeader *subblocks;
} ApeallocPoolBlockHeader;

typedef struct {
	size_t block_size;
	size_t block_count;
	size_t used_block_count;
	size_t used_block_count_threshold;
	ApeallocPoolBlockHeader *first;
	ApeallocPoolBlockHeader *last;
} ApeallocPool;

APEALLOC_DEF void *apealloc_pool_create(size_t block_size, size_t block_count)
{
	ApeallocPool *pool = (ApeallocPool *)APEALLOC_MALLOC(sizeof(ApeallocPool));
	pool->block_size = block_size;
	pool->block_count = block_count;
	pool->used_block_count = 0;
	pool->used_block_count_threshold = block_count * 3 / 4;
	for (size_t i = 0; i < block_count; i++) {
		ApeallocPoolBlockHeader *block = (ApeallocPoolBlockHeader *)APEALLOC_MALLOC(sizeof(ApeallocPoolBlockHeader) +
											    (sizeof(ApeallocSubblockHeader) + block_size));
		memset(block, 0, sizeof(ApeallocPoolBlockHeader) + (sizeof(ApeallocSubblockHeader) + block_size));
		if (i == 0)
			pool->first = block;
		block->size = block_size;
		block->subblock_count = 1;
		block->used_subblock_count = 0;
		block->next = NULL;
		block->first_free = block->subblocks;
		block->subblocks = (ApeallocSubblockHeader *)((char *)block + sizeof(ApeallocPoolBlockHeader));
		block->subblocks->is_free = 1;
		block->subblocks->index = i;
		block->subblocks->size = block_size;
		block->subblocks->next = NULL;
		block->subblocks->data = (void *)((char *)block->subblocks + sizeof(ApeallocSubblockHeader));
		if (pool->last)
			pool->last->next = block;
		pool->last = block;
	}
	return pool;
}

#define APEALLOC_POOL_DEFAULT_BLOCK_SIZE 4096
#define APEALLOC_POOL_DEFAULT_BLOCK_COUNT 8

APEALLOC_PRIVATE ApeallocPool *apealloc_pool_add_blocks(ApeallocPool *p, size_t count)
{
	if (!p) {
		size_t c = APEALLOC_POOL_DEFAULT_BLOCK_COUNT;
		while (c < count)
			c *= 2;
		p = apealloc_pool_create(APEALLOC_POOL_DEFAULT_BLOCK_SIZE, c);
		return p;
	}
	if (count < p->block_count)
		return p;
	for (size_t i = p->block_count; i < count; i++) {
		ApeallocPoolBlockHeader *new_block = (ApeallocPoolBlockHeader *)APEALLOC_MALLOC(
			sizeof(ApeallocPoolBlockHeader) + (sizeof(ApeallocSubblockHeader) + p->block_size));
		memset(new_block, 0, sizeof(ApeallocPoolBlockHeader) + (sizeof(ApeallocSubblockHeader) + p->block_size));
		new_block->size = p->block_size;
		new_block->subblock_count = 1;
		new_block->used_subblock_count = 0;
		new_block->next = NULL;
		new_block->first_free = new_block->subblocks;
		new_block->subblocks = (ApeallocSubblockHeader *)((char *)new_block + sizeof(ApeallocPoolBlockHeader));
		new_block->subblocks->is_free = 1;
		new_block->subblocks->index = i;
		new_block->subblocks->size = p->block_size;
		new_block->subblocks->next = NULL;
		new_block->subblocks->data = (void *)((char *)new_block->subblocks + sizeof(ApeallocSubblockHeader));
		if (p->last)
			p->last->next = new_block;
		p->last = new_block;
	}
	p->block_count = count;
	return p;
}

APEALLOC_DEF void *apealloc_pool_alloc(void *pool, size_t size)
{
	if (pool == NULL) {
		pool = apealloc_pool_create(APEALLOC_POOL_DEFAULT_BLOCK_SIZE, APEALLOC_POOL_DEFAULT_BLOCK_COUNT);
	}
	ApeallocPool *p = (ApeallocPool *)pool;
	if (p->used_block_count >= p->used_block_count_threshold) {
		apealloc_pool_add_blocks(p, p->block_count * 2);
	}
	ApeallocPoolBlockHeader *block = p->first;
	while (block) {
		if (block->first_free) {
			ApeallocSubblockHeader *subblock = block->first_free;
			if (subblock->size >= size) {
				//TODO: split subblock if needed
				ApeallocSubblockHeader *new_subblock = APEALLOC_MALLOC(sizeof(ApeallocSubblockHeader) + size);
				new_subblock->is_free = 0;
			}
		}
	}
}
/* END pool.c */

#endif

#endif
