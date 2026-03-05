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
