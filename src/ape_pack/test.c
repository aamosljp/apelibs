#include "test.h"
#include "ape_pack_api.h"

/* ============================================================================
 * Reader Tests
 * ============================================================================ */

TEST(read_u8)
{
	unsigned char buf[4] = { 0x00, 0x7F, 0x80, 0xFF };
	ApePackReader reader = ape_pack_reader_from(buf, sizeof(buf));
	ASSERT_EQ(ape_pack_reader_read_u8(&reader), 0x00);
	ASSERT_EQ(ape_pack_reader_read_u8(&reader), 0x7F);
	ASSERT_EQ(ape_pack_reader_read_u8(&reader), 0x80);
	ASSERT_EQ(ape_pack_reader_read_u8(&reader), 0xFF);
	return PASSED;
}

TEST(read_u16)
{
	/* little-endian: LSB first */
	unsigned char buf[4] = { 0x02, 0x01, 0xBB, 0xAA };
	ApePackReader reader = ape_pack_reader_from(buf, sizeof(buf));
	ASSERT_EQ(ape_pack_reader_read_u16(&reader), 0x0102);
	ASSERT_EQ(ape_pack_reader_read_u16(&reader), 0xAABB);
	return PASSED;
}

TEST(read_u32)
{
	/* little-endian: LSB first */
	unsigned char buf[4] = { 0x04, 0x03, 0x02, 0x01 };
	ApePackReader reader = ape_pack_reader_from(buf, sizeof(buf));
	ASSERT_EQ(ape_pack_reader_read_u32(&reader), 0x01020304);
	return PASSED;
}

TEST(read_u64)
{
	/* little-endian: LSB first */
	unsigned char buf[8] = { 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01 };
	ApePackReader reader = ape_pack_reader_from(buf, sizeof(buf));
	ASSERT_EQ(ape_pack_reader_read_u64(&reader), 0x0102030405060708ULL);
	return PASSED;
}

TEST(read_i8)
{
	unsigned char buf[4] = { 0xFF, 0x00, 0x7F, 0x80 };
	ApePackReader reader = ape_pack_reader_from(buf, sizeof(buf));
	ASSERT_EQ(ape_pack_reader_read_i8(&reader), -1);
	ASSERT_EQ(ape_pack_reader_read_i8(&reader), 0);
	ASSERT_EQ(ape_pack_reader_read_i8(&reader), 127);
	ASSERT_EQ(ape_pack_reader_read_i8(&reader), -128);
	return PASSED;
}

TEST(read_i16)
{
	/* little-endian */
	unsigned char buf[4] = { 0xFF, 0xFF, 0x00, 0x01 };
	ApePackReader reader = ape_pack_reader_from(buf, sizeof(buf));
	ASSERT_EQ(ape_pack_reader_read_i16(&reader), -1);
	ASSERT_EQ(ape_pack_reader_read_i16(&reader), 256);
	return PASSED;
}

