#ifndef APE_LIBS_TEST_H
#define APE_LIBS_TEST_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* Simple test framework */
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define LOG_INFO(fmt, ...) fprintf(stderr, "\x1b[34mINFO\x1b[0m\t" fmt "\n", ##__VA_ARGS__)

#define FAILED 1
#define PASSED 0

#define TEST(name)                                                                \
	static int test_##name(void);                                             \
	static void run_test_##name(void) {                                       \
		tests_run++;                                                      \
		if (test_##name() == FAILED) {                                    \
			tests_failed++;                                           \
			LOG_INFO("  Testing %s... \x1b[31mFAILED\x1b[0m", #name); \
		} else {                                                          \
			tests_passed++;                                           \
			LOG_INFO("  Testing %s... \x1b[32mPASSED\x1b[0m", #name); \
		}                                                                 \
	}                                                                         \
	static int test_##name(void)

#define ASSERT(cond)                                        \
	if (!(cond)) {                                      \
		LOG_INFO("  Assertion failed: %s", #cond);  \
		LOG_INFO("  At %s:%d", __FILE__, __LINE__); \
		return FAILED;                              \
	}

/* Boolean assertions */
#define ASSERT_TRUE(cond) ASSERT(cond)
#define ASSERT_FALSE(cond) ASSERT(!(cond))

/* Integer comparison assertions (print actual values on failure) */
#define ASSERT_EQ(a, b)                                                   \
	do {                                                              \
		long long _a = (long long)(a);                            \
		long long _b = (long long)(b);                            \
		if (_a != _b) {                                           \
			LOG_INFO("  Assertion failed: %s == %s", #a, #b); \
			LOG_INFO("    left:  %lld", _a);                  \
			LOG_INFO("    right: %lld", _b);                  \
			LOG_INFO("  At %s:%d", __FILE__, __LINE__);       \
			return FAILED;                                    \
		}                                                         \
	} while (0)

#define ASSERT_NE(a, b)                                                   \
	do {                                                              \
		long long _a = (long long)(a);                            \
		long long _b = (long long)(b);                            \
		if (_a == _b) {                                           \
			LOG_INFO("  Assertion failed: %s != %s", #a, #b); \
			LOG_INFO("    both:  %lld", _a);                  \
			LOG_INFO("  At %s:%d", __FILE__, __LINE__);       \
			return FAILED;                                    \
		}                                                         \
	} while (0)

#define ASSERT_GT(a, b)                                                  \
	do {                                                             \
		long long _a = (long long)(a);                           \
		long long _b = (long long)(b);                           \
		if (!(_a > _b)) {                                        \
			LOG_INFO("  Assertion failed: %s > %s", #a, #b); \
			LOG_INFO("    left:  %lld", _a);                 \
			LOG_INFO("    right: %lld", _b);                 \
			LOG_INFO("  At %s:%d", __FILE__, __LINE__);      \
			return FAILED;                                   \
		}                                                        \
	} while (0)

#define ASSERT_GE(a, b)                                                   \
	do {                                                              \
		long long _a = (long long)(a);                            \
		long long _b = (long long)(b);                            \
		if (!(_a >= _b)) {                                        \
			LOG_INFO("  Assertion failed: %s >= %s", #a, #b); \
			LOG_INFO("    left:  %lld", _a);                  \
			LOG_INFO("    right: %lld", _b);                  \
			LOG_INFO("  At %s:%d", __FILE__, __LINE__);       \
			return FAILED;                                    \
		}                                                         \
	} while (0)

#define ASSERT_LT(a, b)                                                  \
	do {                                                             \
		long long _a = (long long)(a);                           \
		long long _b = (long long)(b);                           \
		if (!(_a < _b)) {                                        \
			LOG_INFO("  Assertion failed: %s < %s", #a, #b); \
			LOG_INFO("    left:  %lld", _a);                 \
			LOG_INFO("    right: %lld", _b);                 \
			LOG_INFO("  At %s:%d", __FILE__, __LINE__);      \
			return FAILED;                                   \
		}                                                        \
	} while (0)

#define ASSERT_LE(a, b)                                                   \
	do {                                                              \
		long long _a = (long long)(a);                            \
		long long _b = (long long)(b);                            \
		if (!(_a <= _b)) {                                        \
			LOG_INFO("  Assertion failed: %s <= %s", #a, #b); \
			LOG_INFO("    left:  %lld", _a);                  \
			LOG_INFO("    right: %lld", _b);                  \
			LOG_INFO("  At %s:%d", __FILE__, __LINE__);       \
			return FAILED;                                    \
		}                                                         \
	} while (0)

/* Pointer assertions */
#define ASSERT_NULL(a)                                                  \
	do {                                                            \
		const void *_a = (const void *)(a);                     \
		if (_a != NULL) {                                       \
			LOG_INFO("  Assertion failed: %s == NULL", #a); \
			LOG_INFO("    got: %p", _a);                    \
			LOG_INFO("  At %s:%d", __FILE__, __LINE__);     \
			return FAILED;                                  \
		}                                                       \
	} while (0)

#define ASSERT_NOT_NULL(a)                                              \
	do {                                                            \
		if ((const void *)(a) == NULL) {                        \
			LOG_INFO("  Assertion failed: %s != NULL", #a); \
			LOG_INFO("  At %s:%d", __FILE__, __LINE__);     \
			return FAILED;                                  \
		}                                                       \
	} while (0)

/* String assertions */
#define ASSERT_STR_EQ(a, b)                                               \
	do {                                                              \
		const char *_a = (a);                                     \
		const char *_b = (b);                                     \
		if (strcmp(_a, _b) != 0) {                                \
			LOG_INFO("  Assertion failed: %s == %s", #a, #b); \
			LOG_INFO("    left:  \"%s\"", _a);                \
			LOG_INFO("    right: \"%s\"", _b);                \
			LOG_INFO("  At %s:%d", __FILE__, __LINE__);       \
			return FAILED;                                    \
		}                                                         \
	} while (0)

#define ASSERT_STR_NE(a, b)                                               \
	do {                                                              \
		const char *_a = (a);                                     \
		const char *_b = (b);                                     \
		if (strcmp(_a, _b) == 0) {                                \
			LOG_INFO("  Assertion failed: %s != %s", #a, #b); \
			LOG_INFO("    both:  \"%s\"", _a);                \
			LOG_INFO("  At %s:%d", __FILE__, __LINE__);       \
			return FAILED;                                    \
		}                                                         \
	} while (0)

#define ASSERT_STR_CONTAINS(haystack, needle)                                                       \
	do {                                                                                        \
		const char *_h = (haystack);                                                        \
		const char *_n = (needle);                                                          \
		if (strstr(_h, _n) == NULL) {                                                       \
			LOG_INFO("  Assertion failed: \"%s\" contains \"%s\"", #haystack, #needle); \
			LOG_INFO("    haystack: \"%s\"", _h);                                       \
			LOG_INFO("    needle:   \"%s\"", _n);                                       \
			LOG_INFO("  At %s:%d", __FILE__, __LINE__);                                 \
			return FAILED;                                                              \
		}                                                                                   \
	} while (0)

/* Memory assertions */
#define ASSERT_MEM_EQ(a, b, len)                                                                                      \
	do {                                                                                                          \
		const unsigned char *_a = (const unsigned char *)(a);                                                 \
		const unsigned char *_b = (const unsigned char *)(b);                                                 \
		size_t _len = (len);                                                                                  \
		if (memcmp(_a, _b, _len) != 0) {                                                                      \
			size_t _i;                                                                                    \
			LOG_INFO("  Assertion failed: %s == %s (%zu bytes)", #a, #b, _len);                           \
			for (_i = 0; _i < _len; _i++) {                                                               \
				if (_a[_i] != _b[_i]) {                                                               \
					LOG_INFO("    first diff at byte %zu: 0x%02x vs 0x%02x", _i, _a[_i], _b[_i]); \
					break;                                                                        \
				}                                                                                     \
			}                                                                                             \
			LOG_INFO("  At %s:%d", __FILE__, __LINE__);                                                   \
			return FAILED;                                                                                \
		}                                                                                                     \
	} while (0)

/* Filesystem assertions */
#define ASSERT_FILE_EXISTS(path) ASSERT(access((path), F_OK) == 0)

#define RUN_TEST(name) run_test_##name()

/* Example main function
int main(void)
{
	LOG_INFO("Running tests...");
    // Run tests
	LOG_INFO("Tests finished");
	LOG_INFO("%d Total", tests_run);
	if (tests_failed > 0)
		LOG_INFO("%d \x1b[31mFAILED\x1b[0m", tests_failed);
	if (tests_passed > 0)
		LOG_INFO("%d \x1b[32mPASSED\x1b[0m", tests_passed);
	return tests_failed > 0 ? 1 : 0;
}*/

#endif
