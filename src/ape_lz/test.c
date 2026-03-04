#include "test.h"
#include "ape_lz_api.h"

#include <string.h>

/* ============================================================================
 * Error String Tests
 * ============================================================================ */

TEST(error_strings) {
	ASSERT_STR_EQ(ape_lz4_error_string(APE_LZ4_OK), "success");
	ASSERT_STR_EQ(ape_lz4_error_string(APE_LZ4_ERR_NULL_INPUT), "null input pointer");
	ASSERT_STR_EQ(ape_lz4_error_string(APE_LZ4_ERR_BAD_MAGIC), "invalid frame magic number");
	ASSERT_STR_EQ(ape_lz4_error_string(APE_LZ4_ERR_CORRUPT), "compressed data is corrupt or truncated");
	return PASSED;
}

/* ============================================================================
 * Compress Error Cases
 * ============================================================================ */

TEST(compress_null_input) {
	ApeLZ4Result r = ape_lz4_compress(NULL, 100, NULL);
	ASSERT_EQ(r.error, APE_LZ4_ERR_NULL_INPUT);
	ASSERT_NULL(r.data);
	return PASSED;
}

TEST(compress_null_input_zero_len) {
	/* NULL with length 0 is valid (empty input) */
	ApeLZ4Result r = ape_lz4_compress(NULL, 0, NULL);
	ASSERT_EQ(r.error, APE_LZ4_OK);
	ASSERT_NOT_NULL(r.data);
	ASSERT_GT(r.size, 0);
	APE_LZ_FREE(r.data);
	return PASSED;
}

/* ============================================================================
 * Decompress Error Cases
 * ============================================================================ */

TEST(decompress_null_input) {
	ApeLZ4Result r = ape_lz4_decompress(NULL, 100);
	ASSERT_EQ(r.error, APE_LZ4_ERR_NULL_INPUT);
	ASSERT_NULL(r.data);
	return PASSED;
}

TEST(decompress_truncated) {
	uint8_t buf[3] = { 0x04, 0x22, 0x4D };
	ApeLZ4Result r = ape_lz4_decompress(buf, 3);
	ASSERT_EQ(r.error, APE_LZ4_ERR_CORRUPT);
	return PASSED;
}

TEST(decompress_bad_magic) {
	uint8_t buf[11] = { 0 };
	/* Wrong magic */
	buf[0] = 0xFF;
	buf[1] = 0xFF;
	buf[2] = 0xFF;
	buf[3] = 0xFF;
	ApeLZ4Result r = ape_lz4_decompress(buf, sizeof(buf));
	ASSERT_EQ(r.error, APE_LZ4_ERR_BAD_MAGIC);
	return PASSED;
}

/* ============================================================================
 * Roundtrip Tests
 * ============================================================================ */

TEST(roundtrip_empty) {
	uint8_t empty = 0;
	ApeLZ4Result comp = ape_lz4_compress(&empty, 0, NULL);
	ASSERT_EQ(comp.error, APE_LZ4_OK);
	ASSERT_NOT_NULL(comp.data);

	ApeLZ4Result dec = ape_lz4_decompress(comp.data, comp.size);
	ASSERT_EQ(dec.error, APE_LZ4_OK);
	ASSERT_EQ(dec.size, 0);

	APE_LZ_FREE(comp.data);
	APE_LZ_FREE(dec.data);
	return PASSED;
}

TEST(roundtrip_small) {
	const char *msg = "hello world";
	size_t len = strlen(msg);

	ApeLZ4Result comp = ape_lz4_compress((const uint8_t *)msg, len, NULL);
	ASSERT_EQ(comp.error, APE_LZ4_OK);
	ASSERT_NOT_NULL(comp.data);

	ApeLZ4Result dec = ape_lz4_decompress(comp.data, comp.size);
	ASSERT_EQ(dec.error, APE_LZ4_OK);
	ASSERT_EQ(dec.size, len);
	ASSERT_MEM_EQ(dec.data, msg, len);

	APE_LZ_FREE(comp.data);
	APE_LZ_FREE(dec.data);
	return PASSED;
}

