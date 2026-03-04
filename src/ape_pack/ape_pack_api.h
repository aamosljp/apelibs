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
