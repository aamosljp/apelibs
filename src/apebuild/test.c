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

#define FAILED 1
#define PASSED 0

#define TEST(name)                                                             \
	static int test_##name(void);                                          \
	static void run_test_##name(void) {                                    \
		tests_run++;                                                   \
		if (test_##name() == FAILED) {                                 \
			tests_failed++;                                        \
			ape_log_info("  Testing %s... \x1b[31mFAILED", #name); \
		} else {                                                       \
			tests_passed++;                                        \
			ape_log_info("  Testing %s... \x1b[32mPASSED", #name); \
		}                                                              \
	}                                                                      \
	static int test_##name(void)

#define ASSERT(cond)                                                    \
	do {                                                            \
		if (!(cond)) {                                          \
			ape_log_info("  Assertion failed: %s", #cond);  \
			ape_log_info("  At %s:%d", __FILE__, __LINE__); \
			return FAILED;                                  \
		}                                                       \
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

TEST(str_dup) {
	char *s = ape_str_dup("hello");
	ASSERT_NOT_NULL(s);
	ASSERT_STR_EQ(s, "hello");
	APEBUILD_FREE(s);
	return PASSED;
}

TEST(str_ndup) {
	char *s = ape_str_ndup("hello world", 5);
	ASSERT_NOT_NULL(s);
	ASSERT_STR_EQ(s, "hello");
	APEBUILD_FREE(s);
	return PASSED;
}

TEST(str_concat) {
	char *s = ape_str_concat("hello", " world");
	ASSERT_NOT_NULL(s);
	ASSERT_STR_EQ(s, "hello world");
	APEBUILD_FREE(s);
	return PASSED;
}

TEST(str_eq) {
	ASSERT(ape_str_eq("hello", "hello"));
	ASSERT(!ape_str_eq("hello", "world"));
	ASSERT(!ape_str_eq("hello", NULL));
	ASSERT(!ape_str_eq(NULL, "hello"));
	ASSERT(ape_str_eq(NULL, NULL));
	return PASSED;
}

TEST(str_eq_nocase) {
	ASSERT(ape_str_eq_nocase("Hello", "hello"));
	ASSERT(ape_str_eq_nocase("HELLO", "hello"));
	ASSERT(!ape_str_eq_nocase("hello", "world"));
	return PASSED;
}

TEST(str_starts_with) {
	ASSERT(ape_str_starts_with("hello world", "hello"));
	ASSERT(!ape_str_starts_with("hello world", "world"));
	ASSERT(ape_str_starts_with("hello", ""));
	return PASSED;
}

TEST(str_ends_with) {
	ASSERT(ape_str_ends_with("hello world", "world"));
	ASSERT(!ape_str_ends_with("hello world", "hello"));
	ASSERT(ape_str_ends_with("hello", ""));
	return PASSED;
}

TEST(str_contains) {
	ASSERT(ape_str_contains("hello world", "lo wo"));
	ASSERT(!ape_str_contains("hello world", "xyz"));
	return PASSED;
}

TEST(str_is_empty) {
	ASSERT(ape_str_is_empty(NULL));
	ASSERT(ape_str_is_empty(""));
	ASSERT(!ape_str_is_empty("hello"));
	return PASSED;
}

TEST(str_trim) {
	char *s = ape_str_trim("  hello world  ");
	ASSERT_STR_EQ(s, "hello world");
	APEBUILD_FREE(s);

	s = ape_str_trim("hello");
	ASSERT_STR_EQ(s, "hello");
	APEBUILD_FREE(s);
	return PASSED;
}

TEST(str_to_lower) {
	char *s = ape_str_to_lower("Hello World");
	ASSERT_STR_EQ(s, "hello world");
	APEBUILD_FREE(s);
	return PASSED;
}

TEST(str_to_upper) {
	char *s = ape_str_to_upper("Hello World");
	ASSERT_STR_EQ(s, "HELLO WORLD");
	APEBUILD_FREE(s);
	return PASSED;
}

TEST(str_replace) {
	char *s = ape_str_replace("hello world", "world", "there");
	ASSERT_STR_EQ(s, "hello there");
	APEBUILD_FREE(s);
	return PASSED;
}

TEST(str_replace_all) {
	char *s = ape_str_replace_all("hello hello hello", "hello", "hi");
	ASSERT_STR_EQ(s, "hi hi hi");
	APEBUILD_FREE(s);
	return PASSED;
}

TEST(str_find) {
	ASSERT_EQ(ape_str_find("hello world", "world"), 6);
	ASSERT_EQ(ape_str_find("hello world", "xyz"), -1);
	return PASSED;
}

TEST(str_rfind) {
	ASSERT_EQ(ape_str_rfind("hello hello", "hello"), 6);
	return PASSED;
}

TEST(str_split) {
	ApeStrList list = ape_str_split("a,b,c", ",");
	ASSERT_EQ(ape_sl_len(&list), 3);
	ASSERT_STR_EQ(ape_sl_get(&list, 0), "a");
	ASSERT_STR_EQ(ape_sl_get(&list, 1), "b");
	ASSERT_STR_EQ(ape_sl_get(&list, 2), "c");
	ape_sl_free(&list);
	return PASSED;
}

TEST(str_to_int) {
	int val;
	ASSERT(ape_str_to_int("123", &val));
	ASSERT_EQ(val, 123);

	ASSERT(ape_str_to_int("-456", &val));
	ASSERT_EQ(val, -456);

	ASSERT(!ape_str_to_int("abc", &val));
	ASSERT(!ape_str_to_int("12abc", &val));
	return PASSED;
}

/* ============================================================================
 * String Builder Tests
 * ============================================================================ */

TEST(sb_basic) {
	ApeStrBuilder sb = ape_sb_new();
	ape_sb_append_str(&sb, "hello");
	ape_sb_append_char(&sb, ' ');
	ape_sb_append_str(&sb, "world");

	const char *str = ape_sb_to_str(&sb);
	ASSERT_STR_EQ(str, "hello world");
	ASSERT_EQ(ape_sb_len(&sb), 11);

	ape_sb_free(&sb);
	return PASSED;
}

TEST(sb_fmt) {
	ApeStrBuilder sb = ape_sb_new();
	ape_sb_append_fmt(&sb, "value: %d", 42);

	const char *str = ape_sb_to_str(&sb);
	ASSERT_STR_EQ(str, "value: 42");

	ape_sb_free(&sb);
	return PASSED;
}

TEST(sb_prepend) {
	ApeStrBuilder sb = ape_sb_new();
	ape_sb_append_str(&sb, "world");
	ape_sb_prepend_str(&sb, "hello ");

	const char *str = ape_sb_to_str(&sb);
	ASSERT_STR_EQ(str, "hello world");

	ape_sb_free(&sb);
	return PASSED;
}

/* ============================================================================
 * String List Tests
 * ============================================================================ */

TEST(sl_basic) {
	ApeStrList sl = ape_sl_new();
	ape_sl_append_dup(&sl, "first");
	ape_sl_append_dup(&sl, "second");
	ape_sl_append_dup(&sl, "third");

	ASSERT_EQ(ape_sl_len(&sl), 3);
	ASSERT_STR_EQ(ape_sl_get(&sl, 0), "first");
	ASSERT_STR_EQ(ape_sl_get(&sl, 1), "second");
	ASSERT_STR_EQ(ape_sl_get(&sl, 2), "third");

	ape_sl_free(&sl);
	return PASSED;
}

TEST(sl_join) {
	ApeStrList sl = ape_sl_new();
	ape_sl_append_dup(&sl, "a");
	ape_sl_append_dup(&sl, "b");
	ape_sl_append_dup(&sl, "c");

	char *joined = ape_sl_join(&sl, ", ");
	ASSERT_STR_EQ(joined, "a, b, c");

	APEBUILD_FREE(joined);
	ape_sl_free(&sl);
	return PASSED;
}

TEST(sl_contains) {
	ApeStrList sl = ape_sl_new();
	ape_sl_append_dup(&sl, "apple");
	ape_sl_append_dup(&sl, "banana");

	ASSERT(ape_sl_contains(&sl, "apple"));
	ASSERT(ape_sl_contains(&sl, "banana"));
	ASSERT(!ape_sl_contains(&sl, "cherry"));

	ape_sl_free(&sl);
	return PASSED;
}

TEST(sl_remove) {
	ApeStrList sl = ape_sl_new();
	ape_sl_append_dup(&sl, "a");
	ape_sl_append_dup(&sl, "b");
	ape_sl_append_dup(&sl, "c");

	ape_sl_remove(&sl, 1);

	ASSERT_EQ(ape_sl_len(&sl), 2);
	ASSERT_STR_EQ(ape_sl_get(&sl, 0), "a");
	ASSERT_STR_EQ(ape_sl_get(&sl, 1), "c");

	ape_sl_free(&sl);
	return PASSED;
}

/* ============================================================================
 * Filesystem Tests
 * ============================================================================ */

TEST(fs_join) {
	char *path = ape_fs_join("/home", "user");
	ASSERT_STR_EQ(path, "/home/user");
	APEBUILD_FREE(path);

	path = ape_fs_join("/home/", "user");
	ASSERT_STR_EQ(path, "/home/user");
	APEBUILD_FREE(path);

	path = ape_fs_join("/home", "/user");
	ASSERT_STR_EQ(path, "/home/user");
	APEBUILD_FREE(path);
	return PASSED;
}

TEST(fs_dirname) {
	char *dir = ape_fs_dirname("/home/user/file.txt");
	ASSERT_STR_EQ(dir, "/home/user");
	APEBUILD_FREE(dir);

	dir = ape_fs_dirname("file.txt");
	ASSERT_STR_EQ(dir, ".");
	APEBUILD_FREE(dir);

	dir = ape_fs_dirname("/file.txt");
	ASSERT_STR_EQ(dir, "/");
	APEBUILD_FREE(dir);
	return PASSED;
}

TEST(fs_basename) {
	char *base = ape_fs_basename("/home/user/file.txt");
	ASSERT_STR_EQ(base, "file.txt");
	APEBUILD_FREE(base);

	base = ape_fs_basename("file.txt");
	ASSERT_STR_EQ(base, "file.txt");
	APEBUILD_FREE(base);
	return PASSED;
}

TEST(fs_extension) {
	char *ext = ape_fs_extension("/home/user/file.txt");
	ASSERT_STR_EQ(ext, ".txt");
	APEBUILD_FREE(ext);

	ext = ape_fs_extension("file.tar.gz");
	ASSERT_STR_EQ(ext, ".gz");
	APEBUILD_FREE(ext);

	ext = ape_fs_extension("file");
	ASSERT_STR_EQ(ext, "");
	APEBUILD_FREE(ext);
	return PASSED;
}

TEST(fs_stem) {
	char *stem = ape_fs_stem("/home/user/file.txt");
	ASSERT_STR_EQ(stem, "file");
	APEBUILD_FREE(stem);

	stem = ape_fs_stem("file");
	ASSERT_STR_EQ(stem, "file");
	APEBUILD_FREE(stem);
	return PASSED;
}

TEST(fs_change_extension) {
	char *path = ape_fs_change_extension("file.c", ".o");
	ASSERT_STR_EQ(path, "file.o");
	APEBUILD_FREE(path);

	path = ape_fs_change_extension("src/file.c", "o");
	ASSERT_STR_EQ(path, "src/file.o");
	APEBUILD_FREE(path);
	return PASSED;
}

TEST(fs_normalize) {
	char *path = ape_fs_normalize("/home/user/../user/./file.txt");
	ASSERT_STR_EQ(path, "/home/user/file.txt");
	APEBUILD_FREE(path);

	path = ape_fs_normalize("./foo/bar/../baz");
	ASSERT_STR_EQ(path, "foo/baz");
	APEBUILD_FREE(path);
	return PASSED;
}

TEST(fs_is_absolute) {
	ASSERT(ape_fs_is_absolute("/home/user"));
	ASSERT(!ape_fs_is_absolute("home/user"));
	ASSERT(!ape_fs_is_absolute("./file"));
	return PASSED;
}

TEST(fs_exists) {
	ASSERT(ape_fs_exists("/tmp"));
	ASSERT(!ape_fs_exists("/nonexistent/path/that/should/not/exist"));
	return PASSED;
}

TEST(fs_is_dir) {
	ASSERT(ape_fs_is_dir("/tmp"));
	return PASSED;
}

TEST(fs_write_read) {
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
	return PASSED;
}

TEST(fs_needs_rebuild) {
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
	return PASSED;
}

/* ============================================================================
 * Command Module Tests
 * ============================================================================ */

TEST(cmd_basic) {
	ApeCmd cmd = ape_cmd_new();
	ape_cmd_append(&cmd, "echo");
	ape_cmd_append(&cmd, "hello");

	ASSERT_EQ(cmd.count, 2);
	ASSERT_STR_EQ(cmd.items[0], "echo");
	ASSERT_STR_EQ(cmd.items[1], "hello");

	ape_cmd_free(&cmd);
	return PASSED;
}

TEST(cmd_from) {
	ApeCmd cmd = ape_cmd_from("gcc");
	ape_cmd_append(&cmd, "-c");
	ape_cmd_append(&cmd, "file.c");

	ASSERT_EQ(cmd.count, 3);
	ASSERT_STR_EQ(cmd.items[0], "gcc");

	ape_cmd_free(&cmd);
	return PASSED;
}

TEST(cmd_render) {
	ApeCmd cmd = ape_cmd_new();
	ape_cmd_append(&cmd, "gcc");
	ape_cmd_append(&cmd, "-o");
	ape_cmd_append(&cmd, "output");
	ape_cmd_append(&cmd, "file.c");

	char *rendered = ape_cmd_render(&cmd);
	ASSERT_STR_EQ(rendered, "gcc -o output file.c");

	APEBUILD_FREE(rendered);
	ape_cmd_free(&cmd);
	return PASSED;
}

TEST(cmd_run) {
	ApeCmd cmd = ape_cmd_from("true");
	ASSERT(ape_cmd_run(&cmd));
	ape_cmd_free(&cmd);

	cmd = ape_cmd_from("false");
	ASSERT(!ape_cmd_run(&cmd));
	ape_cmd_free(&cmd);
	return PASSED;
}

TEST(cmd_run_capture) {
	ApeCmd cmd = ape_cmd_from("echo");
	ape_cmd_append(&cmd, "hello");

	int exit_code;
	char *output = ape_cmd_run_capture(&cmd, &exit_code);

	ASSERT_NOT_NULL(output);
	ASSERT_EQ(exit_code, 0);
	ASSERT(ape_str_starts_with(output, "hello"));

	APEBUILD_FREE(output);
	ape_cmd_free(&cmd);
	return PASSED;
}

TEST(cmd_async) {
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
	return PASSED;
}

TEST(cmd_cwd) {
	ApeCmd cmd = ape_cmd_from("pwd");
	ape_cmd_set_cwd(&cmd, "/tmp");

	int exit_code;
	char *output = ape_cmd_run_capture(&cmd, &exit_code);

	ASSERT_NOT_NULL(output);
	ASSERT(ape_str_starts_with(output, "/tmp"));

	APEBUILD_FREE(output);
	ape_cmd_free(&cmd);
	return PASSED;
}

/* ============================================================================
 * Logging Module Tests
 * ============================================================================ */

TEST(log_level_name) {
	ASSERT_STR_EQ(ape_log_level_name(APE_LOG_INFO), "INFO");
	ASSERT_STR_EQ(ape_log_level_name(APE_LOG_ERROR), "ERROR");
	return PASSED;
}

TEST(log_level_from_name) {
	ApeLogLevel level;
	ASSERT(ape_log_level_from_name("info", &level));
	ASSERT_EQ(level, APE_LOG_INFO);

	ASSERT(ape_log_level_from_name("ERROR", &level));
	ASSERT_EQ(level, APE_LOG_ERROR);

	ASSERT(!ape_log_level_from_name("invalid", &level));
	return PASSED;
}

TEST(log_set_level) {
	ape_log_init();

	ApeLogLevel old = ape_log_get_level();

	ape_log_set_level(APE_LOG_ERROR);
	ASSERT_EQ(ape_log_get_level(), APE_LOG_ERROR);

	ape_log_set_level(old);
	return PASSED;
}

/* ============================================================================
 * Emscripten/WASM Toolchain Tests
 * ============================================================================ */

TEST(emcc_toolchain_creation) {
	ape_build_reset();
	ApeToolchainHandle tc = ape_toolchain_emcc();
	ASSERT_NE(tc, APE_INVALID_TOOLCHAIN);

	ApeToolchain *toolchain = ape_toolchain_get(tc);
	ASSERT_NOT_NULL(toolchain);
	ASSERT_STR_EQ(toolchain->name, "emcc");
	ASSERT_STR_EQ(toolchain->cc, "emcc");
	ASSERT_STR_EQ(toolchain->cxx, "em++");
	ASSERT_STR_EQ(toolchain->ld, "emcc");
	ASSERT_STR_EQ(toolchain->ar, "emar");
	ASSERT_STR_EQ(toolchain->exe_ext, ".html");
	ASSERT_STR_EQ(toolchain->shared_lib_ext, ".js");
	ASSERT_STR_EQ(toolchain->obj_ext, ".o");
	ASSERT_STR_EQ(toolchain->static_lib_ext, ".a");

	ape_build_reset();
	return PASSED;
}

TEST(emcc_builder_wasm_output_path) {
	ape_build_reset();
	ApeToolchainHandle tc = ape_toolchain_emcc();
	ApeBuilderHandle builder = ape_builder_new("myapp");
	ape_builder_set_type(builder, APE_TARGET_WASM);
	ape_builder_set_toolchain(builder, tc);
	ape_builder_set_output_dir(builder, "build_wasm");

	/* Default: HTML output */
	char *path = ape_builder_output_path(builder);
	ASSERT_NOT_NULL(path);
	ASSERT_STR_EQ(path, "build_wasm/myapp.html");
	APEBUILD_FREE(path);

	/* Switch to JS-only output */
	ape_builder_set_wasm_output_mode(builder, 0);
	path = ape_builder_output_path(builder);
	ASSERT_NOT_NULL(path);
	ASSERT_STR_EQ(path, "build_wasm/myapp.js");
	APEBUILD_FREE(path);

	ape_build_reset();
	return PASSED;
}

TEST(emcc_builder_shell_file) {
	ape_build_reset();
	ApeBuilderHandle builder = ape_builder_new("myapp");

	/* Initially NULL */
	ApeBuilder *b = ape_builder_get(builder);
	ASSERT_NOT_NULL(b);
	ASSERT_NULL(b->shell_file);

	/* Set custom shell file */
	ape_builder_set_shell_file(builder, "my_shell.html");
	b = ape_builder_get(builder);
	ASSERT_STR_EQ(b->shell_file, "my_shell.html");

	ape_build_reset();
	return PASSED;
}

TEST(emcc_builder_exported_functions) {
	ape_build_reset();
	ApeBuilderHandle builder = ape_builder_new("myapp");

	ape_builder_add_exported_function(builder, "_main");
	ape_builder_add_exported_function(builder, "_my_function");

	ApeBuilder *b = ape_builder_get(builder);
	ASSERT_NOT_NULL(b);
	ASSERT_EQ(b->exported_functions.count, 2);
	ASSERT_STR_EQ(b->exported_functions.items[0], "_main");
	ASSERT_STR_EQ(b->exported_functions.items[1], "_my_function");

	ape_build_reset();
	return PASSED;
}

TEST(emcc_builder_preload_embed_files) {
	ape_build_reset();
	ApeBuilderHandle builder = ape_builder_new("myapp");

	ape_builder_add_preload_file(builder, "assets/texture.png");
	ape_builder_add_preload_file(builder, "assets/music.ogg");
	ape_builder_add_embed_file(builder, "config.json");

	ApeBuilder *b = ape_builder_get(builder);
	ASSERT_NOT_NULL(b);
	ASSERT_EQ(b->preload_files.count, 2);
	ASSERT_STR_EQ(b->preload_files.items[0], "assets/texture.png");
	ASSERT_STR_EQ(b->preload_files.items[1], "assets/music.ogg");
	ASSERT_EQ(b->embed_files.count, 1);
	ASSERT_STR_EQ(b->embed_files.items[0], "config.json");

	ape_build_reset();
	return PASSED;
}

TEST(emcc_builder_memory_settings) {
	ape_build_reset();
	ApeBuilderHandle builder = ape_builder_new("myapp");

	/* Initially zero (use emcc defaults) */
	ApeBuilder *b = ape_builder_get(builder);
	ASSERT_EQ(b->wasm_initial_memory, 0);
	ASSERT_EQ(b->wasm_max_memory, 0);

	/* Set memory */
	ape_builder_set_wasm_memory(builder, 64, 256);
	b = ape_builder_get(builder);
	ASSERT_EQ(b->wasm_initial_memory, 64);
	ASSERT_EQ(b->wasm_max_memory, 256);

	ape_build_reset();
	return PASSED;
}

TEST(emcc_default_shell) {
	const char *shell = ape_emcc_default_shell();
	ASSERT_NOT_NULL(shell);

	/* Verify it contains the expected markers */
	ASSERT(ape_str_contains(shell, "{{{ SCRIPT }}}"));
	ASSERT(ape_str_contains(shell, "<!doctype html>"));
	ASSERT(ape_str_contains(shell, "<canvas"));
	ASSERT(ape_str_contains(shell, "Module"));

	return PASSED;
}

TEST(emcc_task_generation) {
	ape_build_reset();
	ApeToolchainHandle tc = ape_toolchain_emcc();
	ApeBuilderHandle builder = ape_builder_new("myapp");
	ape_builder_set_type(builder, APE_TARGET_WASM);
	ape_builder_set_toolchain(builder, tc);
	ape_builder_set_output_dir(builder, "/tmp/ape_test_wasm_build");

	/* Add a fake source file - we just test task generation, not execution */
	ape_builder_add_source(builder, "main.c");

	/* Configure emcc-specific options */
	ape_builder_add_exported_function(builder, "_main");
	ape_builder_add_exported_function(builder, "_my_func");
	ape_builder_set_wasm_memory(builder, 32, 128);
	ape_builder_add_preload_file(builder, "assets/data.bin");
	ape_builder_add_embed_file(builder, "config.txt");

	/* Generate tasks (doesn't execute them) */
	ape_builder_generate_tasks(builder);

	ApeBuilder *b = ape_builder_get(builder);
	ASSERT_NOT_NULL(b);
	ASSERT_EQ(b->tasks.count, 2); /* 1 compile + 1 link */

	/* Check compile task uses emcc */
	ApeTask *compile_task = ape_task_get(b->tasks.items[0]);
	ASSERT_NOT_NULL(compile_task);
	ASSERT_EQ(compile_task->type, APE_TASK_TYPE_COMPILE);
	ASSERT_STR_EQ(compile_task->cmd.items[0], "emcc");

	/* Check link task uses emcc and has wasm-specific flags */
	ApeTask *link_task = ape_task_get(b->tasks.items[1]);
	ASSERT_NOT_NULL(link_task);
	ASSERT_EQ(link_task->type, APE_TASK_TYPE_LINK);
	ASSERT_STR_EQ(link_task->cmd.items[0], "emcc");

	/* Verify the link command contains wasm-specific flags */
	char *link_cmd_str = ape_cmd_render(&link_task->cmd);
	ASSERT_NOT_NULL(link_cmd_str);
	ASSERT(ape_str_contains(link_cmd_str, "--shell-file"));
	ASSERT(ape_str_contains(link_cmd_str, "-sEXPORTED_FUNCTIONS=_main,_my_func"));
	ASSERT(ape_str_contains(link_cmd_str, "--preload-file"));
	ASSERT(ape_str_contains(link_cmd_str, "assets/data.bin"));
	ASSERT(ape_str_contains(link_cmd_str, "--embed-file"));
	ASSERT(ape_str_contains(link_cmd_str, "config.txt"));
	ASSERT(ape_str_contains(link_cmd_str, "-sINITIAL_MEMORY="));
	ASSERT(ape_str_contains(link_cmd_str, "-sALLOW_MEMORY_GROWTH=1"));
	ASSERT(ape_str_contains(link_cmd_str, "-sMAXIMUM_MEMORY="));
	ASSERT(ape_str_ends_with(link_task->output, ".html"));
	APEBUILD_FREE(link_cmd_str);

	/* Clean up generated shell file */
	ape_fs_remove("/tmp/ape_test_wasm_build/ape_shell_default.html");
	ape_fs_rmdir("/tmp/ape_test_wasm_build");

	ape_build_reset();
	return PASSED;
}

TEST(emcc_custom_shell_file) {
	ape_build_reset();
	ApeToolchainHandle tc = ape_toolchain_emcc();
	ApeBuilderHandle builder = ape_builder_new("myapp");
	ape_builder_set_type(builder, APE_TARGET_WASM);
	ape_builder_set_toolchain(builder, tc);
	ape_builder_set_output_dir(builder, "/tmp/ape_test_wasm_build2");
	ape_builder_set_shell_file(builder, "custom_shell.html");

	ape_builder_add_source(builder, "main.c");
	ape_builder_generate_tasks(builder);

	ApeBuilder *b = ape_builder_get(builder);
	ApeTask *link_task = ape_task_get(b->tasks.items[1]);
	ASSERT_NOT_NULL(link_task);

	/* Verify custom shell file is used instead of default */
	char *link_cmd_str = ape_cmd_render(&link_task->cmd);
	ASSERT(ape_str_contains(link_cmd_str, "--shell-file"));
	ASSERT(ape_str_contains(link_cmd_str, "custom_shell.html"));
	/* Should NOT contain the default shell path */
	ASSERT(!ape_str_contains(link_cmd_str, "ape_shell_default.html"));
	APEBUILD_FREE(link_cmd_str);

	ape_fs_rmdir("/tmp/ape_test_wasm_build2");
	ape_build_reset();
	return PASSED;
}

TEST(emcc_js_only_output) {
	ape_build_reset();
	ApeToolchainHandle tc = ape_toolchain_emcc();
	ApeBuilderHandle builder = ape_builder_new("myapp");
	ape_builder_set_type(builder, APE_TARGET_WASM);
	ape_builder_set_toolchain(builder, tc);
	ape_builder_set_output_dir(builder, "/tmp/ape_test_wasm_build3");
	ape_builder_set_wasm_output_mode(builder, 0); /* JS only, no shell */

	ape_builder_add_source(builder, "main.c");
	ape_builder_generate_tasks(builder);

	ApeBuilder *b = ape_builder_get(builder);
	ApeTask *link_task = ape_task_get(b->tasks.items[1]);
	ASSERT_NOT_NULL(link_task);

	/* Output should be .js */
	ASSERT(ape_str_ends_with(link_task->output, ".js"));

	/* Should NOT have --shell-file since wasm_html_output is 0 */
	char *link_cmd_str = ape_cmd_render(&link_task->cmd);
	ASSERT(!ape_str_contains(link_cmd_str, "--shell-file"));
	APEBUILD_FREE(link_cmd_str);

	ape_build_reset();
	return PASSED;
}

/* ============================================================================
 * Test Runner
 * ============================================================================ */

static void run_string_tests(void) {
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

static void run_sb_tests(void) {
	printf("String builder tests:\n");
	RUN_TEST(sb_basic);
	RUN_TEST(sb_fmt);
	RUN_TEST(sb_prepend);
	printf("\n");
}

static void run_sl_tests(void) {
	printf("String list tests:\n");
	RUN_TEST(sl_basic);
	RUN_TEST(sl_join);
	RUN_TEST(sl_contains);
	RUN_TEST(sl_remove);
	printf("\n");
}

static void run_fs_tests(void) {
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

static void run_cmd_tests(void) {
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

static void run_log_tests(void) {
	printf("Logging module tests:\n");
	RUN_TEST(log_level_name);
	RUN_TEST(log_level_from_name);
	RUN_TEST(log_set_level);
	printf("\n");
}

static void run_emcc_tests(void) {
	printf("Emscripten/WASM toolchain tests:\n");
	RUN_TEST(emcc_toolchain_creation);
	RUN_TEST(emcc_builder_wasm_output_path);
	RUN_TEST(emcc_builder_shell_file);
	RUN_TEST(emcc_builder_exported_functions);
	RUN_TEST(emcc_builder_preload_embed_files);
	RUN_TEST(emcc_builder_memory_settings);
	RUN_TEST(emcc_default_shell);
	RUN_TEST(emcc_task_generation);
	RUN_TEST(emcc_custom_shell_file);
	RUN_TEST(emcc_js_only_output);
	printf("\n");
}

int main(void) {
	ape_log_init();
	ape_log_set_level(APE_LOG_TRACE);
	ape_log_info("Apebuild Unit Tests");

	run_string_tests();
	run_sb_tests();
	run_sl_tests();
	run_fs_tests();
	run_cmd_tests();
	run_log_tests();
	run_emcc_tests();

	ape_log_info("Tests finished");
	ape_log_info("%d Total", tests_run);
	ape_log_info("%d \x1b[31mFAILED", tests_failed);
	ape_log_info("%d \x1b[32mPASSED", tests_passed);

	return tests_failed > 0 ? 1 : 0;
}