TEST(roundtrip_single_byte) {
	uint8_t byte = 0xAB;

	ApeLZ4Result comp = ape_lz4_compress(&byte, 1, NULL);
	ASSERT_EQ(comp.error, APE_LZ4_OK);

	ApeLZ4Result dec = ape_lz4_decompress(comp.data, comp.size);
	ASSERT_EQ(dec.error, APE_LZ4_OK);
	ASSERT_EQ(dec.size, 1);
	ASSERT_EQ(dec.data[0], 0xAB);

	APE_LZ_FREE(comp.data);
	APE_LZ_FREE(dec.data);
	return PASSED;
}

TEST(roundtrip_repeated) {
	/* Highly compressible: same byte repeated */
	size_t len = 10000;
	uint8_t *data = (uint8_t *)APE_LZ_MALLOC(len);
	ASSERT_NOT_NULL(data);
	memset(data, 'A', len);

	ApeLZ4Result comp = ape_lz4_compress(data, len, NULL);
	ASSERT_EQ(comp.error, APE_LZ4_OK);
	/* Should actually compress */

	ApeLZ4Result dec = ape_lz4_decompress(comp.data, comp.size);
	ASSERT_EQ(dec.error, APE_LZ4_OK);
	ASSERT_EQ(dec.size, len);
	ASSERT_MEM_EQ(dec.data, data, len);

	APE_LZ_FREE(data);
	APE_LZ_FREE(comp.data);
	APE_LZ_FREE(dec.data);
	return PASSED;
}

TEST(roundtrip_pattern) {
	/* Repeating 4-byte pattern — should compress well */
	size_t len = 8192;
	uint8_t *data = (uint8_t *)APE_LZ_MALLOC(len);
	ASSERT_NOT_NULL(data);
	size_t i;
	for (i = 0; i < len; i++) { data[i] = (uint8_t)(i % 4); }

	ApeLZ4Result comp = ape_lz4_compress(data, len, NULL);
	ASSERT_EQ(comp.error, APE_LZ4_OK);
	ASSERT_LT(comp.size, len);

	ApeLZ4Result dec = ape_lz4_decompress(comp.data, comp.size);
	ASSERT_EQ(dec.error, APE_LZ4_OK);
	ASSERT_EQ(dec.size, len);
	ASSERT_MEM_EQ(dec.data, data, len);

	APE_LZ_FREE(data);
	APE_LZ_FREE(comp.data);
	APE_LZ_FREE(dec.data);
	return PASSED;
}

TEST(roundtrip_incompressible) {
	/* Pseudo-random data — should roundtrip even if not compressible */
	size_t len = 4096;
	uint8_t *data = (uint8_t *)APE_LZ_MALLOC(len);
	ASSERT_NOT_NULL(data);
	uint32_t rng = 0xDEADBEEF;
	size_t i;
	for (i = 0; i < len; i++) {
		rng ^= rng << 13;
		rng ^= rng >> 17;
		rng ^= rng << 5;
		data[i] = (uint8_t)(rng & 0xFF);
	}

	ApeLZ4Result comp = ape_lz4_compress(data, len, NULL);
	ASSERT_EQ(comp.error, APE_LZ4_OK);

	ApeLZ4Result dec = ape_lz4_decompress(comp.data, comp.size);
	ASSERT_EQ(dec.error, APE_LZ4_OK);
	ASSERT_EQ(dec.size, len);
	ASSERT_MEM_EQ(dec.data, data, len);

	APE_LZ_FREE(data);
	APE_LZ_FREE(comp.data);
	APE_LZ_FREE(dec.data);
	return PASSED;
}