TEST(read_i32)
{
	/* little-endian: -1 = 0xFFFFFFFF */
	unsigned char buf[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
	ApePackReader reader = ape_pack_reader_from(buf, sizeof(buf));
	ASSERT_EQ(ape_pack_reader_read_i32(&reader), -1);
	return PASSED;
}

TEST(read_i64)
{
	/* little-endian: -1 = 0xFFFFFFFFFFFFFFFF */
	unsigned char buf[8] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
	ApePackReader reader = ape_pack_reader_from(buf, sizeof(buf));
	ASSERT_EQ(ape_pack_reader_read_i64(&reader), -1);
	return PASSED;
}

TEST(read_f32)
{
	/* write 1.0f then read it back from known bytes */
	unsigned char buf[4] = { 0 };
	float val = 1.0f;
	memcpy(buf, &val, sizeof(val));
	ApePackReader reader = ape_pack_reader_from(buf, sizeof(buf));
	ASSERT(ape_pack_reader_read_f32(&reader) == 1.0f);
	return PASSED;
}

TEST(read_f64)
{
	unsigned char buf[8] = { 0 };
	double val = 3.14159265358979323846;
	memcpy(buf, &val, sizeof(val));
	ApePackReader reader = ape_pack_reader_from(buf, sizeof(buf));
	ASSERT(ape_pack_reader_read_f64(&reader) == 3.14159265358979323846);
	return PASSED;
}

TEST(read_raw)
{
	unsigned char buf[] = "hello world";
	ApePackReader reader = ape_pack_reader_from(buf, 11);
	char out[16] = { 0 };
	size_t n = ape_pack_reader_read(&reader, out, 11);
	ASSERT_EQ(n, 11);
	ASSERT_MEM_EQ(out, "hello world", 11);
	return PASSED;
}

TEST(read_le_byte_order)
{
	/* verify little-endian reads LSB first */
	unsigned char buf[8] = { 0x78, 0x56, 0x34, 0x12, 0xEF, 0xBE, 0xAD, 0xDE };
	ApePackReader reader = ape_pack_reader_from(buf, sizeof(buf));
	ASSERT_EQ(ape_pack_reader_read_u32(&reader), 0x12345678);
	ASSERT_EQ(ape_pack_reader_read_u32(&reader), 0xDEADBEEF);
	return PASSED;
}

TEST(read_be_byte_order)
{
	/* verify big-endian reads MSB first */
	unsigned char buf[8] = { 0x12, 0x34, 0x56, 0x78, 0xDE, 0xAD, 0xBE, 0xEF };
	ApePackReader reader = ape_pack_reader_from(buf, sizeof(buf));
	reader.endianess = 1;
	ASSERT_EQ(ape_pack_reader_read_u32(&reader), 0x12345678);
	ASSERT_EQ(ape_pack_reader_read_u32(&reader), 0xDEADBEEF);
	return PASSED;
}

TEST(reader_offset_tracking)
{
	unsigned char buf[64] = { 0 };
	ApePackReader reader = ape_pack_reader_from(buf, sizeof(buf));
	ASSERT_EQ(reader.offset, 0);
	ape_pack_reader_read_u8(&reader);
	ASSERT_EQ(reader.offset, 1);
	ape_pack_reader_read_u16(&reader);
	ASSERT_EQ(reader.offset, 3);
	ape_pack_reader_read_u32(&reader);
	ASSERT_EQ(reader.offset, 7);
	ape_pack_reader_read_u64(&reader);
	ASSERT_EQ(reader.offset, 15);
	ape_pack_reader_read_f32(&reader);
	ASSERT_EQ(reader.offset, 19);
	ape_pack_reader_read_f64(&reader);
	ASSERT_EQ(reader.offset, 27);
	return PASSED;
}

TEST(reader_eof)
{
	unsigned char buf[2] = { 0xAA, 0xBB };
	ApePackReader reader = ape_pack_reader_from(buf, sizeof(buf));
	ASSERT_FALSE(ape_pack_reader_eof(&reader));
	ape_pack_reader_read_u8(&reader);
	ASSERT_FALSE(ape_pack_reader_eof(&reader));
	ape_pack_reader_read_u8(&reader);
	ASSERT_TRUE(ape_pack_reader_eof(&reader));
	/* reading past end returns 0 */
	ASSERT_EQ(ape_pack_reader_read_u8(&reader), 0);
	ASSERT_TRUE(ape_pack_reader_eof(&reader));
	return PASSED;
}

TEST(reader_skip)
{
	unsigned char buf[8] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77 };
	ApePackReader reader = ape_pack_reader_from(buf, sizeof(buf));
	ape_pack_reader_skip(&reader, 3);
	ASSERT_EQ(reader.offset, 3);
	ASSERT_EQ(ape_pack_reader_read_u8(&reader), 0x33);
	ape_pack_reader_skip(&reader, 2);
	ASSERT_EQ(reader.offset, 6);
	ASSERT_EQ(ape_pack_reader_read_u8(&reader), 0x66);
	/* skip past end does nothing */
	ape_pack_reader_skip(&reader, 100);
	ASSERT_EQ(reader.offset, 7);
	return PASSED;
}

