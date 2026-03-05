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
 * ape_lz.h - v0.1
 *
 * Example usage:
 *     #define APE_LZ_IMPLEMENTATION
 *     #include "ape_lz.h"
 * 
 *     int main(void) {
 *         const char *input = "Hello, world!";
 *         ApeLZ4Result result = ape_lz4_compress((const uint8_t *)input, strlen(input), NULL);
 *         if (result.error != APE_LZ4_OK) {
 *             printf("Compression failed: %s\n", ape_lz4_error_string(result.error));
 *             return 1;
 *         }
 * 
 *         ApeLZ4Result decompressed = ape_lz4_decompress(result.data, result.size);
 *         if (decompressed.error != APE_LZ4_OK) {
 *             printf("Decompression failed: %s\n", ape_lz4_error_string(decompressed.error));
 *             APE_LZ_FREE(decompressed.data);
 *             return 1;
 *         }
 * 
 *         printf("Original: %s\n", input);
 *         printf("Compressed: %s\n", (const char *)result.data);
 *         printf("Decompressed: %s\n", (const char *)decompressed.data);
 * 
 *         APE_LZ_FREE(result.data);
 *         APE_LZ_FREE(decompressed.data);
 *         return 0;
 *     }
 * 
 * API functions:
 *   ape_lz4_compress - Compress raw bytes into an LZ4 frame
 *   ape_lz4_decompress - Decompress an LZ4 frame into raw bytes
 *   ape_lz4_error_string - Returns a human-readable error message
 */

#ifndef APE_LZ_EXTERNAL_APE_PACK
/* --- embedded ape_pack.h begin --- */
#ifndef APE_PACK_IMPLEMENTATION
#define APE_PACK_IMPLEMENTATION
#endif
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
 * ape_pack.h - v0.1
 *
 * Example usage:
 *   #define APE_PACK_IMPLEMENTATION
 *   #include "ape_pack.h"
 * 
 *   int main(void) {
 *       ApePackReader *reader = ape_pack_reader_from("hello world", 11);
 *       char buf[12];
 *       ape_pack_reader_read(reader, buf, 11);
 *       printf("%s\n", buf); // Should print "hello world"
 *       return 0;
 *   }
 * 
 * API:
 * 
 *   Reader:
 *     ape_pack_reader_from - Creates a reader from a buffer
 *     ape_pack_reader_eof - Returns true if the reader is at the end of the buffer
 *     ape_pack_reader_skip - Skips n bytes in the reader
 *     ape_pack_reader_reset - Resets the reader to the beginning of the buffer
 * 
 *     Unsigned integer reads (endianness-aware):
 *       ape_pack_reader_read_u8
 *       ape_pack_reader_read_u16
 *       ape_pack_reader_read_u32
 *       ape_pack_reader_read_u64
 * 
 *     Signed integer reads (endianness-aware):
 *       ape_pack_reader_read_i8
 *       ape_pack_reader_read_i16
 *       ape_pack_reader_read_i32
 *       ape_pack_reader_read_i64
 * 
 *     Floating point reads (endianness-aware):
 *       ape_pack_reader_read_f32
 *       ape_pack_reader_read_f64
 * 
 *     Raw byte reads (no endianness conversion):
 *       ape_pack_reader_read
 * 
 *   Writer:
 *     ape_pack_writer_to - Creates a writer to a buffer
 *     ape_pack_writer_reset - Resets the writer to the beginning of the buffer
 * 
 *     Unsigned integer writes (endianness-aware):
 *       ape_pack_writer_write_u8
 *       ape_pack_writer_write_u16
 *       ape_pack_writer_write_u32
 *       ape_pack_writer_write_u64
 * 
 *     Signed integer writes (endianness-aware):
 *       ape_pack_writer_write_i8
 *       ape_pack_writer_write_i16
 *       ape_pack_writer_write_i32
 *       ape_pack_writer_write_i64
 * 
 *     Floating point writes (endianness-aware):
 *       ape_pack_writer_write_f32
 *       ape_pack_writer_write_f64
 * 
 *     Raw byte writes (no endianness conversion):
 *       ape_pack_writer_write
 */

#ifndef APE_PACK_INCLUDED
#define APE_PACK_INCLUDED

#define APE_PACK_VERSION_MAJOR 0
#define APE_PACK_VERSION_MINOR 1

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define APE_PACK_WINDOWS
#if defined(_WIN64)
#define APE_PACK_WINDOWS_X64
#endif
#elif defined(__linux__) || defined(__unix__)
#define APE_PACK_LINUX
#elif defined(__APPLE__)
#define APE_PACK_APPLE
#endif

#ifndef APE_PACK_MALLOC
#if defined(APE_PACK_REALLOC) || defined(APE_PACK_FREE)
#pragma GCC error "Need to define MALLOC, REALLOC and FREE or none of them"
#else
#include <stdlib.h>
#define APE_PACK_MALLOC malloc
#define APE_PACK_REALLOC realloc
#define APE_PACK_FREE free
#endif
#else
#if !defined(APE_PACK_REALLOC) || !defined(APE_PACK_FREE)
#pragma GCC error "Need to define MALLOC, REALLOC and FREE or none of them"
#endif
#endif