TEST(roundtrip_64k) {
	/* Larger than the hash table range, still within one block */
	size_t len = 65536;
	uint8_t *data = (uint8_t *)APE_LZ_MALLOC(len);
	ASSERT_NOT_NULL(data);
	size_t i;
	for (i = 0; i < len; i++) { data[i] = (uint8_t)(i * 7 + i / 256); }

	ApeLZ4Result comp = ape_lz4_compress(data, len, NULL);
	ASSERT_EQ(comp.error, APE_LZ4_OK);

	ApeLZ4Result dec = ape_lz4_decompress(comp.data, comp.size);
	ASSERT_EQ(dec.error, APE_LZ4_OK);
	ASSERT_EQ(dec.size, len);
	ASSERT_MEM_EQ(dec.data, data, len);

	APE_LZ_FREE(data);
	APE_LZ_FREE(comp.data);
	APE_LZ_FREE(dec.data);
	return PASSED;
}

/* ============================================================================
 * Options Tests
 * ============================================================================ */

TEST(roundtrip_with_block_checksum) {
	const char *msg = "block checksum test data with some repetition repetition repetition";
	size_t len = strlen(msg);

	ApeLZ4Options opts;
	opts.block_checksum = 1;
	opts.content_checksum = 0;
	opts.content_size = 0;

	ApeLZ4Result comp = ape_lz4_compress((const uint8_t *)msg, len, &opts);
	ASSERT_EQ(comp.error, APE_LZ4_OK);

	ApeLZ4Result dec = ape_lz4_decompress(comp.data, comp.size);
	ASSERT_EQ(dec.error, APE_LZ4_OK);
	ASSERT_EQ(dec.size, len);
	ASSERT_MEM_EQ(dec.data, msg, len);

	APE_LZ_FREE(comp.data);
	APE_LZ_FREE(dec.data);
	return PASSED;
}

TEST(roundtrip_with_content_checksum) {
	const char *msg = "content checksum test data with repetition repetition repetition";
	size_t len = strlen(msg);

	ApeLZ4Options opts;
	opts.block_checksum = 0;
	opts.content_checksum = 1;
	opts.content_size = 0;

	ApeLZ4Result comp = ape_lz4_compress((const uint8_t *)msg, len, &opts);
	ASSERT_EQ(comp.error, APE_LZ4_OK);

	ApeLZ4Result dec = ape_lz4_decompress(comp.data, comp.size);
	ASSERT_EQ(dec.error, APE_LZ4_OK);
	ASSERT_EQ(dec.size, len);
	ASSERT_MEM_EQ(dec.data, msg, len);

	APE_LZ_FREE(comp.data);
	APE_LZ_FREE(dec.data);
	return PASSED;
}

TEST(roundtrip_with_content_size) {
	const char *msg = "content size test data";
	size_t len = strlen(msg);

	ApeLZ4Options opts;
	opts.block_checksum = 0;
	opts.content_checksum = 0;
	opts.content_size = 1;

	ApeLZ4Result comp = ape_lz4_compress((const uint8_t *)msg, len, &opts);
	ASSERT_EQ(comp.error, APE_LZ4_OK);

	ApeLZ4Result dec = ape_lz4_decompress(comp.data, comp.size);
	ASSERT_EQ(dec.error, APE_LZ4_OK);
	ASSERT_EQ(dec.size, len);
	ASSERT_MEM_EQ(dec.data, msg, len);

	APE_LZ_FREE(comp.data);
	APE_LZ_FREE(dec.data);
	return PASSED;
}

TEST(roundtrip_all_options) {
	size_t len = 5000;
	uint8_t *data = (uint8_t *)APE_LZ_MALLOC(len);
	ASSERT_NOT_NULL(data);
	memset(data, 'X', len);
	/* Mix in some variation */
	size_t i;
	for (i = 0; i < len; i += 100) { data[i] = (uint8_t)(i & 0xFF); }

	ApeLZ4Options opts;
	opts.block_checksum = 1;
	opts.content_checksum = 1;
	opts.content_size = 1;

	ApeLZ4Result comp = ape_lz4_compress(data, len, &opts);
	ASSERT_EQ(comp.error, APE_LZ4_OK);

	ApeLZ4Result dec = ape_lz4_decompress(comp.data, comp.size);
	ASSERT_EQ(dec.error, APE_LZ4_OK);
	ASSERT_EQ(dec.size, len);
	ASSERT_MEM_EQ(dec.data, data, len);

	APE_LZ_FREE(data);
	APE_LZ_FREE(comp.data);
	APE_LZ_FREE(dec.data);
	return PASSED;
}