TEST(reader_reset)
{
	unsigned char buf[4] = { 0xAA, 0xBB, 0xCC, 0xDD };
	ApePackReader reader = ape_pack_reader_from(buf, sizeof(buf));
	ape_pack_reader_read_u16(&reader);
	ASSERT_EQ(reader.offset, 2);
	ape_pack_reader_reset(&reader);
	ASSERT_EQ(reader.offset, 0);
	/* can re-read from the start */
	ASSERT_EQ(ape_pack_reader_read_u8(&reader), 0xAA);
	return PASSED;
}

TEST(read_raw_bounds)
{
	unsigned char buf[4] = { 0x01, 0x02, 0x03, 0x04 };
	ApePackReader reader = ape_pack_reader_from(buf, sizeof(buf));
	char out[8] = { 0 };
	/* read more than available returns 0 and doesn't advance */
	size_t n = ape_pack_reader_read(&reader, out, 8);
	ASSERT_EQ(n, 0);
	ASSERT_EQ(reader.offset, 0);
	/* read exactly what's available works */
	n = ape_pack_reader_read(&reader, out, 4);
	ASSERT_EQ(n, 4);
	ASSERT_MEM_EQ(out, buf, 4);
	return PASSED;
}

/* ============================================================================
 * Writer Tests
 * ============================================================================ */

TEST(write_u8)
{
	unsigned char buf[8] = { 0 };
	ApePackWriter writer = ape_pack_writer_to(buf, sizeof(buf));
	uint8_t i;
	for (i = 0; i < 8; i++) {
		ape_pack_writer_write_u8(&writer, i * 10);
	}
	ASSERT_EQ(writer.offset, 8);
	for (i = 0; i < 8; i++) {
		ASSERT_EQ(buf[i], i * 10);
	}
	return PASSED;
}

TEST(write_u16)
{
	unsigned char buf[16] = { 0 };
	ApePackWriter writer = ape_pack_writer_to(buf, sizeof(buf));
	ape_pack_writer_write_u16(&writer, 0x0102);
	ape_pack_writer_write_u16(&writer, 0xAABB);
	/* default is little-endian: LSB first */
	ASSERT_EQ(buf[0], 0x02);
	ASSERT_EQ(buf[1], 0x01);
	ASSERT_EQ(buf[2], 0xBB);
	ASSERT_EQ(buf[3], 0xAA);
	return PASSED;
}

TEST(write_u32)
{
	unsigned char buf[8] = { 0 };
	ApePackWriter writer = ape_pack_writer_to(buf, sizeof(buf));
	ape_pack_writer_write_u32(&writer, 0x01020304);
	/* little-endian: LSB first */
	ASSERT_EQ(buf[0], 0x04);
	ASSERT_EQ(buf[1], 0x03);
	ASSERT_EQ(buf[2], 0x02);
	ASSERT_EQ(buf[3], 0x01);
	return PASSED;
}

TEST(write_u64)
{
	unsigned char buf[8] = { 0 };
	ApePackWriter writer = ape_pack_writer_to(buf, sizeof(buf));
	ape_pack_writer_write_u64(&writer, 0x0102030405060708ULL);
	/* little-endian: LSB first */
	ASSERT_EQ(buf[0], 0x08);
	ASSERT_EQ(buf[1], 0x07);
	ASSERT_EQ(buf[2], 0x06);
	ASSERT_EQ(buf[3], 0x05);
	ASSERT_EQ(buf[4], 0x04);
	ASSERT_EQ(buf[5], 0x03);
	ASSERT_EQ(buf[6], 0x02);
	ASSERT_EQ(buf[7], 0x01);
	return PASSED;
}

TEST(write_i8)
{
	unsigned char buf[4] = { 0 };
	ApePackWriter writer = ape_pack_writer_to(buf, sizeof(buf));
	ape_pack_writer_write_i8(&writer, -1);
	ape_pack_writer_write_i8(&writer, 0);
	ape_pack_writer_write_i8(&writer, 127);
	ape_pack_writer_write_i8(&writer, -128);
	ASSERT_EQ(buf[0], 0xFF);
	ASSERT_EQ(buf[1], 0x00);
	ASSERT_EQ(buf[2], 0x7F);
	ASSERT_EQ(buf[3], 0x80);
	return PASSED;
}