#ifndef APE_PACK_ASSERT
#ifdef APE_PACK_USE_STDLIB_ASSERT
#include <assert.h>
#define APE_PACK_ASSERT(c) assert(c)
#else
#include <stdio.h>
#include <stdlib.h>
#define APE_PACK_ASSERT(c)                                                                 \
	if (!(c)) {                                                                        \
		fprintf(stderr, "%s:%d Assertion '%s' failed\n", __FILE__, __LINE__, ##c); \
		exit(1);                                                                   \
	}
#endif
#endif

#include <stdint.h>

#include <stddef.h>

#define APE_PACK_LITTLE_ENDIAN 0
#define APE_PACK_BIG_ENDIAN 1

#ifndef APE_PACK_LOG_WARN
#define APE_PACK_LOG_WARN(fmt, ...) fprintf(stderr, "APE_PACK WARNING: " fmt "\n", ##__VA_ARGS__)
#endif

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct {
	int endianess; // 0 = little endian, 1 = big endian
	unsigned int size;
	unsigned int offset;
	unsigned char *data;
} ApePackReader;

typedef struct {
	int endianess; // 0 = little endian, 1 = big endian
	unsigned int size;
	unsigned int offset;
	unsigned char *data;
} ApePackWriter;

/* Reader creation and control */
ApePackReader ape_pack_reader_from(void *data, unsigned int size);
int ape_pack_reader_eof(ApePackReader *reader);
void ape_pack_reader_skip(ApePackReader *reader, unsigned int count);
void ape_pack_reader_reset(ApePackReader *reader);

/* Unsigned integer reads (endianness-aware) */
uint8_t ape_pack_reader_read_u8(ApePackReader *reader);
uint16_t ape_pack_reader_read_u16(ApePackReader *reader);
uint32_t ape_pack_reader_read_u32(ApePackReader *reader);
uint64_t ape_pack_reader_read_u64(ApePackReader *reader);

/* Signed integer reads (endianness-aware) */
int8_t ape_pack_reader_read_i8(ApePackReader *reader);
int16_t ape_pack_reader_read_i16(ApePackReader *reader);
int32_t ape_pack_reader_read_i32(ApePackReader *reader);
int64_t ape_pack_reader_read_i64(ApePackReader *reader);

/* Floating point reads (endianness-aware) */
float ape_pack_reader_read_f32(ApePackReader *reader);
double ape_pack_reader_read_f64(ApePackReader *reader);

/* Raw byte reads (no endianness conversion) */
size_t ape_pack_reader_read(ApePackReader *reader, char *out, size_t len);

/* Writer creation and control */
ApePackWriter ape_pack_writer_to(void *data, unsigned int size);
void ape_pack_writer_reset(ApePackWriter *writer);

/* Unsigned integer writes (endianness-aware) */
void ape_pack_writer_write_u8(ApePackWriter *writer, uint8_t value);
void ape_pack_writer_write_u16(ApePackWriter *writer, uint16_t value);
void ape_pack_writer_write_u32(ApePackWriter *writer, uint32_t value);
void ape_pack_writer_write_u64(ApePackWriter *writer, uint64_t value);

/* Signed integer writes (endianness-aware) */
void ape_pack_writer_write_i8(ApePackWriter *writer, int8_t value);
void ape_pack_writer_write_i16(ApePackWriter *writer, int16_t value);
void ape_pack_writer_write_i32(ApePackWriter *writer, int32_t value);
void ape_pack_writer_write_i64(ApePackWriter *writer, int64_t value);

/* Floating point writes (endianness-aware) */
void ape_pack_writer_write_f32(ApePackWriter *writer, float value);
void ape_pack_writer_write_f64(ApePackWriter *writer, double value);

/* Raw byte writes (no endianness conversion) */
void ape_pack_writer_write(ApePackWriter *writer, void *data, size_t len);

#if defined(__cplusplus)
}
#endif

#if defined(APE_PACK_STRIP_PREFIX)

#endif

#endif

#ifdef APE_PACK_IMPLEMENTATION

#ifndef APE_PACK_IMPLEMENTATION_INCLUDED
#define APE_PACK_IMPLEMENTATION_INCLUDED

/* User can define a custom function prefix (eg. static) */
#ifndef APE_PACK_DEF
#define APE_PACK_DEF
#endif

/* Can also define custom private function prefix */
#ifndef APE_PACK_PRIVATE
#define APE_PACK_PRIVATE static
#endif

#ifndef APE_PACK_TRUE
#define APE_PACK_TRUE (1)
#define APE_PACK_FALSE (0)
#else
#if !defined(APE_PACK_FALSE)
#pragma GCC error "Need to define both APE_PACK_TRUE and APE_PACK_FALSE or neither"
#endif
#endif

#include <string.h>

/* BEGIN ape_pack.c */

/* ============================================================================
 * Reader Implementation
 * ============================================================================ */

APE_PACK_DEF ApePackReader ape_pack_reader_from(void *data, unsigned int size)
{
	ApePackReader reader;
	reader.endianess = 0;
	reader.offset = 0;
	reader.data = data;
	reader.size = size;
	return reader;
}

APE_PACK_DEF int ape_pack_reader_eof(ApePackReader *reader)
{
	return reader->offset >= reader->size;
}

APE_PACK_DEF uint8_t ape_pack_reader_read_u8(ApePackReader *reader)
{
	if (reader->offset >= reader->size) {
		APE_PACK_LOG_WARN("read_u8: read past end (offset %u, size %u)", reader->offset, reader->size);
		return 0;
	}
	return reader->data[reader->offset++];
}

APE_PACK_DEF void ape_pack_reader_skip(ApePackReader *reader, unsigned int count)
{
	if (reader->offset + count > reader->size) {
		APE_PACK_LOG_WARN("skip: would exceed bounds (offset %u + %u > size %u)", reader->offset, count, reader->size);
		return;
	}
	reader->offset += count;
}

APE_PACK_DEF void ape_pack_reader_reset(ApePackReader *reader)
{
	reader->offset = 0;
}

APE_PACK_DEF uint16_t ape_pack_reader_read_u16(ApePackReader *reader)
{
	uint16_t result = 0;
	if (reader->endianess) {
		result |= (uint16_t)ape_pack_reader_read_u8(reader) << 8;
		result |= (uint16_t)ape_pack_reader_read_u8(reader);
	} else {
		result |= (uint16_t)ape_pack_reader_read_u8(reader);
		result |= (uint16_t)ape_pack_reader_read_u8(reader) << 8;
	}
	return result;
}