/* ============================================================================
 * Corruption Detection Tests
 * ============================================================================ */

TEST(detect_corrupted_block_checksum) {
	const char *msg = "checksum corruption test with repetition repetition repetition";
	size_t len = strlen(msg);

	ApeLZ4Options opts;
	opts.block_checksum = 1;
	opts.content_checksum = 0;
	opts.content_size = 0;

	ApeLZ4Result comp = ape_lz4_compress((const uint8_t *)msg, len, &opts);
	ASSERT_EQ(comp.error, APE_LZ4_OK);

	/* Corrupt a byte in the compressed data (after the header) */
	if (comp.size > 15) { comp.data[comp.size - 6] ^= 0xFF; }

	ApeLZ4Result dec = ape_lz4_decompress(comp.data, comp.size);
	ASSERT_NE(dec.error, APE_LZ4_OK);

	if (dec.data) APE_LZ_FREE(dec.data);
	APE_LZ_FREE(comp.data);
	return PASSED;
}

TEST(detect_corrupted_content_checksum) {
	const char *msg = "content corruption detection test with repetition repetition";
	size_t len = strlen(msg);

	ApeLZ4Options opts;
	opts.block_checksum = 0;
	opts.content_checksum = 1;
	opts.content_size = 0;

	ApeLZ4Result comp = ape_lz4_compress((const uint8_t *)msg, len, &opts);
	ASSERT_EQ(comp.error, APE_LZ4_OK);

	/* Corrupt the last 4 bytes (content checksum) */
	comp.data[comp.size - 1] ^= 0xFF;

	ApeLZ4Result dec = ape_lz4_decompress(comp.data, comp.size);
	ASSERT_EQ(dec.error, APE_LZ4_ERR_BAD_CHECKSUM);

	if (dec.data) APE_LZ_FREE(dec.data);
	APE_LZ_FREE(comp.data);
	return PASSED;
}

/* ============================================================================
 * Test Runner
 * ============================================================================ */

static void run_error_tests(void) {
	LOG_INFO("Error tests:");
	RUN_TEST(error_strings);
	RUN_TEST(compress_null_input);
	RUN_TEST(compress_null_input_zero_len);
	RUN_TEST(decompress_null_input);
	RUN_TEST(decompress_truncated);
	RUN_TEST(decompress_bad_magic);
	LOG_INFO("");
}

static void run_roundtrip_tests(void) {
	LOG_INFO("Roundtrip tests:");
	RUN_TEST(roundtrip_empty);
	RUN_TEST(roundtrip_small);
	RUN_TEST(roundtrip_single_byte);
	RUN_TEST(roundtrip_repeated);
	RUN_TEST(roundtrip_pattern);
	RUN_TEST(roundtrip_incompressible);
	RUN_TEST(roundtrip_64k);
	LOG_INFO("");
}

static void run_options_tests(void) {
	LOG_INFO("Options tests:");
	RUN_TEST(roundtrip_with_block_checksum);
	RUN_TEST(roundtrip_with_content_checksum);
	RUN_TEST(roundtrip_with_content_size);
	RUN_TEST(roundtrip_all_options);
	LOG_INFO("");
}

static void run_corruption_tests(void) {
	LOG_INFO("Corruption detection tests:");
	RUN_TEST(detect_corrupted_block_checksum);
	RUN_TEST(detect_corrupted_content_checksum);
	LOG_INFO("");
}

int main(void) {
	LOG_INFO("Running tests...");
	run_error_tests();
	run_roundtrip_tests();
	run_options_tests();
	run_corruption_tests();
	LOG_INFO("Tests finished");
	LOG_INFO("%d Total", tests_run);
	if (tests_failed > 0) LOG_INFO("%d \x1b[31mFAILED\x1b[0m", tests_failed);
	if (tests_passed > 0) LOG_INFO("%d \x1b[32mPASSED\x1b[0m", tests_passed);
	return tests_failed > 0 ? 1 : 0;
}