TEST(write_i16)
{
	unsigned char buf[4] = { 0 };
	ApePackWriter writer = ape_pack_writer_to(buf, sizeof(buf));
	ape_pack_writer_write_i16(&writer, -1);
	/* little-endian: 0xFFFF -> FF FF */
	ASSERT_EQ(buf[0], 0xFF);
	ASSERT_EQ(buf[1], 0xFF);
	ape_pack_writer_write_i16(&writer, 256);
	/* little-endian: 0x0100 -> 00 01 */
	ASSERT_EQ(buf[2], 0x00);
	ASSERT_EQ(buf[3], 0x01);
	return PASSED;
}

TEST(write_i32)
{
	unsigned char buf[4] = { 0 };
	ApePackWriter writer = ape_pack_writer_to(buf, sizeof(buf));
	ape_pack_writer_write_i32(&writer, -1);
	ASSERT_EQ(buf[0], 0xFF);
	ASSERT_EQ(buf[1], 0xFF);
	ASSERT_EQ(buf[2], 0xFF);
	ASSERT_EQ(buf[3], 0xFF);
	return PASSED;
}

TEST(write_i64)
{
	unsigned char buf[8] = { 0 };
	ApePackWriter writer = ape_pack_writer_to(buf, sizeof(buf));
	ape_pack_writer_write_i64(&writer, -1);
	int i;
	for (i = 0; i < 8; i++) {
		ASSERT_EQ(buf[i], 0xFF);
	}
	return PASSED;
}

TEST(write_f32)
{
	unsigned char buf[4] = { 0 };
	ApePackWriter writer = ape_pack_writer_to(buf, sizeof(buf));
	ape_pack_writer_write_f32(&writer, 1.0f);
	/* read it back */
	ApePackReader reader = ape_pack_reader_from(buf, sizeof(buf));
	float val = ape_pack_reader_read_f32(&reader);
	ASSERT(val == 1.0f);
	return PASSED;
}

TEST(write_f64)
{
	unsigned char buf[8] = { 0 };
	ApePackWriter writer = ape_pack_writer_to(buf, sizeof(buf));
	ape_pack_writer_write_f64(&writer, 3.14159265358979323846);
	ApePackReader reader = ape_pack_reader_from(buf, sizeof(buf));
	double val = ape_pack_reader_read_f64(&reader);
	ASSERT(val == 3.14159265358979323846);
	return PASSED;
}

TEST(write_raw)
{
	unsigned char buf[16] = { 0 };
	ApePackWriter writer = ape_pack_writer_to(buf, sizeof(buf));
	char data[] = "hello world";
	ape_pack_writer_write(&writer, data, 11);
	ASSERT_EQ(writer.offset, 11);
	ASSERT_MEM_EQ(buf, "hello world", 11);
	return PASSED;
}

/* ============================================================================
 * Writer/Reader Roundtrip Tests
 * ============================================================================ */

TEST(roundtrip_le)
{
	unsigned char buf[64] = { 0 };
	ApePackWriter writer = ape_pack_writer_to(buf, sizeof(buf));
	/* endianess 0 = little-endian (default) */

	ape_pack_writer_write_u8(&writer, 0xAB);
	ape_pack_writer_write_u16(&writer, 0x1234);
	ape_pack_writer_write_u32(&writer, 0xDEADBEEF);
	ape_pack_writer_write_u64(&writer, 0x0102030405060708ULL);
	ape_pack_writer_write_i32(&writer, -12345);
	ape_pack_writer_write_f32(&writer, 2.5f);
	ape_pack_writer_write_f64(&writer, -99.99);

	ApePackReader reader = ape_pack_reader_from(buf, sizeof(buf));
	/* reader defaults to little-endian too */

	ASSERT_EQ(ape_pack_reader_read_u8(&reader), 0xAB);
	ASSERT_EQ(ape_pack_reader_read_u16(&reader), 0x1234);
	ASSERT_EQ(ape_pack_reader_read_u32(&reader), 0xDEADBEEF);
	ASSERT_EQ(ape_pack_reader_read_u64(&reader), 0x0102030405060708ULL);
	ASSERT_EQ(ape_pack_reader_read_i32(&reader), -12345);
	ASSERT(ape_pack_reader_read_f32(&reader) == 2.5f);
	ASSERT(ape_pack_reader_read_f64(&reader) == -99.99);

	return PASSED;
}