APE_PACK_DEF uint32_t ape_pack_reader_read_u32(ApePackReader *reader)
{
	uint32_t result = 0;
	if (reader->endianess) {
		result |= (uint32_t)ape_pack_reader_read_u8(reader) << 24;
		result |= (uint32_t)ape_pack_reader_read_u8(reader) << 16;
		result |= (uint32_t)ape_pack_reader_read_u8(reader) << 8;
		result |= (uint32_t)ape_pack_reader_read_u8(reader);
	} else {
		result |= (uint32_t)ape_pack_reader_read_u8(reader);
		result |= (uint32_t)ape_pack_reader_read_u8(reader) << 8;
		result |= (uint32_t)ape_pack_reader_read_u8(reader) << 16;
		result |= (uint32_t)ape_pack_reader_read_u8(reader) << 24;
	}
	return result;
}

APE_PACK_DEF uint64_t ape_pack_reader_read_u64(ApePackReader *reader)
{
	uint64_t result = 0;
	if (reader->endianess) {
		result |= (uint64_t)ape_pack_reader_read_u8(reader) << 56;
		result |= (uint64_t)ape_pack_reader_read_u8(reader) << 48;
		result |= (uint64_t)ape_pack_reader_read_u8(reader) << 40;
		result |= (uint64_t)ape_pack_reader_read_u8(reader) << 32;
		result |= (uint64_t)ape_pack_reader_read_u8(reader) << 24;
		result |= (uint64_t)ape_pack_reader_read_u8(reader) << 16;
		result |= (uint64_t)ape_pack_reader_read_u8(reader) << 8;
		result |= (uint64_t)ape_pack_reader_read_u8(reader);
	} else {
		result |= (uint64_t)ape_pack_reader_read_u8(reader);
		result |= (uint64_t)ape_pack_reader_read_u8(reader) << 8;
		result |= (uint64_t)ape_pack_reader_read_u8(reader) << 16;
		result |= (uint64_t)ape_pack_reader_read_u8(reader) << 24;
		result |= (uint64_t)ape_pack_reader_read_u8(reader) << 32;
		result |= (uint64_t)ape_pack_reader_read_u8(reader) << 40;
		result |= (uint64_t)ape_pack_reader_read_u8(reader) << 48;
		result |= (uint64_t)ape_pack_reader_read_u8(reader) << 56;
	}
	return result;
}

APE_PACK_DEF int8_t ape_pack_reader_read_i8(ApePackReader *reader)
{
	return (int8_t)ape_pack_reader_read_u8(reader);
}

APE_PACK_DEF int16_t ape_pack_reader_read_i16(ApePackReader *reader)
{
	return (int16_t)ape_pack_reader_read_u16(reader);
}

APE_PACK_DEF int32_t ape_pack_reader_read_i32(ApePackReader *reader)
{
	return (int32_t)ape_pack_reader_read_u32(reader);
}

APE_PACK_DEF int64_t ape_pack_reader_read_i64(ApePackReader *reader)
{
	return (int64_t)ape_pack_reader_read_u64(reader);
}

APE_PACK_DEF float ape_pack_reader_read_f32(ApePackReader *reader)
{
	float result;
	uint32_t bits = ape_pack_reader_read_u32(reader);
	memcpy(&result, &bits, sizeof(result));
	return result;
}

APE_PACK_DEF double ape_pack_reader_read_f64(ApePackReader *reader)
{
	double result;
	uint64_t bits = ape_pack_reader_read_u64(reader);
	memcpy(&result, &bits, sizeof(result));
	return result;
}

APE_PACK_DEF size_t ape_pack_reader_read(ApePackReader *reader, char *out, size_t len)
{
	if (reader->offset + len > reader->size) {
		APE_PACK_LOG_WARN("read: would exceed bounds (offset %u + %zu > size %u)", reader->offset, len, reader->size);
		return 0;
	}
	size_t read = 0;
	while (read < len) {
		out[read] = reader->data[reader->offset++];
		read++;
	}
	return read;
}

/* ============================================================================
 * Writer Implementation
 * ============================================================================ */

APE_PACK_DEF ApePackWriter ape_pack_writer_to(void *data, unsigned int size)
{
	ApePackWriter writer;
	writer.endianess = 0;
	writer.offset = 0;
	writer.size = size;
	writer.data = data;
	return writer;
}

APE_PACK_DEF void ape_pack_writer_reset(ApePackWriter *writer)
{
	writer->offset = 0;
}

APE_PACK_DEF void ape_pack_writer_write_u8(ApePackWriter *writer, uint8_t value)
{
	if (writer->offset >= writer->size) {
		APE_PACK_LOG_WARN("write_u8: write past end (offset %u, size %u)", writer->offset, writer->size);
		return;
	}
	writer->data[writer->offset++] = value;
}

APE_PACK_DEF void ape_pack_writer_write_u16(ApePackWriter *writer, uint16_t value)
{
	if (writer->endianess) {
		ape_pack_writer_write_u8(writer, value >> 8);
		ape_pack_writer_write_u8(writer, value);
	} else {
		ape_pack_writer_write_u8(writer, value);
		ape_pack_writer_write_u8(writer, value >> 8);
	}
}

APE_PACK_DEF void ape_pack_writer_write_u32(ApePackWriter *writer, uint32_t value)
{
	if (writer->endianess) {
		ape_pack_writer_write_u8(writer, value >> 24);
		ape_pack_writer_write_u8(writer, value >> 16);
		ape_pack_writer_write_u8(writer, value >> 8);
		ape_pack_writer_write_u8(writer, value);
	} else {
		ape_pack_writer_write_u8(writer, value);
		ape_pack_writer_write_u8(writer, value >> 8);
		ape_pack_writer_write_u8(writer, value >> 16);
		ape_pack_writer_write_u8(writer, value >> 24);
	}
}

