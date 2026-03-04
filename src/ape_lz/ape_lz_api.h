#ifndef APE_LZ_INCLUDED
#define APE_LZ_INCLUDED

#define APE_LZ_VERSION_MAJOR 0
#define APE_LZ_VERSION_MINOR 1

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define APE_LZ_WINDOWS
#if defined(_WIN64)
#define APE_LZ_WINDOWS_X64
#endif
#elif defined(__linux__) || defined(__unix__)
#define APE_LZ_LINUX
#elif defined(__APPLE__)
#define APE_LZ_APPLE
#endif

#ifndef APE_LZ_MALLOC
#if defined(APE_LZ_REALLOC) || defined(APE_LZ_FREE)
#pragma GCC error "Need to define MALLOC, REALLOC and FREE or none of them"
#else
#include <stdlib.h>
#define APE_LZ_MALLOC malloc
#define APE_LZ_REALLOC realloc
#define APE_LZ_FREE free
#endif
#else
#if !defined(APE_LZ_REALLOC) || !defined(APE_LZ_FREE)
#pragma GCC error "Need to define MALLOC, REALLOC and FREE or none of them"
#endif
#endif

#ifndef APE_LZ_ASSERT
#ifdef APE_LZ_USE_STDLIB_ASSERT
#include <assert.h>
#define APE_LZ_ASSERT(c) assert(c)
#else
#include <stdio.h>
#include <stdlib.h>
#define APE_LZ_ASSERT(c)                                                                   \
	if (!(c)) {                                                                        \
		fprintf(stderr, "%s:%d Assertion '%s' failed\n", __FILE__, __LINE__, ##c); \
		exit(1);                                                                   \
	}
#endif
#endif

#include <stdint.h>
#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

/* Error codes returned by ape_lz4 functions */
typedef enum {
	APE_LZ4_OK = 0,			   /* Success */
	APE_LZ4_ERR_NULL_INPUT = -1,	   /* NULL pointer passed as input */
	APE_LZ4_ERR_NULL_OUTPUT = -2,	   /* NULL pointer passed as output */
	APE_LZ4_ERR_INPUT_TOO_LARGE = -3,  /* Input exceeds maximum supported size */
	APE_LZ4_ERR_OUTPUT_TOO_SMALL = -4, /* Output buffer too small for result */
	APE_LZ4_ERR_ALLOC_FAILED = -5,	   /* Memory allocation failed */
	APE_LZ4_ERR_BAD_MAGIC = -6,	   /* Frame magic number mismatch */
	APE_LZ4_ERR_BAD_VERSION = -7,	   /* Unsupported frame version */
	APE_LZ4_ERR_BAD_CHECKSUM = -8,	   /* Header, block, or content checksum mismatch */
	APE_LZ4_ERR_BAD_BLOCK = -9,	   /* Block size invalid or exceeds maximum */
	APE_LZ4_ERR_CORRUPT = -10,	   /* Compressed data is malformed (bad offset, truncated) */
	APE_LZ4_ERR_NO_ENDMARK = -11,	   /* Frame missing end mark */
} ApeLZ4Error;

/* Options for frame-level encode/decode */
typedef struct {
	int block_checksum;   /* 1 = write/verify per-block xxHash32 checksums */
	int content_checksum; /* 1 = write/verify content xxHash32 checksum */
	int content_size;     /* 1 = store original content size in frame header */
} ApeLZ4Options;

/* Result of a compress or decompress operation */
typedef struct {
	ApeLZ4Error error; /* APE_LZ4_OK on success, negative on failure */
	uint8_t *data;	   /* Output buffer (caller must APE_LZ_FREE) */
	size_t size;	   /* Bytes written to data */
} ApeLZ4Result;

/*
 * Compress raw bytes into an LZ4 frame.
 *
 * src:     input data
 * src_len: input length in bytes
 * opts:    compression options (NULL for defaults)
 *
 * Returns an ApeLZ4Result. On success, .data is a heap-allocated buffer
 * containing the LZ4 frame (caller must free with APE_LZ_FREE).
 * On failure, .error is set and .data is NULL.
 */
ApeLZ4Result ape_lz4_compress(const uint8_t *src, size_t src_len, const ApeLZ4Options *opts);

/*
 * Decompress an LZ4 frame back to raw bytes.
 *
 * src:     compressed LZ4 frame data
 * src_len: compressed data length in bytes
 *
 * Returns an ApeLZ4Result. On success, .data is a heap-allocated buffer
 * containing the decompressed data (caller must free with APE_LZ_FREE).
 * On failure, .error is set and .data is NULL.
 */
ApeLZ4Result ape_lz4_decompress(const uint8_t *src, size_t src_len);

/*
 * Return a human-readable string for an ApeLZ4Error code.
 */
const char *ape_lz4_error_string(ApeLZ4Error err);

#if defined(__cplusplus)
}
#endif

#if defined(APE_LZ_STRIP_PREFIX)

#endif

#endif