TEST(roundtrip_be)
{
	unsigned char buf[64] = { 0 };
	ApePackWriter writer = ape_pack_writer_to(buf, sizeof(buf));
	writer.endianess = 1; /* big-endian */

	ape_pack_writer_write_u8(&writer, 0xAB);
	ape_pack_writer_write_u16(&writer, 0x1234);
	ape_pack_writer_write_u32(&writer, 0xDEADBEEF);
	ape_pack_writer_write_u64(&writer, 0x0102030405060708ULL);
	ape_pack_writer_write_i32(&writer, -12345);
	ape_pack_writer_write_f32(&writer, 2.5f);
	ape_pack_writer_write_f64(&writer, -99.99);

	ApePackReader reader = ape_pack_reader_from(buf, sizeof(buf));
	reader.endianess = 1; /* big-endian */

	ASSERT_EQ(ape_pack_reader_read_u8(&reader), 0xAB);
	ASSERT_EQ(ape_pack_reader_read_u16(&reader), 0x1234);
	ASSERT_EQ(ape_pack_reader_read_u32(&reader), 0xDEADBEEF);
	ASSERT_EQ(ape_pack_reader_read_u64(&reader), 0x0102030405060708ULL);
	ASSERT_EQ(ape_pack_reader_read_i32(&reader), -12345);
	ASSERT(ape_pack_reader_read_f32(&reader) == 2.5f);
	ASSERT(ape_pack_reader_read_f64(&reader) == -99.99);

	return PASSED;
}

TEST(roundtrip_be_byte_order)
{
	unsigned char buf[8] = { 0 };
	ApePackWriter writer = ape_pack_writer_to(buf, sizeof(buf));
	writer.endianess = 1;
	ape_pack_writer_write_u32(&writer, 0x01020304);
	/* big-endian: MSB first */
	ASSERT_EQ(buf[0], 0x01);
	ASSERT_EQ(buf[1], 0x02);
	ASSERT_EQ(buf[2], 0x03);
	ASSERT_EQ(buf[3], 0x04);
	return PASSED;
}

TEST(writer_offset_tracking)
{
	unsigned char buf[64] = { 0 };
	ApePackWriter writer = ape_pack_writer_to(buf, sizeof(buf));
	ASSERT_EQ(writer.offset, 0);
	ape_pack_writer_write_u8(&writer, 0);
	ASSERT_EQ(writer.offset, 1);
	ape_pack_writer_write_u16(&writer, 0);
	ASSERT_EQ(writer.offset, 3);
	ape_pack_writer_write_u32(&writer, 0);
	ASSERT_EQ(writer.offset, 7);
	ape_pack_writer_write_u64(&writer, 0);
	ASSERT_EQ(writer.offset, 15);
	ape_pack_writer_write_f32(&writer, 0);
	ASSERT_EQ(writer.offset, 19);
	ape_pack_writer_write_f64(&writer, 0);
	ASSERT_EQ(writer.offset, 27);
	return PASSED;
}

TEST(writer_reset)
{
	unsigned char buf[8] = { 0 };
	ApePackWriter writer = ape_pack_writer_to(buf, sizeof(buf));
	ape_pack_writer_write_u32(&writer, 0xAAAAAAAA);
	ASSERT_EQ(writer.offset, 4);
	ape_pack_writer_reset(&writer);
	ASSERT_EQ(writer.offset, 0);
	/* can write again from the start */
	ape_pack_writer_write_u32(&writer, 0xBBBBBBBB);
	ApePackReader reader = ape_pack_reader_from(buf, sizeof(buf));
	ASSERT_EQ(ape_pack_reader_read_u32(&reader), 0xBBBBBBBB);
	return PASSED;
}