APE_PACK_DEF void ape_pack_writer_write_u64(ApePackWriter *writer, uint64_t value)
{
	if (writer->endianess) {
		ape_pack_writer_write_u8(writer, value >> 56);
		ape_pack_writer_write_u8(writer, value >> 48);
		ape_pack_writer_write_u8(writer, value >> 40);
		ape_pack_writer_write_u8(writer, value >> 32);
		ape_pack_writer_write_u8(writer, value >> 24);
		ape_pack_writer_write_u8(writer, value >> 16);
		ape_pack_writer_write_u8(writer, value >> 8);
		ape_pack_writer_write_u8(writer, value);
	} else {
		ape_pack_writer_write_u8(writer, value);
		ape_pack_writer_write_u8(writer, value >> 8);
		ape_pack_writer_write_u8(writer, value >> 16);
		ape_pack_writer_write_u8(writer, value >> 24);
		ape_pack_writer_write_u8(writer, value >> 32);
		ape_pack_writer_write_u8(writer, value >> 40);
		ape_pack_writer_write_u8(writer, value >> 48);
		ape_pack_writer_write_u8(writer, value >> 56);
	}
}

APE_PACK_DEF void ape_pack_writer_write_i8(ApePackWriter *writer, int8_t value)
{
	ape_pack_writer_write_u8(writer, (uint8_t)value);
}

APE_PACK_DEF void ape_pack_writer_write_i16(ApePackWriter *writer, int16_t value)
{
	ape_pack_writer_write_u16(writer, (uint16_t)value);
}

APE_PACK_DEF void ape_pack_writer_write_i32(ApePackWriter *writer, int32_t value)
{
	ape_pack_writer_write_u32(writer, (uint32_t)value);
}

APE_PACK_DEF void ape_pack_writer_write_i64(ApePackWriter *writer, int64_t value)
{
	ape_pack_writer_write_u64(writer, (uint64_t)value);
}

APE_PACK_DEF void ape_pack_writer_write_f32(ApePackWriter *writer, float value)
{
	uint32_t bits;
	memcpy(&bits, &value, sizeof(bits));
	ape_pack_writer_write_u32(writer, bits);
}

APE_PACK_DEF void ape_pack_writer_write_f64(ApePackWriter *writer, double value)
{
	uint64_t bits;
	memcpy(&bits, &value, sizeof(bits));
	ape_pack_writer_write_u64(writer, bits);
}

APE_PACK_DEF void ape_pack_writer_write(ApePackWriter *writer, void *data, size_t len)
{
	size_t i;
	for (i = 0; i < len; i++) {
		ape_pack_writer_write_u8(writer, ((unsigned char *)data)[i]);
	}
}
/* END ape_pack.c */

#endif

#endif

/* --- embedded ape_pack.h end --- */
#else
#include "ape_pack.h"
#endif /* APE_LZ_EXTERNAL_APE_PACK */
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

#ifdef APE_LZ_IMPLEMENTATION

#ifndef APE_LZ_IMPLEMENTATION_INCLUDED
#define APE_LZ_IMPLEMENTATION_INCLUDED

/* User can define a custom function prefix (eg. static) */
#ifndef APE_LZ_DEF
#define APE_LZ_DEF
#endif

/* Can also define custom private function prefix */
#ifndef APE_LZ_PRIVATE
#define APE_LZ_PRIVATE static
#endif

#ifndef APE_LZ_TRUE
#define APE_LZ_TRUE (1)
#define APE_LZ_FALSE (0)
#else
#if !defined(APE_LZ_FALSE)
#pragma GCC error "Need to define both APE_LZ_TRUE and APE_LZ_FALSE or neither"
#endif
#endif

#include <string.h>
#include <stdint.h>

/* BEGIN ape_lz.c */

/* ============================================================================
 * Constants
 * ============================================================================ */

#define APE_LZ4_MAGIC 0x184D2204

#define APE_LZ4_ENDMARK 0x00000000

/* Block maximum sizes */
#define APE_LZ4_BLKMAX_64K (64 * 1024)
#define APE_LZ4_BLKMAX_256K (256 * 1024)
#define APE_LZ4_BLKMAX_1M (1024 * 1024)
#define APE_LZ4_BLKMAX_4M (4 * 1024 * 1024)

/* Frame descriptor flag bits */
#define APE_LZ4_FLG_VERSION (0x1 << 6)
#define APE_LZ4_FLG_INDEP (0x1 << 5)
#define APE_LZ4_FLG_BLKCHKSUM (0x1 << 4)
#define APE_LZ4_FLG_SIZE (0x1 << 3)
#define APE_LZ4_FLG_FULLCHKSUM (0x1 << 2)
#define APE_LZ4_FLG_DICTID (0x1)

/* BD byte: block max size field (bits 6-4) */
#define APE_LZ4_BD_4M (0x7 << 4)

/* LZ4 block format constants */
#define APE_LZ4_MINMATCH 4
#define APE_LZ4_LASTLITERALS 5
#define APE_LZ4_MFLIMIT 12
#define APE_LZ4_MAX_OFFSET 65535
#define APE_LZ4_HASHTABLE_BITS 16
#define APE_LZ4_HASHTABLE_SIZE (1 << APE_LZ4_HASHTABLE_BITS)

/* Maximum input size: slightly under 2 GB to avoid overflow */
#define APE_LZ4_MAX_INPUT_SIZE 0x7E000000

/* ============================================================================
 * xxHash32 (for LZ4 frame checksums)
 * ============================================================================ */

#define XXH32_PRIME1 0x9E3779B1U
#define XXH32_PRIME2 0x85EBCA77U
#define XXH32_PRIME3 0xC2B2AE3DU
#define XXH32_PRIME4 0x27D4EB2FU
#define XXH32_PRIME5 0x165667B1U

APE_LZ_PRIVATE uint32_t xxh32_rotl(uint32_t x, int r)
{
	return (x << r) | (x >> (32 - r));
}

APE_LZ_PRIVATE uint32_t xxh32_read32(const uint8_t *p)
{
	uint32_t v;
	memcpy(&v, p, 4);
	return v;
}

APE_LZ_PRIVATE uint32_t xxh32_round(uint32_t acc, uint32_t input)
{
	acc += input * XXH32_PRIME2;
	acc = xxh32_rotl(acc, 13);
	acc *= XXH32_PRIME1;
	return acc;
}

