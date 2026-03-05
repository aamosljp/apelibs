#include "ape_pack_internal.h"

#include <string.h>

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