/* ============================================================================
 * Custom Struct Tests
 * ============================================================================ */

/*
 * Mock game entity packed as:
 *   type     (u8)     1 byte
 *   flags    (u16)    2 bytes
 *   id       (u32)    4 bytes
 *   health   (i32)    4 bytes
 *   score    (u64)    8 bytes
 *   speed    (f32)    4 bytes
 *   pos_x    (f64)    8 bytes
 *   tag      (4 raw)  4 bytes
 *   -------------------
 *   total:            35 bytes
 */

typedef struct {
	uint8_t type;
	uint16_t flags;
	uint32_t id;
	int32_t health;
	uint64_t score;
	float speed;
	double pos_x;
	uint8_t tag[4];
} MockEntity;

#define MOCK_ENTITY_PACKED_SIZE 35

static MockEntity mock_entity_default(void)
{
	MockEntity e;
	e.type = 0x03;
	e.flags = 0xABCD;
	e.id = 0xDEADBEEF;
	e.health = -500;
	e.score = 0x0102030405060708ULL;
	e.speed = 1.5f;
	e.pos_x = -273.15;
	e.tag[0] = 'T';
	e.tag[1] = 'E';
	e.tag[2] = 'S';
	e.tag[3] = 'T';
	return e;
}

static void mock_entity_write(ApePackWriter *w, const MockEntity *e)
{
	ape_pack_writer_write_u8(w, e->type);
	ape_pack_writer_write_u16(w, e->flags);
	ape_pack_writer_write_u32(w, e->id);
	ape_pack_writer_write_i32(w, e->health);
	ape_pack_writer_write_u64(w, e->score);
	ape_pack_writer_write_f32(w, e->speed);
	ape_pack_writer_write_f64(w, e->pos_x);
	ape_pack_writer_write(w, (void *)e->tag, 4);
}

static MockEntity mock_entity_read(ApePackReader *r)
{
	MockEntity e;
	e.type = ape_pack_reader_read_u8(r);
	e.flags = ape_pack_reader_read_u16(r);
	e.id = ape_pack_reader_read_u32(r);
	e.health = ape_pack_reader_read_i32(r);
	e.score = ape_pack_reader_read_u64(r);
	e.speed = ape_pack_reader_read_f32(r);
	e.pos_x = ape_pack_reader_read_f64(r);
	ape_pack_reader_read(r, (char *)e.tag, 4);
	return e;
}

static int mock_entity_eq(const MockEntity *a, const MockEntity *b)
{
	return a->type == b->type && a->flags == b->flags && a->id == b->id && a->health == b->health && a->score == b->score &&
	       a->speed == b->speed && a->pos_x == b->pos_x && a->tag[0] == b->tag[0] && a->tag[1] == b->tag[1] && a->tag[2] == b->tag[2] &&
	       a->tag[3] == b->tag[3];
}

/*
 * Little-endian packed layout for mock_entity_default():
 *
 * type     (u8)  : 03
 * flags    (u16) : CD AB
 * id       (u32) : EF BE AD DE
 * health   (i32) : 0C FE FF FF   (-500 = 0xFFFFFE0C)
 * score    (u64) : 08 07 06 05 04 03 02 01
 * speed    (f32) : 00 00 C0 3F   (1.5f = 0x3FC00000)
 * pos_x    (f64) : 66 66 66 66 66 12 71 C0  (-273.15 = 0xC071126666666666)
 * tag      (raw) : 54 45 53 54   ("TEST")
 */
static const unsigned char MOCK_LE_BYTES[MOCK_ENTITY_PACKED_SIZE] = {
	0x03,						/* type */
	0xCD, 0xAB,					/* flags */
	0xEF, 0xBE, 0xAD, 0xDE,				/* id */
	0x0C, 0xFE, 0xFF, 0xFF,				/* health */
	0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, /* score */
	0x00, 0x00, 0xC0, 0x3F,				/* speed */
	0x66, 0x66, 0x66, 0x66, 0x66, 0x12, 0x71, 0xC0, /* pos_x */
	0x54, 0x45, 0x53, 0x54,				/* tag */
};