APE_LZ_PRIVATE uint32_t xxh32(const uint8_t *data, size_t len, uint32_t seed)
{
	const uint8_t *p = data;
	const uint8_t *end = data + len;
	uint32_t h;

	if (len >= 16) {
		const uint8_t *limit = end - 16;
		uint32_t v1 = seed + XXH32_PRIME1 + XXH32_PRIME2;
		uint32_t v2 = seed + XXH32_PRIME2;
		uint32_t v3 = seed;
		uint32_t v4 = seed - XXH32_PRIME1;
		do {
			v1 = xxh32_round(v1, xxh32_read32(p));
			p += 4;
			v2 = xxh32_round(v2, xxh32_read32(p));
			p += 4;
			v3 = xxh32_round(v3, xxh32_read32(p));
			p += 4;
			v4 = xxh32_round(v4, xxh32_read32(p));
			p += 4;
		} while (p <= limit);
		h = xxh32_rotl(v1, 1) + xxh32_rotl(v2, 7) + xxh32_rotl(v3, 12) + xxh32_rotl(v4, 18);
	} else {
		h = seed + XXH32_PRIME5;
	}

	h += (uint32_t)len;

	while (p + 4 <= end) {
		h += xxh32_read32(p) * XXH32_PRIME3;
		h = xxh32_rotl(h, 17) * XXH32_PRIME4;
		p += 4;
	}

	while (p < end) {
		h += (*p) * XXH32_PRIME5;
		h = xxh32_rotl(h, 11) * XXH32_PRIME1;
		p++;
	}

	h ^= h >> 15;
	h *= XXH32_PRIME2;
	h ^= h >> 13;
	h *= XXH32_PRIME3;
	h ^= h >> 16;

	return h;
}

/* ============================================================================
 * LZ4 Block Compression
 * ============================================================================ */

APE_LZ_PRIVATE uint32_t lz4_hash(uint32_t val)
{
	return (val * 2654435761U) >> (32 - APE_LZ4_HASHTABLE_BITS);
}

APE_LZ_PRIVATE uint32_t lz4_read32(const uint8_t *p)
{
	uint32_t v;
	memcpy(&v, p, 4);
	return v;
}

APE_LZ_PRIVATE void lz4_write16le(uint8_t *p, uint16_t v)
{
	p[0] = (uint8_t)(v);
	p[1] = (uint8_t)(v >> 8);
}

APE_LZ_PRIVATE uint16_t lz4_read16le(const uint8_t *p)
{
	return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

/*
 * Compress a single LZ4 block.
 * Returns compressed size, or 0 if the data is not compressible
 * (output would be >= input).
 * dst must be at least src_len + (src_len / 255) + 16 bytes.
 */
APE_LZ_PRIVATE size_t lz4_block_compress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t dst_cap)
{
	if (src_len == 0)
		return 0;
	if (src_len > APE_LZ4_MAX_INPUT_SIZE)
		return 0;

	uint16_t htable[APE_LZ4_HASHTABLE_SIZE];
	memset(htable, 0, sizeof(htable));

	const uint8_t *ip = src;
	const uint8_t *anchor = src;
	const uint8_t *iend = src + src_len;
	const uint8_t *mflimit = iend - APE_LZ4_MFLIMIT;
	const uint8_t *matchlimit = iend - APE_LZ4_LASTLITERALS;

	uint8_t *op = dst;
	uint8_t *oend = dst + dst_cap;

	/* Input too small for any match */
	if (src_len < APE_LZ4_MFLIMIT) {
		goto emit_last_literals;
	}

	/* First byte */
	htable[lz4_hash(lz4_read32(ip))] = (uint16_t)(ip - src);
	ip++;

	/* Main compression loop */
	for (;;) {
		const uint8_t *match;
		uint8_t *token_ptr;
		size_t literal_len;
		size_t match_len;
		uint16_t offset;

		/* Find a match using hash lookup */
		{
			const uint8_t *forwardip = ip;
			unsigned step = 1;
			unsigned skip = 64; /* acceleration */

			do {
				ip = forwardip;
				forwardip += step;
				step = (skip++ >> 6);

				if (forwardip > mflimit)
					goto emit_last_literals;

				uint32_t h = lz4_hash(lz4_read32(ip));
				match = src + htable[h];
				htable[h] = (uint16_t)(ip - src);
			} while (lz4_read32(match) != lz4_read32(ip) || (size_t)(ip - match) > APE_LZ4_MAX_OFFSET);
		}

		/* Emit literals from anchor to ip */
		literal_len = (size_t)(ip - anchor);
		token_ptr = op++;
		if (op >= oend)
			return 0;

		/* Write literal length */
		if (literal_len >= 15) {
			*token_ptr = 0xF0;
			size_t remaining = literal_len - 15;
			while (remaining >= 255) {
				if (op >= oend)
					return 0;
				*op++ = 255;
				remaining -= 255;
			}
			if (op >= oend)
				return 0;
			*op++ = (uint8_t)remaining;
		} else {
			*token_ptr = (uint8_t)(literal_len << 4);
		}

		/* Copy literals */
		if (op + literal_len > oend)
			return 0;
		memcpy(op, anchor, literal_len);
		op += literal_len;

		/* Encode match */
		for (;;) {
			/* Offset (little-endian) */
			offset = (uint16_t)(ip - match);
			if (op + 2 > oend)
				return 0;
			lz4_write16le(op, offset);
			op += 2;

			/* Count match length beyond MINMATCH */
			{
				const uint8_t *mp = match + APE_LZ4_MINMATCH;
				const uint8_t *ipp = ip + APE_LZ4_MINMATCH;
				while (ipp < matchlimit && *ipp == *mp) {
					ipp++;
					mp++;
				}
				match_len = (size_t)(ipp - ip) - APE_LZ4_MINMATCH;
				ip = ipp;
			}

			/* Write match length into token */
			if (match_len >= 15) {
				*token_ptr |= 0x0F;
				size_t remaining = match_len - 15;
				while (remaining >= 255) {
					if (op >= oend)
						return 0;
					*op++ = 255;
					remaining -= 255;
				}
				if (op >= oend)
					return 0;
				*op++ = (uint8_t)remaining;
			} else {
				*token_ptr |= (uint8_t)match_len;
			}

			anchor = ip;

			/* Check end of block */
			if (ip > mflimit)
				goto emit_last_literals;

			/* Update hash table */
			htable[lz4_hash(lz4_read32(ip - 2))] = (uint16_t)(ip - 2 - src);

			/* Try next match at current position */
			{
				uint32_t h = lz4_hash(lz4_read32(ip));
				match = src + htable[h];
				htable[h] = (uint16_t)(ip - src);
			}
			if (lz4_read32(match) != lz4_read32(ip) || (size_t)(ip - match) > APE_LZ4_MAX_OFFSET)
				break;

			/* Chain: emit zero-literal token for next match */
			token_ptr = op++;
			if (op >= oend)
				return 0;
			*token_ptr = 0;
		}

		ip++;
		htable[lz4_hash(lz4_read32(ip - 1))] = (uint16_t)(ip - 1 - src);
	}

emit_last_literals:;
	/* Emit last literals (mandatory — the last sequence has no match) */
	size_t last_literal_len = (size_t)(iend - anchor);
	uint8_t *last_token = op++;
	if (op >= oend)
		return 0;

	if (last_literal_len >= 15) {
		*last_token = 0xF0;
		size_t remaining = last_literal_len - 15;
		while (remaining >= 255) {
			if (op >= oend)
				return 0;
			*op++ = 255;
			remaining -= 255;
		}
		if (op >= oend)
			return 0;
		*op++ = (uint8_t)remaining;
	} else {
		*last_token = (uint8_t)(last_literal_len << 4);
	}

	if (op + last_literal_len > oend)
		return 0;
	memcpy(op, anchor, last_literal_len);
	op += last_literal_len;

	return (size_t)(op - dst);
}

