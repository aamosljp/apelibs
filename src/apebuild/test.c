/*
 * test.c - Unit tests for Apebuild modules
 */

#include "apebuild_api.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* Simple test framework */
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name)                                 \
	static void test_##name(void);             \
	static void run_test_##name(void)          \
	{                                          \
		tests_run++;                       \
		printf("  Testing %s... ", #name); \
		test_##name();                     \
		tests_passed++;                    \
		printf("OK\n");                    \
	}                                          \
	static void test_##name(void)

#define ASSERT(cond)                                                  \
	do {                                                          \
		if (!(cond)) {                                        \
			printf("FAILED\n");                           \
			printf("    Assertion failed: %s\n", #cond);  \
			printf("    At %s:%d\n", __FILE__, __LINE__); \
			tests_failed++;                               \
			return;                                       \
		}                                                     \
	} while (0)

#define ASSERT_EQ(a, b) ASSERT((a) == (b))
#define ASSERT_NE(a, b) ASSERT((a) != (b))
#define ASSERT_STR_EQ(a, b) ASSERT(strcmp((a), (b)) == 0)
#define ASSERT_NULL(a) ASSERT((a) == NULL)
#define ASSERT_NOT_NULL(a) ASSERT((a) != NULL)
#define ASSERT_FILE_EXISTS(path) ASSERT(access((path), F_OK) == 0)

#define RUN_TEST(name) run_test_##name()

/* ============================================================================
 * String Module Tests
 * ============================================================================ */

TEST(str_dup)
{
	char *s = ape_str_dup("hello");
	ASSERT_NOT_NULL(s);
	ASSERT_STR_EQ(s, "hello");
	APEBUILD_FREE(s);
}

TEST(str_ndup)
{
	char *s = ape_str_ndup("hello world", 5);
	ASSERT_NOT_NULL(s);
	ASSERT_STR_EQ(s, "hello");
	APEBUILD_FREE(s);
}

TEST(str_concat)
{
	char *s = ape_str_concat("hello", " world");
	ASSERT_NOT_NULL(s);
	ASSERT_STR_EQ(s, "hello world");
	APEBUILD_FREE(s);
}

TEST(str_eq)
{
	ASSERT(ape_str_eq("hello", "hello"));
	ASSERT(!ape_str_eq("hello", "world"));
	ASSERT(!ape_str_eq("hello", NULL));
	ASSERT(!ape_str_eq(NULL, "hello"));
	ASSERT(ape_str_eq(NULL, NULL));
}

TEST(str_eq_nocase)
{
	ASSERT(ape_str_eq_nocase("Hello", "hello"));
	ASSERT(ape_str_eq_nocase("HELLO", "hello"));
	ASSERT(!ape_str_eq_nocase("hello", "world"));
}

TEST(str_starts_with)
{
	ASSERT(ape_str_starts_with("hello world", "hello"));
	ASSERT(!ape_str_starts_with("hello world", "world"));
	ASSERT(ape_str_starts_with("hello", ""));
}

TEST(str_ends_with)
{
	ASSERT(ape_str_ends_with("hello world", "world"));
	ASSERT(!ape_str_ends_with("hello world", "hello"));
	ASSERT(ape_str_ends_with("hello", ""));
}

TEST(str_contains)
{
	ASSERT(ape_str_contains("hello world", "lo wo"));
	ASSERT(!ape_str_contains("hello world", "xyz"));
}

TEST(str_is_empty)
{
	ASSERT(ape_str_is_empty(NULL));
	ASSERT(ape_str_is_empty(""));
	ASSERT(!ape_str_is_empty("hello"));
}

TEST(str_trim)
{
	char *s = ape_str_trim("  hello world  ");
	ASSERT_STR_EQ(s, "hello world");
	APEBUILD_FREE(s);

	s = ape_str_trim("hello");
	ASSERT_STR_EQ(s, "hello");
	APEBUILD_FREE(s);
}

TEST(str_to_lower)
{
	char *s = ape_str_to_lower("Hello World");
	ASSERT_STR_EQ(s, "hello world");
	APEBUILD_FREE(s);
}

TEST(str_to_upper)
{
	char *s = ape_str_to_upper("Hello World");
	ASSERT_STR_EQ(s, "HELLO WORLD");
	APEBUILD_FREE(s);
}

TEST(str_replace)
{
	char *s = ape_str_replace("hello world", "world", "there");
	ASSERT_STR_EQ(s, "hello there");
	APEBUILD_FREE(s);
}

TEST(str_replace_all)
{
	char *s = ape_str_replace_all("hello hello hello", "hello", "hi");
	ASSERT_STR_EQ(s, "hi hi hi");
	APEBUILD_FREE(s);
}

TEST(str_find)
{
	ASSERT_EQ(ape_str_find("hello world", "world"), 6);
	ASSERT_EQ(ape_str_find("hello world", "xyz"), -1);
}

TEST(str_rfind)
{
	ASSERT_EQ(ape_str_rfind("hello hello", "hello"), 6);
}

TEST(str_split)
{
	ApeStrList list = ape_str_split("a,b,c", ",");
	ASSERT_EQ(ape_sl_len(&list), 3);
	ASSERT_STR_EQ(ape_sl_get(&list, 0), "a");
	ASSERT_STR_EQ(ape_sl_get(&list, 1), "b");
	ASSERT_STR_EQ(ape_sl_get(&list, 2), "c");
	ape_sl_free(&list);
}

TEST(str_to_int)
{
	int val;
	ASSERT(ape_str_to_int("123", &val));
	ASSERT_EQ(val, 123);

	ASSERT(ape_str_to_int("-456", &val));
	ASSERT_EQ(val, -456);

	ASSERT(!ape_str_to_int("abc", &val));
	ASSERT(!ape_str_to_int("12abc", &val));
}

/* ============================================================================
 * String Builder Tests
 * ============================================================================ */

TEST(sb_basic)
{
	ApeStrBuilder sb = ape_sb_new();
	ape_sb_append_str(&sb, "hello");
	ape_sb_append_char(&sb, ' ');
	ape_sb_append_str(&sb, "world");

	const char *str = ape_sb_to_str(&sb);
	ASSERT_STR_EQ(str, "hello world");
	ASSERT_EQ(ape_sb_len(&sb), 11);

	ape_sb_free(&sb);
}

TEST(sb_fmt)
{
	ApeStrBuilder sb = ape_sb_new();
	ape_sb_append_fmt(&sb, "value: %d", 42);

	const char *str = ape_sb_to_str(&sb);
	ASSERT_STR_EQ(str, "value: 42");

	ape_sb_free(&sb);
}

TEST(sb_prepend)
{
	ApeStrBuilder sb = ape_sb_new();
	ape_sb_append_str(&sb, "world");
	ape_sb_prepend_str(&sb, "hello ");

	const char *str = ape_sb_to_str(&sb);
	ASSERT_STR_EQ(str, "hello world");

	ape_sb_free(&sb);
}

/* ============================================================================
 * String List Tests
 * ============================================================================ */

TEST(sl_basic)
{
	ApeStrList sl = ape_sl_new();
	ape_sl_append_dup(&sl, "first");
	ape_sl_append_dup(&sl, "second");
	ape_sl_append_dup(&sl, "third");

	ASSERT_EQ(ape_sl_len(&sl), 3);
	ASSERT_STR_EQ(ape_sl_get(&sl, 0), "first");
	ASSERT_STR_EQ(ape_sl_get(&sl, 1), "second");
	ASSERT_STR_EQ(ape_sl_get(&sl, 2), "third");

	ape_sl_free(&sl);
}

TEST(sl_join)
{
	ApeStrList sl = ape_sl_new();
	ape_sl_append_dup(&sl, "a");
	ape_sl_append_dup(&sl, "b");
	ape_sl_append_dup(&sl, "c");

	char *joined = ape_sl_join(&sl, ", ");
	ASSERT_STR_EQ(joined, "a, b, c");

	APEBUILD_FREE(joined);
	ape_sl_free(&sl);
}

TEST(sl_contains)
{
	ApeStrList sl = ape_sl_new();
	ape_sl_append_dup(&sl, "apple");
	ape_sl_append_dup(&sl, "banana");

	ASSERT(ape_sl_contains(&sl, "apple"));
	ASSERT(ape_sl_contains(&sl, "banana"));
	ASSERT(!ape_sl_contains(&sl, "cherry"));

	ape_sl_free(&sl);
}

TEST(sl_remove)
{
	ApeStrList sl = ape_sl_new();
	ape_sl_append_dup(&sl, "a");
	ape_sl_append_dup(&sl, "b");
	ape_sl_append_dup(&sl, "c");

	ape_sl_remove(&sl, 1);

	ASSERT_EQ(ape_sl_len(&sl), 2);
	ASSERT_STR_EQ(ape_sl_get(&sl, 0), "a");
	ASSERT_STR_EQ(ape_sl_get(&sl, 1), "c");

	ape_sl_free(&sl);
}

/* ============================================================================
 * Filesystem Tests
 * ============================================================================ */

TEST(fs_join)
{
	char *path = ape_fs_join("/home", "user");
	ASSERT_STR_EQ(path, "/home/user");
	APEBUILD_FREE(path);

	path = ape_fs_join("/home/", "user");
	ASSERT_STR_EQ(path, "/home/user");
	APEBUILD_FREE(path);

	path = ape_fs_join("/home", "/user");
	ASSERT_STR_EQ(path, "/home/user");
	APEBUILD_FREE(path);
}

TEST(fs_dirname)
{
	char *dir = ape_fs_dirname("/home/user/file.txt");
	ASSERT_STR_EQ(dir, "/home/user");
	APEBUILD_FREE(dir);

	dir = ape_fs_dirname("file.txt");
	ASSERT_STR_EQ(dir, ".");
	APEBUILD_FREE(dir);

	dir = ape_fs_dirname("/file.txt");
	ASSERT_STR_EQ(dir, "/");
	APEBUILD_FREE(dir);
}

TEST(fs_basename)
{
	char *base = ape_fs_basename("/home/user/file.txt");
	ASSERT_STR_EQ(base, "file.txt");
	APEBUILD_FREE(base);

	base = ape_fs_basename("file.txt");
	ASSERT_STR_EQ(base, "file.txt");
	APEBUILD_FREE(base);
}

TEST(fs_extension)
{
	char *ext = ape_fs_extension("/home/user/file.txt");
	ASSERT_STR_EQ(ext, ".txt");
	APEBUILD_FREE(ext);

	ext = ape_fs_extension("file.tar.gz");
	ASSERT_STR_EQ(ext, ".gz");
	APEBUILD_FREE(ext);

	ext = ape_fs_extension("file");
	ASSERT_STR_EQ(ext, "");
	APEBUILD_FREE(ext);
}

TEST(fs_stem)
{
	char *stem = ape_fs_stem("/home/user/file.txt");
	ASSERT_STR_EQ(stem, "file");
	APEBUILD_FREE(stem);

	stem = ape_fs_stem("file");
	ASSERT_STR_EQ(stem, "file");
	APEBUILD_FREE(stem);
}

TEST(fs_change_extension)
{
	char *path = ape_fs_change_extension("file.c", ".o");
	ASSERT_STR_EQ(path, "file.o");
	APEBUILD_FREE(path);

	path = ape_fs_change_extension("src/file.c", "o");
	ASSERT_STR_EQ(path, "src/file.o");
	APEBUILD_FREE(path);
}

TEST(fs_normalize)
{
	char *path = ape_fs_normalize("/home/user/../user/./file.txt");
	ASSERT_STR_EQ(path, "/home/user/file.txt");
	APEBUILD_FREE(path);

	path = ape_fs_normalize("./foo/bar/../baz");
	ASSERT_STR_EQ(path, "foo/baz");
	APEBUILD_FREE(path);
}

TEST(fs_is_absolute)
{
	ASSERT(ape_fs_is_absolute("/home/user"));
	ASSERT(!ape_fs_is_absolute("home/user"));
	ASSERT(!ape_fs_is_absolute("./file"));
}

TEST(fs_exists)
{
	ASSERT(ape_fs_exists("/tmp"));
	ASSERT(!ape_fs_exists("/nonexistent/path/that/should/not/exist"));
}

TEST(fs_is_dir)
{
	ASSERT(ape_fs_is_dir("/tmp"));
}

TEST(fs_write_read)
{
	char *tmpfile = ape_fs_temp_file("test");
	ASSERT_NOT_NULL(tmpfile);

	const char *content = "Hello, World!\n";
	ASSERT(ape_fs_write_file(tmpfile, content, strlen(content)));

	size_t size;
	char *read_content = ape_fs_read_file(tmpfile, &size);
	ASSERT_NOT_NULL(read_content);
	ASSERT_EQ(size, strlen(content));
	ASSERT_STR_EQ(read_content, content);

	APEBUILD_FREE(read_content);
	ape_fs_remove(tmpfile);
	APEBUILD_FREE(tmpfile);
}

TEST(fs_needs_rebuild)
{
	/* Create two temp files */
	char *file1 = ape_fs_temp_file("test1");
	char *file2 = ape_fs_temp_file("test2");

	/* Write to both with longer delay to ensure different mtime */
	ape_fs_write_file(file1, "a", 1);
	sleep(1); /* Wait 1 second to ensure different mtime */
	ape_fs_write_file(file2, "b", 1);

	/* file2 is newer than file1, so file1 needs rebuild */
	ASSERT(ape_fs_needs_rebuild1(file1, file2));
	/* file2 doesn't need rebuild from file1 */
	ASSERT(!ape_fs_needs_rebuild1(file2, file1));

	ape_fs_remove(file1);
	ape_fs_remove(file2);
	APEBUILD_FREE(file1);
	APEBUILD_FREE(file2);
}

/* ============================================================================
 * Command Module Tests
 * ============================================================================ */

TEST(cmd_basic)
{
	ApeCmd cmd = ape_cmd_new();
	ape_cmd_append(&cmd, "echo");
	ape_cmd_append(&cmd, "hello");

	ASSERT_EQ(cmd.count, 2);
	ASSERT_STR_EQ(cmd.items[0], "echo");
	ASSERT_STR_EQ(cmd.items[1], "hello");

	ape_cmd_free(&cmd);
}

TEST(cmd_from)
{
	ApeCmd cmd = ape_cmd_from("gcc");
	ape_cmd_append(&cmd, "-c");
	ape_cmd_append(&cmd, "file.c");

	ASSERT_EQ(cmd.count, 3);
	ASSERT_STR_EQ(cmd.items[0], "gcc");

	ape_cmd_free(&cmd);
}

TEST(cmd_render)
{
	ApeCmd cmd = ape_cmd_new();
	ape_cmd_append(&cmd, "gcc");
	ape_cmd_append(&cmd, "-o");
	ape_cmd_append(&cmd, "output");
	ape_cmd_append(&cmd, "file.c");

	char *rendered = ape_cmd_render(&cmd);
	ASSERT_STR_EQ(rendered, "gcc -o output file.c");

	APEBUILD_FREE(rendered);
	ape_cmd_free(&cmd);
}

TEST(cmd_run)
{
	ApeCmd cmd = ape_cmd_from("true");
	ASSERT(ape_cmd_run(&cmd));
	ape_cmd_free(&cmd);

	cmd = ape_cmd_from("false");
	ASSERT(!ape_cmd_run(&cmd));
	ape_cmd_free(&cmd);
}

TEST(cmd_run_capture)
{
	ApeCmd cmd = ape_cmd_from("echo");
	ape_cmd_append(&cmd, "hello");

	int exit_code;
	char *output = ape_cmd_run_capture(&cmd, &exit_code);

	ASSERT_NOT_NULL(output);
	ASSERT_EQ(exit_code, 0);
	ASSERT(ape_str_starts_with(output, "hello"));

	APEBUILD_FREE(output);
	ape_cmd_free(&cmd);
}

TEST(cmd_async)
{
	ApeCmd cmd = ape_cmd_from("sleep");
	ape_cmd_append(&cmd, "0.1");

	ApeProcHandle handle = ape_cmd_start(&cmd);
	ASSERT_NE(handle, APE_INVALID_HANDLE);

	/* Should still be running immediately */
	ASSERT(!ape_proc_poll(handle));
	ASSERT_EQ(ape_proc_status(handle), APE_PROC_RUNNING);

	/* Wait for completion */
	ASSERT(ape_proc_wait(handle));

	ApeProcResult result = ape_proc_result(handle);
	ASSERT_EQ(result.status, APE_PROC_COMPLETED);
	ASSERT_EQ(result.exit_code, 0);

	ape_proc_handle_release(handle);
	ape_cmd_free(&cmd);
}

TEST(cmd_cwd)
{
	ApeCmd cmd = ape_cmd_from("pwd");
	ape_cmd_set_cwd(&cmd, "/tmp");

	int exit_code;
	char *output = ape_cmd_run_capture(&cmd, &exit_code);

	ASSERT_NOT_NULL(output);
	ASSERT(ape_str_starts_with(output, "/tmp"));

	APEBUILD_FREE(output);
	ape_cmd_free(&cmd);
}

/* ============================================================================
 * Logging Module Tests
 * ============================================================================ */

TEST(log_level_name)
{
	ASSERT_STR_EQ(ape_log_level_name(APE_LOG_INFO), "INFO");
	ASSERT_STR_EQ(ape_log_level_name(APE_LOG_ERROR), "ERROR");
}

TEST(log_level_from_name)
{
	ApeLogLevel level;
	ASSERT(ape_log_level_from_name("info", &level));
	ASSERT_EQ(level, APE_LOG_INFO);

	ASSERT(ape_log_level_from_name("ERROR", &level));
	ASSERT_EQ(level, APE_LOG_ERROR);

	ASSERT(!ape_log_level_from_name("invalid", &level));
}

TEST(log_set_level)
{
	ape_log_init();

	ApeLogLevel old = ape_log_get_level();

	ape_log_set_level(APE_LOG_ERROR);
	ASSERT_EQ(ape_log_get_level(), APE_LOG_ERROR);

	ape_log_set_level(old);
}

/* ============================================================================
 * Download Module Tests
 * ============================================================================ */

TEST(dl_fetch)
{
	ape_dl_fetch(APE_URL("raw.githubusercontent.com/aamosljp/apelibs/refs/heads/master/src/apebuild/test_file", "/aamosljp/apelibs"),
		     "/tmp/apebuild", 1024, NULL);
	ASSERT_FILE_EXISTS("/tmp/apebuild/test_file");
	FILE *fp = fopen("/tmp/apebuild/test_file", "r");
	FILE *ofp = fopen("src/apebuild/test_file", "r");
	char buf1[1024];
	char buf2[1024];
	size_t len1 = fread(buf1, 1, sizeof(buf1), fp);
	size_t len2 = fread(buf2, 1, sizeof(buf2), ofp);
	ASSERT_EQ(len1, len2);
	ASSERT_STR_EQ(buf1, buf2);
	fclose(fp);
	fclose(ofp);
	remove("/tmp/apebuild/test_file");
}

/* ============================================================================
 * Test Runner
 * ============================================================================ */

static void run_string_tests(void)
{
	printf("String module tests:\n");
	RUN_TEST(str_dup);
	RUN_TEST(str_ndup);
	RUN_TEST(str_concat);
	RUN_TEST(str_eq);
	RUN_TEST(str_eq_nocase);
	RUN_TEST(str_starts_with);
	RUN_TEST(str_ends_with);
	RUN_TEST(str_contains);
	RUN_TEST(str_is_empty);
	RUN_TEST(str_trim);
	RUN_TEST(str_to_lower);
	RUN_TEST(str_to_upper);
	RUN_TEST(str_replace);
	RUN_TEST(str_replace_all);
	RUN_TEST(str_find);
	RUN_TEST(str_rfind);
	RUN_TEST(str_split);
	RUN_TEST(str_to_int);
	printf("\n");
}

static void run_sb_tests(void)
{
	printf("String builder tests:\n");
	RUN_TEST(sb_basic);
	RUN_TEST(sb_fmt);
	RUN_TEST(sb_prepend);
	printf("\n");
}

static void run_sl_tests(void)
{
	printf("String list tests:\n");
	RUN_TEST(sl_basic);
	RUN_TEST(sl_join);
	RUN_TEST(sl_contains);
	RUN_TEST(sl_remove);
	printf("\n");
}

static void run_fs_tests(void)
{
	printf("Filesystem module tests:\n");
	RUN_TEST(fs_join);
	RUN_TEST(fs_dirname);
	RUN_TEST(fs_basename);
	RUN_TEST(fs_extension);
	RUN_TEST(fs_stem);
	RUN_TEST(fs_change_extension);
	RUN_TEST(fs_normalize);
	RUN_TEST(fs_is_absolute);
	RUN_TEST(fs_exists);
	RUN_TEST(fs_is_dir);
	RUN_TEST(fs_write_read);
	RUN_TEST(fs_needs_rebuild);
	printf("\n");
}

static void run_cmd_tests(void)
{
	printf("Command module tests:\n");
	RUN_TEST(cmd_basic);
	RUN_TEST(cmd_from);
	RUN_TEST(cmd_render);
	RUN_TEST(cmd_run);
	RUN_TEST(cmd_run_capture);
	RUN_TEST(cmd_async);
	RUN_TEST(cmd_cwd);
	printf("\n");
}

static void run_log_tests(void)
{
	printf("Logging module tests:\n");
	RUN_TEST(log_level_name);
	RUN_TEST(log_level_from_name);
	RUN_TEST(log_set_level);
	printf("\n");
}

static void run_dl_tests(void)
{
	printf("Download module tests:\n");
	RUN_TEST(dl_fetch);
	printf("\n");
}

int main(void)
{
	printf("=== Apebuild Unit Tests ===\n\n");

	run_string_tests();
	run_sb_tests();
	run_sl_tests();
	run_fs_tests();
	run_cmd_tests();
	run_log_tests();
	run_dl_tests();

	printf("=== Results ===\n");
	printf("Tests run:    %d\n", tests_run);
	printf("Tests passed: %d\n", tests_passed);
	printf("Tests failed: %d\n", tests_failed);

	return tests_failed > 0 ? 1 : 0;
}