/*
 * Big-endian packed layout for mock_entity_default():
 * Same values, MSB first for multi-byte fields.
 */
static const unsigned char MOCK_BE_BYTES[MOCK_ENTITY_PACKED_SIZE] = {
	0x03,						/* type */
	0xAB, 0xCD,					/* flags */
	0xDE, 0xAD, 0xBE, 0xEF,				/* id */
	0xFF, 0xFF, 0xFE, 0x0C,				/* health */
	0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, /* score */
	0x3F, 0xC0, 0x00, 0x00,				/* speed */
	0xC0, 0x71, 0x12, 0x66, 0x66, 0x66, 0x66, 0x66, /* pos_x */
	0x54, 0x45, 0x53, 0x54,				/* tag */
};

TEST(struct_read_le)
{
	unsigned char buf[MOCK_ENTITY_PACKED_SIZE];
	memcpy(buf, MOCK_LE_BYTES, MOCK_ENTITY_PACKED_SIZE);

	ApePackReader reader = ape_pack_reader_from(buf, MOCK_ENTITY_PACKED_SIZE);
	MockEntity got = mock_entity_read(&reader);
	MockEntity expected = mock_entity_default();

	ASSERT_EQ(got.type, expected.type);
	ASSERT_EQ(got.flags, expected.flags);
	ASSERT_EQ(got.id, expected.id);
	ASSERT_EQ(got.health, expected.health);
	ASSERT_EQ(got.score, expected.score);
	ASSERT(got.speed == expected.speed);
	ASSERT(got.pos_x == expected.pos_x);
	ASSERT_MEM_EQ(got.tag, expected.tag, 4);
	ASSERT_EQ(reader.offset, MOCK_ENTITY_PACKED_SIZE);
	return PASSED;
}

TEST(struct_read_be)
{
	unsigned char buf[MOCK_ENTITY_PACKED_SIZE];
	memcpy(buf, MOCK_BE_BYTES, MOCK_ENTITY_PACKED_SIZE);

	ApePackReader reader = ape_pack_reader_from(buf, MOCK_ENTITY_PACKED_SIZE);
	reader.endianess = 1;
	MockEntity got = mock_entity_read(&reader);
	MockEntity expected = mock_entity_default();

	ASSERT_EQ(got.type, expected.type);
	ASSERT_EQ(got.flags, expected.flags);
	ASSERT_EQ(got.id, expected.id);
	ASSERT_EQ(got.health, expected.health);
	ASSERT_EQ(got.score, expected.score);
	ASSERT(got.speed == expected.speed);
	ASSERT(got.pos_x == expected.pos_x);
	ASSERT_MEM_EQ(got.tag, expected.tag, 4);
	ASSERT_EQ(reader.offset, MOCK_ENTITY_PACKED_SIZE);
	return PASSED;
}

TEST(struct_write_le)
{
	unsigned char buf[MOCK_ENTITY_PACKED_SIZE] = { 0 };
	ApePackWriter writer = ape_pack_writer_to(buf, sizeof(buf));
	MockEntity e = mock_entity_default();
	mock_entity_write(&writer, &e);

	ASSERT_EQ(writer.offset, MOCK_ENTITY_PACKED_SIZE);
	ASSERT_MEM_EQ(buf, MOCK_LE_BYTES, MOCK_ENTITY_PACKED_SIZE);
	return PASSED;
}

TEST(struct_write_be)
{
	unsigned char buf[MOCK_ENTITY_PACKED_SIZE] = { 0 };
	ApePackWriter writer = ape_pack_writer_to(buf, sizeof(buf));
	writer.endianess = 1;
	MockEntity e = mock_entity_default();
	mock_entity_write(&writer, &e);

	ASSERT_EQ(writer.offset, MOCK_ENTITY_PACKED_SIZE);
	ASSERT_MEM_EQ(buf, MOCK_BE_BYTES, MOCK_ENTITY_PACKED_SIZE);
	return PASSED;
}