/* ============================================================================
 * LZ4 Block Decompression
 * ============================================================================ */

/*
 * Decompress a single LZ4 block.
 * Returns decompressed size, or 0 on error.
 */
APE_LZ_PRIVATE size_t lz4_block_decompress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t dst_cap)
{
	const uint8_t *ip = src;
	const uint8_t *iend = src + src_len;
	uint8_t *op = dst;
	uint8_t *oend = dst + dst_cap;

	for (;;) {
		if (ip >= iend)
			return 0; /* truncated */

		/* Read token */
		uint8_t token = *ip++;
		size_t literal_len = (token >> 4) & 0x0F;
		size_t match_len;

		/* Decode literal length */
		if (literal_len == 15) {
			uint8_t s;
			do {
				if (ip >= iend)
					return 0;
				s = *ip++;
				literal_len += s;
			} while (s == 255);
		}

		/* Copy literals */
		if (ip + literal_len > iend)
			return 0;
		if (op + literal_len > oend)
			return 0;
		memcpy(op, ip, literal_len);
		ip += literal_len;
		op += literal_len;

		/* Check if this was the last sequence (no offset follows) */
		if (ip >= iend)
			break;

		/* Read offset (2 bytes, little-endian) */
		if (ip + 2 > iend)
			return 0;
		uint16_t offset = lz4_read16le(ip);
		ip += 2;

		if (offset == 0)
			return 0; /* invalid offset */

		const uint8_t *match = op - offset;
		if (match < dst)
			return 0; /* offset points before output buffer */

		/* Decode match length */
		match_len = (token & 0x0F) + APE_LZ4_MINMATCH;
		if ((token & 0x0F) == 15) {
			uint8_t s;
			do {
				if (ip >= iend)
					return 0;
				s = *ip++;
				match_len += s;
			} while (s == 255);
		}

		/* Copy match (byte-by-byte to handle overlapping correctly) */
		if (op + match_len > oend)
			return 0;
		{
			size_t i;
			for (i = 0; i < match_len; i++) {
				op[i] = match[i];
			}
		}
		op += match_len;
	}

	return (size_t)(op - dst);
}

/* ============================================================================
 * Frame Encode
 * ============================================================================ */