TEST(struct_roundtrip_le)
{
	unsigned char buf[MOCK_ENTITY_PACKED_SIZE] = { 0 };
	MockEntity original = mock_entity_default();

	ApePackWriter writer = ape_pack_writer_to(buf, sizeof(buf));
	mock_entity_write(&writer, &original);

	ApePackReader reader = ape_pack_reader_from(buf, sizeof(buf));
	MockEntity restored = mock_entity_read(&reader);

	ASSERT_TRUE(mock_entity_eq(&original, &restored));
	ASSERT_EQ(writer.offset, reader.offset);
	return PASSED;
}

TEST(struct_roundtrip_be)
{
	unsigned char buf[MOCK_ENTITY_PACKED_SIZE] = { 0 };
	MockEntity original = mock_entity_default();

	ApePackWriter writer = ape_pack_writer_to(buf, sizeof(buf));
	writer.endianess = 1;
	mock_entity_write(&writer, &original);

	ApePackReader reader = ape_pack_reader_from(buf, sizeof(buf));
	reader.endianess = 1;
	MockEntity restored = mock_entity_read(&reader);

	ASSERT_TRUE(mock_entity_eq(&original, &restored));
	ASSERT_EQ(writer.offset, reader.offset);
	return PASSED;
}

static void run_reader_tests(void)
{
	LOG_INFO("Reader tests:");
	RUN_TEST(read_u8);
	RUN_TEST(read_u16);
	RUN_TEST(read_u32);
	RUN_TEST(read_u64);
	RUN_TEST(read_i8);
	RUN_TEST(read_i16);
	RUN_TEST(read_i32);
	RUN_TEST(read_i64);
	RUN_TEST(read_f32);
	RUN_TEST(read_f64);
	RUN_TEST(read_raw);
	RUN_TEST(read_le_byte_order);
	RUN_TEST(read_be_byte_order);
	RUN_TEST(reader_offset_tracking);
	RUN_TEST(reader_eof);
	RUN_TEST(reader_skip);
	RUN_TEST(reader_reset);
	RUN_TEST(read_raw_bounds);
	LOG_INFO("");
}

static void run_writer_tests(void)
{
	LOG_INFO("Writer tests:");
	RUN_TEST(write_u8);
	RUN_TEST(write_u16);
	RUN_TEST(write_u32);
	RUN_TEST(write_u64);
	RUN_TEST(write_i8);
	RUN_TEST(write_i16);
	RUN_TEST(write_i32);
	RUN_TEST(write_i64);
	RUN_TEST(write_f32);
	RUN_TEST(write_f64);
	RUN_TEST(write_raw);
	LOG_INFO("");
}

static void run_roundtrip_tests(void)
{
	LOG_INFO("Roundtrip tests:");
	RUN_TEST(roundtrip_le);
	RUN_TEST(roundtrip_be);
	RUN_TEST(roundtrip_be_byte_order);
	RUN_TEST(writer_offset_tracking);
	RUN_TEST(writer_reset);
	LOG_INFO("");
}

static void run_struct_tests(void)
{
	LOG_INFO("Struct tests:");
	RUN_TEST(struct_read_le);
	RUN_TEST(struct_read_be);
	RUN_TEST(struct_write_le);
	RUN_TEST(struct_write_be);
	RUN_TEST(struct_roundtrip_le);
	RUN_TEST(struct_roundtrip_be);
	LOG_INFO("");
}

int main(void)
{
	LOG_INFO("Running tests...");
	run_reader_tests();
	run_writer_tests();
	run_roundtrip_tests();
	run_struct_tests();
	LOG_INFO("Tests finished");
	LOG_INFO("%d Total", tests_run);
	if (tests_failed > 0)
		LOG_INFO("%d \x1b[31mFAILED\x1b[0m", tests_failed);
	if (tests_passed > 0)
		LOG_INFO("%d \x1b[32mPASSED\x1b[0m", tests_passed);
	return tests_failed > 0 ? 1 : 0;
}