APE_LZ_DEF ApeLZ4Result ape_lz4_compress(const uint8_t *src, size_t src_len, const ApeLZ4Options *opts)
{
	ApeLZ4Result result;
	result.data = NULL;
	result.size = 0;
	result.error = APE_LZ4_OK;

	if (src == NULL && src_len > 0) {
		result.error = APE_LZ4_ERR_NULL_INPUT;
		return result;
	}

	if (src_len > APE_LZ4_MAX_INPUT_SIZE) {
		result.error = APE_LZ4_ERR_INPUT_TOO_LARGE;
		return result;
	}

	ApeLZ4Options defaults;
	defaults.block_checksum = 0;
	defaults.content_checksum = 0;
	defaults.content_size = 0;
	if (opts == NULL)
		opts = &defaults;

	size_t block_max = APE_LZ4_BLKMAX_4M;

	/*
	 * Worst-case output size:
	 * 4 (magic) + 3 (FLG+BD+HC) + 8 (content_size) +
	 * for each block: 4 (block_size) + block_data + 4 (block_checksum) +
	 * 4 (endmark) + 4 (content_checksum)
	 *
	 * block_data worst case: src_len + (src_len/255) + 16
	 */
	size_t n_blocks = (src_len + block_max - 1) / block_max;
	if (n_blocks == 0)
		n_blocks = 1;
	size_t out_cap = 4 + 3 + 8 + n_blocks * (4 + block_max + (block_max / 255) + 16 + 4) + 4 + 4;

	uint8_t *out = (uint8_t *)APE_LZ_MALLOC(out_cap);
	if (out == NULL) {
		result.error = APE_LZ4_ERR_ALLOC_FAILED;
		return result;
	}

	ApePackWriter w = ape_pack_writer_to(out, (unsigned int)(out_cap > 0xFFFFFFFF ? 0xFFFFFFFF : out_cap));

	/* --- Frame Header --- */

	/* Magic number (little-endian) */
	ape_pack_writer_write_u32(&w, APE_LZ4_MAGIC);

	/* FLG byte */
	uint8_t flg = APE_LZ4_FLG_VERSION | APE_LZ4_FLG_INDEP; /* version 01, independent blocks */
	if (opts->block_checksum)
		flg |= APE_LZ4_FLG_BLKCHKSUM;
	if (opts->content_checksum)
		flg |= APE_LZ4_FLG_FULLCHKSUM;
	if (opts->content_size)
		flg |= APE_LZ4_FLG_SIZE;

	/* BD byte: block max size = 4MB (value 7 in bits 6-4) */
	uint8_t bd = APE_LZ4_BD_4M;

	/* Compute header checksum over FLG and BD (and content_size if present) */
	/* HC = (xxh32(descriptor, 0) >> 8) & 0xFF */
	uint8_t desc_buf[10]; /* FLG + BD + up to 8 bytes content_size */
	size_t desc_len = 0;
	desc_buf[desc_len++] = flg;
	desc_buf[desc_len++] = bd;
	if (opts->content_size) {
		/* 8-byte little-endian content size */
		uint64_t cs = (uint64_t)src_len;
		size_t i;
		for (i = 0; i < 8; i++) {
			desc_buf[desc_len++] = (uint8_t)(cs & 0xFF);
			cs >>= 8;
		}
	}
	uint8_t hc = (uint8_t)((xxh32(desc_buf, desc_len, 0) >> 8) & 0xFF);

	ape_pack_writer_write_u8(&w, flg);
	ape_pack_writer_write_u8(&w, bd);
	if (opts->content_size) {
		/* Write 8-byte little-endian content size */
		uint64_t cs = (uint64_t)src_len;
		size_t i;
		for (i = 0; i < 8; i++) {
			ape_pack_writer_write_u8(&w, (uint8_t)(cs & 0xFF));
			cs >>= 8;
		}
	}
	ape_pack_writer_write_u8(&w, hc);

	/* --- Data Blocks --- */

	/* Temporary buffer for compressed block data */
	size_t comp_cap = block_max + (block_max / 255) + 16;
	uint8_t *comp_buf = (uint8_t *)APE_LZ_MALLOC(comp_cap);
	if (comp_buf == NULL) {
		APE_LZ_FREE(out);
		result.error = APE_LZ4_ERR_ALLOC_FAILED;
		return result;
	}

	size_t offset = 0;
	while (offset < src_len) {
		size_t chunk = src_len - offset;
		if (chunk > block_max)
			chunk = block_max;

		size_t comp_size = lz4_block_compress(src + offset, chunk, comp_buf, comp_cap);

		if (comp_size > 0 && comp_size < chunk) {
			/* Compressed block (high bit clear) */
			ape_pack_writer_write_u32(&w, (uint32_t)comp_size);
			ape_pack_writer_write(&w, comp_buf, comp_size);
			if (opts->block_checksum) {
				uint32_t bchk = xxh32(comp_buf, comp_size, 0);
				ape_pack_writer_write_u32(&w, bchk);
			}
		} else {
			/* Uncompressed block (high bit set) */
			uint32_t block_size = (uint32_t)chunk | 0x80000000U;
			ape_pack_writer_write_u32(&w, block_size);
			ape_pack_writer_write(&w, (void *)(src + offset), chunk);
			if (opts->block_checksum) {
				uint32_t bchk = xxh32(src + offset, chunk, 0);
				ape_pack_writer_write_u32(&w, bchk);
			}
		}

		offset += chunk;
	}

	/* Handle empty input: emit a zero-length uncompressed block */
	if (src_len == 0) { /* Nothing was emitted above, endmark follows directly */
	}

	APE_LZ_FREE(comp_buf);

	/* --- EndMark --- */
	ape_pack_writer_write_u32(&w, APE_LZ4_ENDMARK);

	/* --- Content Checksum --- */
	if (opts->content_checksum) {
		uint32_t cchk = xxh32(src, src_len, 0);
		ape_pack_writer_write_u32(&w, cchk);
	}

	result.data = out;
	result.size = w.offset;
	return result;
}

/* ============================================================================
 * Frame Decode
 * ============================================================================ */

APE_LZ_DEF ApeLZ4Result ape_lz4_decompress(const uint8_t *src, size_t src_len)
{
	ApeLZ4Result result;
	result.data = NULL;
	result.size = 0;
	result.error = APE_LZ4_OK;

	if (src == NULL) {
		result.error = APE_LZ4_ERR_NULL_INPUT;
		return result;
	}

	if (src_len < 7) { /* minimum: magic(4) + flg(1) + bd(1) + hc(1) */
		result.error = APE_LZ4_ERR_CORRUPT;
		return result;
	}

	ApePackReader r = ape_pack_reader_from((void *)(uintptr_t)src, (unsigned int)src_len);

	/* --- Frame Header --- */
	uint32_t magic = ape_pack_reader_read_u32(&r);
	if (magic != APE_LZ4_MAGIC) {
		result.error = APE_LZ4_ERR_BAD_MAGIC;
		return result;
	}

	uint8_t flg = ape_pack_reader_read_u8(&r);
	uint8_t bd = ape_pack_reader_read_u8(&r);
	(void)bd; /* we accept any block max size */

	/* Check version bits (bits 7-6 must be 01) */
	if ((flg >> 6) != 1) {
		result.error = APE_LZ4_ERR_BAD_VERSION;
		return result;
	}

	int has_block_checksum = (flg & APE_LZ4_FLG_BLKCHKSUM) != 0;
	int has_content_checksum = (flg & APE_LZ4_FLG_FULLCHKSUM) != 0;
	int has_content_size = (flg & APE_LZ4_FLG_SIZE) != 0;

	/* Descriptor checksum validation */
	uint8_t desc_buf[10];
	size_t desc_len = 0;
	desc_buf[desc_len++] = flg;
	desc_buf[desc_len++] = bd;

	uint64_t content_size = 0;
	if (has_content_size) {
		size_t i;
		for (i = 0; i < 8; i++) {
			uint8_t b = ape_pack_reader_read_u8(&r);
			desc_buf[desc_len++] = b;
			content_size |= (uint64_t)b << (i * 8);
		}
	}

	uint8_t hc = ape_pack_reader_read_u8(&r);
	uint8_t expected_hc = (uint8_t)((xxh32(desc_buf, desc_len, 0) >> 8) & 0xFF);
	if (hc != expected_hc) {
		result.error = APE_LZ4_ERR_BAD_CHECKSUM;
		return result;
	}

	/* --- Allocate output buffer --- */
	size_t out_cap;
	if (has_content_size && content_size > 0) {
		out_cap = (size_t)content_size;
	} else {
		/* No content size: start with 256K and grow */
		out_cap = 256 * 1024;
	}

	uint8_t *out = (uint8_t *)APE_LZ_MALLOC(out_cap);
	if (out == NULL) {
		result.error = APE_LZ4_ERR_ALLOC_FAILED;
		return result;
	}
	size_t out_offset = 0;

	/* --- Data Blocks --- */
	for (;;) {
		if (ape_pack_reader_eof(&r)) {
			result.error = APE_LZ4_ERR_NO_ENDMARK;
			APE_LZ_FREE(out);
			return result;
		}

		uint32_t block_header = ape_pack_reader_read_u32(&r);

		/* EndMark */
		if (block_header == APE_LZ4_ENDMARK)
			break;

		int is_uncompressed = (block_header & 0x80000000U) != 0;
		uint32_t block_data_size = block_header & 0x7FFFFFFFU;

		if (block_data_size == 0 || block_data_size > APE_LZ4_BLKMAX_4M) {
			result.error = APE_LZ4_ERR_BAD_BLOCK;
			APE_LZ_FREE(out);
			return result;
		}

		if (r.offset + block_data_size > r.size) {
			result.error = APE_LZ4_ERR_CORRUPT;
			APE_LZ_FREE(out);
			return result;
		}

		const uint8_t *block_data = src + r.offset;
		ape_pack_reader_skip(&r, block_data_size);

		/* Block checksum */
		if (has_block_checksum) {
			if (r.offset + 4 > r.size) {
				result.error = APE_LZ4_ERR_CORRUPT;
				APE_LZ_FREE(out);
				return result;
			}
			uint32_t bchk = ape_pack_reader_read_u32(&r);
			uint32_t expected_bchk = xxh32(block_data, block_data_size, 0);
			if (bchk != expected_bchk) {
				result.error = APE_LZ4_ERR_BAD_CHECKSUM;
				APE_LZ_FREE(out);
				return result;
			}
		}

		if (is_uncompressed) {
			/* Ensure output buffer has space */
			while (out_offset + block_data_size > out_cap) {
				out_cap *= 2;
				uint8_t *tmp = (uint8_t *)APE_LZ_REALLOC(out, out_cap);
				if (tmp == NULL) {
					APE_LZ_FREE(out);
					result.error = APE_LZ4_ERR_ALLOC_FAILED;
					return result;
				}
				out = tmp;
			}
			memcpy(out + out_offset, block_data, block_data_size);
			out_offset += block_data_size;
		} else {
			/* Decompress block */
			/* Ensure at least block_max space for decompressed output */
			while (out_offset + APE_LZ4_BLKMAX_4M > out_cap) {
				out_cap *= 2;
				uint8_t *tmp = (uint8_t *)APE_LZ_REALLOC(out, out_cap);
				if (tmp == NULL) {
					APE_LZ_FREE(out);
					result.error = APE_LZ4_ERR_ALLOC_FAILED;
					return result;
				}
				out = tmp;
			}

			size_t dec_size = lz4_block_decompress(block_data, block_data_size, out + out_offset, out_cap - out_offset);
			if (dec_size == 0) {
				result.error = APE_LZ4_ERR_CORRUPT;
				APE_LZ_FREE(out);
				return result;
			}
			out_offset += dec_size;
		}
	}

	/* --- Content Checksum --- */
	if (has_content_checksum) {
		if (r.offset + 4 > r.size) {
			result.error = APE_LZ4_ERR_CORRUPT;
			APE_LZ_FREE(out);
			return result;
		}
		uint32_t cchk = ape_pack_reader_read_u32(&r);
		uint32_t expected_cchk = xxh32(out, out_offset, 0);
		if (cchk != expected_cchk) {
			result.error = APE_LZ4_ERR_BAD_CHECKSUM;
			APE_LZ_FREE(out);
			return result;
		}
	}

	result.data = out;
	result.size = out_offset;
	return result;
}

/* ============================================================================
 * Error String
 * ============================================================================ */

APE_LZ_DEF const char *ape_lz4_error_string(ApeLZ4Error err)
{
	switch (err) {
		case APE_LZ4_OK: return "success";
		case APE_LZ4_ERR_NULL_INPUT: return "null input pointer";
		case APE_LZ4_ERR_NULL_OUTPUT: return "null output pointer";
		case APE_LZ4_ERR_INPUT_TOO_LARGE: return "input exceeds maximum supported size";
		case APE_LZ4_ERR_OUTPUT_TOO_SMALL: return "output buffer too small";
		case APE_LZ4_ERR_ALLOC_FAILED: return "memory allocation failed";
		case APE_LZ4_ERR_BAD_MAGIC: return "invalid frame magic number";
		case APE_LZ4_ERR_BAD_VERSION: return "unsupported frame version";
		case APE_LZ4_ERR_BAD_CHECKSUM: return "checksum mismatch";
		case APE_LZ4_ERR_BAD_BLOCK: return "invalid block size";
		case APE_LZ4_ERR_CORRUPT: return "compressed data is corrupt or truncated";
		case APE_LZ4_ERR_NO_ENDMARK: return "frame missing end mark";
	}
	return "unknown error";
}
/* END ape_lz.c */

#endif

#endif
