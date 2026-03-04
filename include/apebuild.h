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

#ifndef APEBUILD_INCLUDED
#define APEBUILD_INCLUDED

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define APEBUILD_WINDOWS
#if defined(_WIN64)
#define APEBUILD_WINDOWS_X64
#endif
#elif defined(__linux__) || defined(__unix__)
#define APEBUILD_LINUX
#elif defined(__APPLE__)
#define APEBUILD_APPLE
#endif

#ifndef APEBUILD_MALLOC
#if defined(APEBUILD_REALLOC) || defined(APEBUILD_FREE)
#pragma GCC error "Need to define MALLOC, REALLOC and FREE or none of them"
#else
#include <stdlib.h>
#define APEBUILD_MALLOC malloc
#define APEBUILD_REALLOC realloc
#define APEBUILD_FREE free
#endif
#else
#if !defined(APEBUILD_REALLOC) || !defined(APEBUILD_FREE)
#pragma GCC error "Need to define MALLOC, REALLOC and FREE or none of them"
#endif
#endif

#ifndef APEBUILD_ASSERT
#ifdef APEBUILD_USE_STDLIB_ASSERT
#include <assert.h>
#define APEBUILD_ASSERT(c) assert(c)
#else
#include <stdio.h>
#include <stdlib.h>
#define APEBUILD_ASSERT(c)                                                                 \
	if (!(c)) {                                                                        \
		fprintf(stderr, "%s:%d Assertion '%s' failed\n", __FILE__, __LINE__, ##c); \
		exit(1);                                                                   \
	}
#endif
#endif

#if defined(__cplusplus)
extern "C" {
#endif

/* ============================================================================
 * Public Includes
 * ============================================================================ */

#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>
#include <time.h>

/* ============================================================================
 * Common Constants
 * ============================================================================ */

#ifndef APEBUILD_INIT_CAP
#define APEBUILD_INIT_CAP 256
#endif

/* ============================================================================
 * Generic Dynamic Array Macros
 * ============================================================================ */

#define ape_da_init(da)             \
	do {                        \
		(da)->capacity = 0; \
		(da)->count = 0;    \
		(da)->items = NULL; \
	} while (0)

#define ape_da_free(da)                     \
	do {                                \
		APEBUILD_FREE((da)->items); \
		ape_da_init(da);            \
	} while (0)

#define ape_da_append(da, item)                                                                             \
	do {                                                                                                \
		if ((da)->count >= (da)->capacity) {                                                        \
			(da)->capacity = (da)->capacity == 0 ? APEBUILD_INIT_CAP : (da)->capacity * 2;      \
			(da)->items = APEBUILD_REALLOC((da)->items, (da)->capacity * sizeof(*(da)->items)); \
		}                                                                                           \
		(da)->items[(da)->count++] = (item);                                                        \
	} while (0)

#define ape_da_append_many(da, new_items, n)                                                                \
	do {                                                                                                \
		if ((da)->count + (n) > (da)->capacity) {                                                   \
			if ((da)->capacity == 0) (da)->capacity = APEBUILD_INIT_CAP;                        \
			while ((da)->count + (n) > (da)->capacity) (da)->capacity *= 2;                     \
			(da)->items = APEBUILD_REALLOC((da)->items, (da)->capacity * sizeof(*(da)->items)); \
		}                                                                                           \
		for (size_t _i = 0; _i < (n); _i++) (da)->items[(da)->count++] = (new_items)[_i];           \
	} while (0)

#define ape_da_prepend(da, item)                                                                            \
	do {                                                                                                \
		if ((da)->count >= (da)->capacity) {                                                        \
			(da)->capacity = (da)->capacity == 0 ? APEBUILD_INIT_CAP : (da)->capacity * 2;      \
			(da)->items = APEBUILD_REALLOC((da)->items, (da)->capacity * sizeof(*(da)->items)); \
		}                                                                                           \
		for (size_t _i = (da)->count; _i > 0; _i--) (da)->items[_i] = (da)->items[_i - 1];          \
		(da)->items[0] = (item);                                                                    \
		(da)->count++;                                                                              \
	} while (0)

#define ape_da_insert(da, index, item)                                                                      \
	do {                                                                                                \
		if ((da)->count >= (da)->capacity) {                                                        \
			(da)->capacity = (da)->capacity == 0 ? APEBUILD_INIT_CAP : (da)->capacity * 2;      \
			(da)->items = APEBUILD_REALLOC((da)->items, (da)->capacity * sizeof(*(da)->items)); \
		}                                                                                           \
		for (size_t _i = (da)->count; _i > (index); _i--) (da)->items[_i] = (da)->items[_i - 1];    \
		(da)->items[index] = (item);                                                                \
		(da)->count++;                                                                              \
	} while (0)

#define ape_da_remove(da, index)                                                                             \
	do {                                                                                                 \
		for (size_t _i = (index); _i < (da)->count - 1; _i++) (da)->items[_i] = (da)->items[_i + 1]; \
		(da)->count--;                                                                               \
	} while (0)

#define ape_da_pop(da) ((da)->items[--(da)->count])
#define ape_da_get(da, index) ((da)->items[index])
#define ape_da_set(da, index, item) ((da)->items[index] = (item))
#define ape_da_len(da) ((da)->count)
#define ape_da_capacity(da) ((da)->capacity)
#define ape_da_clear(da) ((da)->count = 0)

#define ape_da_reserve(da, cap)                                                                             \
	do {                                                                                                \
		if ((cap) > (da)->capacity) {                                                               \
			(da)->capacity = (cap);                                                             \
			(da)->items = APEBUILD_REALLOC((da)->items, (da)->capacity * sizeof(*(da)->items)); \
		}                                                                                           \
	} while (0)

#define ape_da_shrink(da)                                                                                   \
	do {                                                                                                \
		if ((da)->count < (da)->capacity && (da)->count > 0) {                                      \
			(da)->capacity = (da)->count;                                                       \
			(da)->items = APEBUILD_REALLOC((da)->items, (da)->capacity * sizeof(*(da)->items)); \
		}                                                                                           \
	} while (0)

/* ============================================================================
 * String Module (ape_str)
 * ============================================================================ */

/* String Builder */
typedef struct {
	size_t capacity;
	size_t count;
	char *items;
} ApeStrBuilder;

void ape_sb_init(ApeStrBuilder *sb);
ApeStrBuilder ape_sb_new(void);
ApeStrBuilder ape_sb_new_cap(size_t cap);
void ape_sb_free(ApeStrBuilder *sb);
void ape_sb_clear(ApeStrBuilder *sb);
void ape_sb_append_char(ApeStrBuilder *sb, char c);
void ape_sb_append_str(ApeStrBuilder *sb, const char *str);
void ape_sb_append_strn(ApeStrBuilder *sb, const char *str, size_t n);
void ape_sb_append_sb(ApeStrBuilder *sb, const ApeStrBuilder *other);
void ape_sb_append_fmt(ApeStrBuilder *sb, const char *fmt, ...);
void ape_sb_append_fmtv(ApeStrBuilder *sb, const char *fmt, va_list args);
void ape_sb_prepend_str(ApeStrBuilder *sb, const char *str);
void ape_sb_insert(ApeStrBuilder *sb, size_t pos, const char *str);
const char *ape_sb_to_str(ApeStrBuilder *sb);
char *ape_sb_to_str_dup(const ApeStrBuilder *sb);
size_t ape_sb_len(const ApeStrBuilder *sb);
size_t ape_sb_capacity(const ApeStrBuilder *sb);
void ape_sb_reserve(ApeStrBuilder *sb, size_t cap);
void ape_sb_shrink(ApeStrBuilder *sb);

/* String List */
typedef struct {
	size_t capacity;
	size_t count;
	char **items;
} ApeStrList;

void ape_sl_init(ApeStrList *sl);
ApeStrList ape_sl_new(void);
void ape_sl_free(ApeStrList *sl);
void ape_sl_free_shallow(ApeStrList *sl);
void ape_sl_clear(ApeStrList *sl);
void ape_sl_append(ApeStrList *sl, char *str);
void ape_sl_append_dup(ApeStrList *sl, const char *str);
void ape_sl_append_many(ApeStrList *sl, char **strs, size_t n);
void ape_sl_prepend(ApeStrList *sl, char *str);
void ape_sl_insert(ApeStrList *sl, size_t index, char *str);
void ape_sl_remove(ApeStrList *sl, size_t index);
char *ape_sl_get(const ApeStrList *sl, size_t index);
size_t ape_sl_len(const ApeStrList *sl);
int ape_sl_contains(const ApeStrList *sl, const char *str);
int ape_sl_index_of(const ApeStrList *sl, const char *str);
char *ape_sl_join(const ApeStrList *sl, const char *sep);
ApeStrList ape_sl_clone(const ApeStrList *sl);

/* String Utilities */
char *ape_str_dup(const char *str);
char *ape_str_ndup(const char *str, size_t n);
char *ape_str_concat(const char *a, const char *b);
char *ape_str_join(const char **strs, size_t count, const char *sep);
ApeStrList ape_str_split(const char *str, const char *delim);
ApeStrList ape_str_split_lines(const char *str);

/* String Predicates */
int ape_str_eq(const char *a, const char *b);
int ape_str_eq_nocase(const char *a, const char *b);
int ape_str_starts_with(const char *str, const char *prefix);
int ape_str_ends_with(const char *str, const char *suffix);
int ape_str_contains(const char *str, const char *substr);
int ape_str_is_empty(const char *str);

/* String Transformation */
char *ape_str_trim(const char *str);
char *ape_str_trim_left(const char *str);
char *ape_str_trim_right(const char *str);
char *ape_str_to_lower(const char *str);
char *ape_str_to_upper(const char *str);
char *ape_str_replace(const char *str, const char *old, const char *new_str);
char *ape_str_replace_all(const char *str, const char *old, const char *new_str);
char *ape_str_substr(const char *str, size_t start, size_t len);

/* String Searching */
int ape_str_find(const char *str, const char *substr);
int ape_str_find_char(const char *str, char c);
int ape_str_rfind(const char *str, const char *substr);
int ape_str_rfind_char(const char *str, char c);

/* String Conversion */
int ape_str_to_int(const char *str, int *out);
int ape_str_to_long(const char *str, long *out);
int ape_str_to_float(const char *str, float *out);
int ape_str_to_double(const char *str, double *out);
char *ape_str_from_int(int val);
char *ape_str_from_long(long val);
char *ape_str_from_float(float val);

/* ============================================================================
 * Logging Module (ape_log)
 * ============================================================================ */

typedef enum { APE_LOG_TRACE, APE_LOG_DEBUG, APE_LOG_INFO, APE_LOG_WARN, APE_LOG_ERROR, APE_LOG_FATAL, APE_LOG_OFF } ApeLogLevel;

typedef struct {
	ApeLogLevel level;
	int use_colors;
	int show_timestamps;
	int show_level;
	int show_file;
	FILE *output;
	FILE *file_output;
	const char *prefix;
} ApeLogConfig;

void ape_log_init(void);
void ape_log_shutdown(void);
void ape_log_set_level(ApeLogLevel level);
ApeLogLevel ape_log_get_level(void);
void ape_log_set_output(FILE *fp);
int ape_log_set_file(const char *path);
void ape_log_set_colors(int enabled);
void ape_log_set_timestamps(int enabled);
void ape_log_set_show_level(int enabled);
void ape_log_set_show_file(int enabled);
void ape_log_set_prefix(const char *prefix);
void ape_log_set_quiet(int quiet);

void ape_log_write(ApeLogLevel level, const char *file, int line, const char *fmt, ...);
void ape_log_writev(ApeLogLevel level, const char *file, int line, const char *fmt, va_list args);

const char *ape_log_level_name(ApeLogLevel level);
int ape_log_level_from_name(const char *name, ApeLogLevel *out);
void ape_log_flush(void);

/* Build system specific logging */
void ape_log_cmd(const char *fmt, ...);
void ape_log_build(const char *fmt, ...);
void ape_log_link(const char *fmt, ...);
void ape_log_success(const char *fmt, ...);
void ape_log_failure(const char *fmt, ...);

/* Convenience macros */
#define ape_log_trace(...) ape_log_write(APE_LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define ape_log_debug(...) ape_log_write(APE_LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define ape_log_info(...) ape_log_write(APE_LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define ape_log_warn(...) ape_log_write(APE_LOG_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define ape_log_error(...) ape_log_write(APE_LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define ape_log_fatal(...) ape_log_write(APE_LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

/* ============================================================================
 * Filesystem Module (ape_fs)
 * ============================================================================ */

/* File stat structure */
typedef struct {
	size_t size;
	time_t mtime;
	time_t atime;
	time_t ctime;
	int is_dir;
	int is_file;
	int is_symlink;
	unsigned int mode;
} ApeFileStat;

/* Directory entry */
typedef struct {
	char *name;
	int is_dir;
	int is_file;
	int is_symlink;
} ApeDirEntry;

/* Directory handle */
typedef struct ApeDir ApeDir;

/* File reading/writing */
char *ape_fs_read_file(const char *path, size_t *out_size);
ApeStrList ape_fs_read_file_lines(const char *path);
int ape_fs_write_file(const char *path, const char *data, size_t size);
int ape_fs_append_file(const char *path, const char *data, size_t size);
int ape_fs_copy_file(const char *src, const char *dst);
int ape_fs_rename(const char *oldpath, const char *newpath);

/* Directory operations */
int ape_fs_mkdir(const char *path);
int ape_fs_mkdir_p(const char *path);
int ape_fs_rmdir(const char *path);
int ape_fs_rmdir_r(const char *path);
int ape_fs_exists(const char *path);
int ape_fs_is_file(const char *path);
int ape_fs_is_dir(const char *path);
int ape_fs_remove(const char *path);

/* Directory iteration */
ApeDir *ape_fs_opendir(const char *path);
ApeDirEntry *ape_fs_readdir(ApeDir *dir);
void ape_fs_closedir(ApeDir *dir);
void ape_fs_direntry_free(ApeDirEntry *entry);

typedef void (*ApeDirCallback)(const char *path, const ApeDirEntry *entry, void *userdata);
int ape_fs_iterdir(const char *path, ApeDirCallback callback, void *userdata);
int ape_fs_iterdir_r(const char *path, ApeDirCallback callback, void *userdata);
ApeStrList ape_fs_glob(const char *pattern);

/* File metadata */
int ape_fs_stat(const char *path, ApeFileStat *out);
time_t ape_fs_mtime(const char *path);
size_t ape_fs_size(const char *path);
int ape_fs_is_newer(const char *a, const char *b);
int ape_fs_needs_rebuild(const char *output, const char **inputs, size_t input_count);
int ape_fs_needs_rebuild1(const char *output, const char *input);

/* Path manipulation */
char *ape_fs_join(const char *a, const char *b);
char *ape_fs_dirname(const char *path);
char *ape_fs_basename(const char *path);
char *ape_fs_extension(const char *path);
char *ape_fs_stem(const char *path);
char *ape_fs_change_extension(const char *path, const char *new_ext);
char *ape_fs_normalize(const char *path);
char *ape_fs_absolute(const char *path);
char *ape_fs_relative(const char *from, const char *to);
int ape_fs_is_absolute(const char *path);

/* Temporary files */
const char *ape_fs_temp_dir(void);
char *ape_fs_temp_file(const char *prefix);
char *ape_fs_temp_mkdir(const char *prefix);

/* Utility */
char *ape_fs_cwd(void);
int ape_fs_chdir(const char *path);
char *ape_fs_home(void);

/* ============================================================================
 * Command Execution Module (ape_cmd)
 * ============================================================================ */

/* Process status */
typedef enum { APE_PROC_RUNNING, APE_PROC_COMPLETED, APE_PROC_FAILED, APE_PROC_SIGNALED, APE_PROC_UNKNOWN } ApeProcStatus;

/* Process result */
typedef struct {
	ApeProcStatus status;
	int exit_code;
	int signal;
} ApeProcResult;

/* Process handle (opaque integer, not pointer) */
typedef int32_t ApeProcHandle;
#define APE_INVALID_HANDLE ((ApeProcHandle) - 1)

/* Command structure */
typedef struct {
	size_t capacity;
	size_t count;
	const char **items;
	char *cwd;
	ApeStrList env;
} ApeCmd;

/* Command list */
typedef struct {
	size_t capacity;
	size_t count;
	ApeCmd *items;
} ApeCmdList;

/* Command building */
void ape_cmd_init(ApeCmd *cmd);
ApeCmd ape_cmd_new(void);
ApeCmd ape_cmd_from(const char *program);
void ape_cmd_append(ApeCmd *cmd, const char *arg);
void ape_cmd_append_many(ApeCmd *cmd, const char **args, size_t count);
void ape_cmd_prepend(ApeCmd *cmd, const char *arg);
ApeCmd ape_cmd_clone(const ApeCmd *cmd);
void ape_cmd_free(ApeCmd *cmd);
void ape_cmd_clear(ApeCmd *cmd);

/* Command display */
char *ape_cmd_render(const ApeCmd *cmd);
char *ape_cmd_render_quoted(const ApeCmd *cmd);
void ape_cmd_print(const ApeCmd *cmd);
void ape_cmd_log(const ApeCmd *cmd, const char *prefix);

/* Synchronous execution */
int ape_cmd_run(ApeCmd *cmd);
int ape_cmd_run_status(ApeCmd *cmd);
char *ape_cmd_run_capture(ApeCmd *cmd, int *exit_code);
int ape_cmds_run_seq(ApeCmdList *cmds);
int ape_cmds_run_all(ApeCmdList *cmds);

/* Asynchronous execution */
ApeProcHandle ape_cmd_start(ApeCmd *cmd);
int ape_proc_poll(ApeProcHandle handle);
int ape_proc_wait(ApeProcHandle handle);
int ape_proc_wait_timeout(ApeProcHandle handle, int timeout_ms);
ApeProcStatus ape_proc_status(ApeProcHandle handle);
ApeProcResult ape_proc_result(ApeProcHandle handle);
int ape_proc_kill(ApeProcHandle handle);
int ape_proc_handle_valid(ApeProcHandle handle);
void ape_proc_handle_release(ApeProcHandle handle);

/* Parallel execution */
typedef struct ApeProcPool ApeProcPool;
ApeProcPool *ape_pool_new(int max_parallel);
int ape_pool_submit(ApeProcPool *pool, ApeCmd *cmd);
ApeProcHandle ape_pool_wait_any(ApeProcPool *pool);
int ape_pool_wait_all(ApeProcPool *pool);
void ape_pool_free(ApeProcPool *pool);
int ape_cmds_run_parallel(ApeCmdList *cmds, int max_parallel);

/* Environment */
void ape_cmd_set_env(ApeCmd *cmd, const char *name, const char *value);
void ape_cmd_clear_env(ApeCmd *cmd);
void ape_cmd_set_cwd(ApeCmd *cmd, const char *cwd);

/* Command list operations */
void ape_cmdlist_init(ApeCmdList *list);
ApeCmdList ape_cmdlist_new(void);
void ape_cmdlist_append(ApeCmdList *list, ApeCmd cmd);
void ape_cmdlist_free(ApeCmdList *list);

/* Variadic command append helper */
#define ape_cmd_append_args(cmd, ...)                                              \
	do {                                                                       \
		const char *_args[] = { __VA_ARGS__ };                             \
		ape_cmd_append_many(cmd, _args, sizeof(_args) / sizeof(_args[0])); \
	} while (0)

/* ============================================================================
 * Core Build Module (ape_build)
 * ============================================================================ */

/* ----------------------------------------------------------------------------
 * Handle Types and Storage Configuration
 *
 * All build objects (toolchains, builders, tasks) are stored in fixed-size
 * internal arrays and referenced by handles (integer IDs) rather than pointers.
 * This simplifies memory management and avoids dangling pointer issues.
 *
 * Users can override the maximum counts by defining these macros before
 * including this header.
 * ---------------------------------------------------------------------------- */

#ifndef APE_MAX_TOOLCHAINS
#define APE_MAX_TOOLCHAINS 16
#endif

#ifndef APE_MAX_BUILDERS
#define APE_MAX_BUILDERS 64
#endif

#ifndef APE_MAX_TASKS
#define APE_MAX_TASKS 1024
#endif

#ifndef APE_MAX_TASK_DEPS
#define APE_MAX_TASK_DEPS 64
#endif

#ifndef APE_MAX_BUILDER_DEPS
#define APE_MAX_BUILDER_DEPS 32
#endif

/* Handle types - all are int32_t indices into internal arrays */
typedef int32_t ApeToolchainHandle;
typedef int32_t ApeBuilderHandle;
typedef int32_t ApeTaskHandle;

#define APE_INVALID_TOOLCHAIN ((ApeToolchainHandle) - 1)
#define APE_INVALID_BUILDER ((ApeBuilderHandle) - 1)
#define APE_INVALID_TASK ((ApeTaskHandle) - 1)

/* Handle list types for dependencies */
typedef struct {
	size_t count;
	ApeTaskHandle items[APE_MAX_TASK_DEPS];
} ApeTaskDepList;

typedef struct {
	size_t count;
	ApeBuilderHandle items[APE_MAX_BUILDER_DEPS];
} ApeBuilderDepList;

typedef struct {
	size_t count;
	ApeTaskHandle items[APE_MAX_TASKS];
} ApeTaskHandleList;

typedef struct {
	size_t count;
	ApeBuilderHandle items[APE_MAX_BUILDERS];
} ApeBuilderHandleList;

/* Verbosity levels */
typedef enum {
	APE_VERBOSE_QUIET,   /* No output except errors */
	APE_VERBOSE_NORMAL,  /* Normal build output */
	APE_VERBOSE_VERBOSE, /* Show commands being run */
	APE_VERBOSE_DEBUG    /* Show all details */
} ApeVerbosity;

/* Build target types */
typedef enum {
	APE_TARGET_EXECUTABLE,
	APE_TARGET_STATIC_LIB,
	APE_TARGET_SHARED_LIB,
	APE_TARGET_OBJECT, /* Compile only, no link */
	APE_TARGET_WASM	   /* Emscripten WebAssembly output (.html or .js) */
} ApeTargetType;

/* Task types */
typedef enum {
	APE_TASK_TYPE_COMPILE, /* Compile source to object */
	APE_TASK_TYPE_LINK,    /* Link objects to target */
	APE_TASK_TYPE_ARCHIVE, /* Create static library */
	APE_TASK_TYPE_COMMAND, /* Run arbitrary command */
	APE_TASK_TYPE_COPY,    /* Copy file */
	APE_TASK_TYPE_MKDIR    /* Create directory */
} ApeTaskType;

/* Task status */
typedef enum {
	APE_TASK_PENDING,
	APE_TASK_RUNNING,
	APE_TASK_COMPLETED,
	APE_TASK_FAILED,
	APE_TASK_SKIPPED /* Already up-to-date */
} ApeTaskStatus;

/* ----------------------------------------------------------------------------
 * Toolchain - Compiler/linker configuration
 * ---------------------------------------------------------------------------- */

typedef struct {
	int in_use;
	char *name;	      /* Toolchain name (e.g., "gcc", "clang") */
	char *cc;	      /* C compiler command */
	char *cxx;	      /* C++ compiler command */
	char *ld;	      /* Linker command */
	char *ar;	      /* Archiver command */
	char *obj_ext;	      /* Object file extension (e.g., ".o") */
	char *exe_ext;	      /* Executable extension (e.g., "", ".exe") */
	char *static_lib_ext; /* Static lib extension (e.g., ".a") */
	char *shared_lib_ext; /* Shared lib extension (e.g., ".so") */
	char *lib_prefix;     /* Library prefix (e.g., "lib") */
	ApeStrList default_cflags;
	ApeStrList default_ldflags;
} ApeToolchain;

/* Toolchain management */
ApeToolchainHandle ape_toolchain_new(const char *name);
void ape_toolchain_free(ApeToolchainHandle handle);
ApeToolchainHandle ape_toolchain_clone(ApeToolchainHandle handle);
ApeToolchainHandle ape_toolchain_gcc(void);
ApeToolchainHandle ape_toolchain_clang(void);
ApeToolchainHandle ape_toolchain_emcc(void);
ApeToolchain *ape_toolchain_get(ApeToolchainHandle handle);
int ape_toolchain_valid(ApeToolchainHandle handle);

/* Toolchain configuration */
void ape_toolchain_set_cc(ApeToolchainHandle handle, const char *cc);
void ape_toolchain_set_cxx(ApeToolchainHandle handle, const char *cxx);
void ape_toolchain_set_ld(ApeToolchainHandle handle, const char *ld);
void ape_toolchain_set_ar(ApeToolchainHandle handle, const char *ar);
void ape_toolchain_add_cflag(ApeToolchainHandle handle, const char *flag);
void ape_toolchain_add_ldflag(ApeToolchainHandle handle, const char *flag);

/* ----------------------------------------------------------------------------
 * Task - Individual build operation
 * ---------------------------------------------------------------------------- */

typedef struct {
	int in_use;
	ApeTaskType type;
	ApeTaskStatus status;
	char *name;		  /* Human-readable name */
	char *input;		  /* Primary input file (for compile) */
	char *output;		  /* Output file */
	ApeStrList inputs;	  /* Additional inputs (for link) */
	ApeCmd cmd;		  /* Command to execute */
	ApeTaskDepList deps;	  /* Task handles that must complete before this one */
	ApeProcHandle proc;	  /* Process handle when running */
	int exit_code;		  /* Exit code after completion */
	ApeBuilderHandle builder; /* Parent builder handle */
} ApeTask;

/* Task management */
ApeTaskHandle ape_task_new(ApeBuilderHandle builder, ApeTaskType type, const char *name);
void ape_task_free(ApeTaskHandle handle);
ApeTask *ape_task_get(ApeTaskHandle handle);
int ape_task_valid(ApeTaskHandle handle);

/* Task configuration */
void ape_task_set_input(ApeTaskHandle handle, const char *input);
void ape_task_set_output(ApeTaskHandle handle, const char *output);
void ape_task_add_input(ApeTaskHandle handle, const char *input);
void ape_task_add_dep(ApeTaskHandle handle, ApeTaskHandle dep);
void ape_task_set_cmd(ApeTaskHandle handle, ApeCmd cmd);
int ape_task_needs_rebuild(ApeTaskHandle handle);
int ape_task_ready(ApeTaskHandle handle); /* All deps completed? */

/* ----------------------------------------------------------------------------
 * Builder - Build target with sources, flags, and tasks
 * ---------------------------------------------------------------------------- */

typedef struct {
	int in_use;
	char *name; /* Target name */
	ApeTargetType type;
	ApeToolchainHandle toolchain; /* Can override context's toolchain */

	/* Source files */
	ApeStrList sources;

	/* Compiler flags (per-builder) */
	ApeStrList cflags;
	ApeStrList include_dirs;
	ApeStrList defines;

	/* Linker flags (per-builder) */
	ApeStrList ldflags;
	ApeStrList lib_dirs;
	ApeStrList libs;

	/* Output configuration */
	char *output_dir;
	char *output_name; /* Override default output name */

	/* Emscripten/WASM configuration */
	char *shell_file;	       /* Path to custom shell HTML file (NULL = use default) */
	int wasm_html_output;	       /* 1 = .html output (default), 0 = .js only */
	ApeStrList exported_functions; /* Functions to export (e.g., "_main") */
	ApeStrList preload_files;      /* Files to preload with --preload-file */
	ApeStrList embed_files;	       /* Files to embed with --embed-file */
	int wasm_initial_memory;       /* Initial memory in MB (0 = emcc default) */
	int wasm_max_memory;	       /* Max memory in MB (0 = emcc default) */

	/* Dependencies */
	ApeBuilderDepList deps; /* Other builder handles to build first */

	/* Tasks (generated during build) */
	ApeTaskHandleList tasks;

	/* State */
	int built; /* Has been built this session */
	int build_failed;
} ApeBuilder;

/* Builder management */
ApeBuilderHandle ape_builder_new(const char *name);
void ape_builder_free(ApeBuilderHandle handle);
ApeBuilder *ape_builder_get(ApeBuilderHandle handle);
int ape_builder_valid(ApeBuilderHandle handle);

/* Builder configuration */
void ape_builder_set_type(ApeBuilderHandle handle, ApeTargetType type);
void ape_builder_set_toolchain(ApeBuilderHandle handle, ApeToolchainHandle tc);
void ape_builder_set_output_dir(ApeBuilderHandle handle, const char *dir);
void ape_builder_set_output_name(ApeBuilderHandle handle, const char *name);

/* Source management */
void ape_builder_add_source(ApeBuilderHandle handle, const char *path);
void ape_builder_add_sources(ApeBuilderHandle handle, const char **paths, size_t count);
void ape_builder_add_source_dir(ApeBuilderHandle handle, const char *dir);
void ape_builder_add_source_dir_r(ApeBuilderHandle handle, const char *dir);
void ape_builder_add_source_glob(ApeBuilderHandle handle, const char *pattern);

/* Compiler flags */
void ape_builder_add_cflag(ApeBuilderHandle handle, const char *flag);
void ape_builder_add_include(ApeBuilderHandle handle, const char *dir);
void ape_builder_add_define(ApeBuilderHandle handle, const char *define);
void ape_builder_add_define_value(ApeBuilderHandle handle, const char *name, const char *value);

/* Linker flags */
void ape_builder_add_ldflag(ApeBuilderHandle handle, const char *flag);
void ape_builder_add_lib_dir(ApeBuilderHandle handle, const char *dir);
void ape_builder_add_lib(ApeBuilderHandle handle, const char *lib);

/* Dependencies */
void ape_builder_depends_on(ApeBuilderHandle handle, ApeBuilderHandle dep);
void ape_builder_link_with(ApeBuilderHandle handle, ApeBuilderHandle lib_builder);

/* Build operations */
int ape_builder_build(ApeBuilderHandle handle);
int ape_builder_clean(ApeBuilderHandle handle);
int ape_builder_rebuild(ApeBuilderHandle handle);
char *ape_builder_output_path(ApeBuilderHandle handle);

/* Emscripten/WASM-specific builder configuration */
void ape_builder_set_shell_file(ApeBuilderHandle handle, const char *path);
void ape_builder_set_wasm_output_mode(ApeBuilderHandle handle, int html);
void ape_builder_add_exported_function(ApeBuilderHandle handle, const char *func);
void ape_builder_add_preload_file(ApeBuilderHandle handle, const char *path);
void ape_builder_add_embed_file(ApeBuilderHandle handle, const char *path);
void ape_builder_set_wasm_memory(ApeBuilderHandle handle, int initial_mb, int max_mb);

/* Get default Emscripten shell HTML (embedded in source, returned as static string) */
const char *ape_emcc_default_shell(void);

/* Task generation (internal, but exposed for flexibility) */
ApeTaskHandle ape_builder_add_compile_task(ApeBuilderHandle handle, const char *source);
ApeTaskHandle ape_builder_add_link_task(ApeBuilderHandle handle);
ApeTaskHandle ape_builder_add_archive_task(ApeBuilderHandle handle);
ApeTaskHandle ape_builder_add_command_task(ApeBuilderHandle handle, const char *name, ApeCmd cmd);
void ape_builder_generate_tasks(ApeBuilderHandle handle);

/* ----------------------------------------------------------------------------
 * Build Context - Global configuration
 *
 * The build context is now a simple configuration struct. Builders and
 * toolchains are stored in global arrays, not inside the context.
 * ---------------------------------------------------------------------------- */

typedef struct {
	ApeToolchainHandle toolchain; /* Default toolchain */
	char *output_dir;	      /* Default output directory */
	int parallel_jobs;	      /* Max parallel tasks (0 = auto) */
	ApeVerbosity verbosity;
	int force_rebuild; /* Ignore timestamps */
	int dry_run;	   /* Don't actually run commands */
	int keep_going;	   /* Continue on errors */
} ApeBuildCtx;

/* Global context - there's one active context at a time */
void ape_ctx_init(ApeBuildCtx *ctx);
void ape_ctx_cleanup(ApeBuildCtx *ctx);
void ape_ctx_set_toolchain(ApeBuildCtx *ctx, ApeToolchainHandle tc);
void ape_ctx_set_output_dir(ApeBuildCtx *ctx, const char *dir);
void ape_ctx_set_parallel(ApeBuildCtx *ctx, int jobs);
void ape_ctx_set_verbosity(ApeBuildCtx *ctx, ApeVerbosity level);
void ape_ctx_set_force_rebuild(ApeBuildCtx *ctx, int force);
void ape_ctx_set_dry_run(ApeBuildCtx *ctx, int dry_run);
void ape_ctx_set_keep_going(ApeBuildCtx *ctx, int keep_going);
ApeToolchainHandle ape_ctx_get_toolchain(ApeBuildCtx *ctx);

/* Build operations using context */
int ape_ctx_build(ApeBuildCtx *ctx, ApeBuilderHandle builder);
int ape_ctx_clean(ApeBuildCtx *ctx, ApeBuilderHandle builder);
int ape_ctx_rebuild(ApeBuildCtx *ctx, ApeBuilderHandle builder);

/* ----------------------------------------------------------------------------
 * Global Storage Management
 *
 * Functions to manage the global storage arrays. Call ape_build_init() at
 * program start and ape_build_shutdown() at program end.
 * ---------------------------------------------------------------------------- */

void ape_build_init(void);
void ape_build_shutdown(void);
void ape_build_reset(void); /* Free all objects and reset storage */

/* Find builder by name */
ApeBuilderHandle ape_builder_find(const char *name);

/* ----------------------------------------------------------------------------
 * Utility functions
 * ---------------------------------------------------------------------------- */

char *ape_build_obj_path(ApeBuildCtx *ctx, ApeBuilderHandle builder, const char *source);
char *ape_build_output_path(ApeBuildCtx *ctx, ApeBuilderHandle builder);
int ape_build_get_cpu_count(void);

/* ----------------------------------------------------------------------------
 * Auto-rebuild support
 * ---------------------------------------------------------------------------- */

int ape_self_needs_rebuild(const char *binary, const char *source);
int ape_self_rebuild(int argc, char **argv, const char *source);

#define APE_REBUILD(argc, argv)                                                                                         \
	do {                                                                                                            \
		if (ape_self_needs_rebuild((argv)[0], __FILE__)) { return ape_self_rebuild((argc), (argv), __FILE__); } \
	} while (0)

#if defined(__cplusplus)
}
#endif

#if defined(APEBUILD_STRIP_PREFIX)

#endif

#endif

#ifdef APEBUILD_IMPLEMENTATION

#ifndef APEBUILD_IMPLEMENTATION_INCLUDED
#define APEBUILD_IMPLEMENTATION_INCLUDED

/* User can define a custom function prefix (eg. static) */
#ifndef APEBUILD_DEF
#define APEBUILD_DEF
#endif

/* Can also define custom private function prefix */
#ifndef APEBUILD_PRIVATE
#define APEBUILD_PRIVATE static
#endif

#ifndef APEBUILD_TRUE
#define APEBUILD_TRUE (1)
#define APEBUILD_FALSE (0)
#else
#if !defined(APEBUILD_FALSE)
#pragma GCC error "Need to define both APEBUILD_TRUE and APEBUILD_FALSE or neither"
#endif
#endif

#include <string.h>
#include <sys/stat.h>
#include <stdio.h>
#include <dirent.h>
#include <time.h>
#include <signal.h>
#include <fnmatch.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>

/* BEGIN ape_build.c */
/*
 * ape_build.c - Core build module implementation
 *
 * Uses handle-based storage for all build objects (toolchains, builders, tasks)
 * instead of pointers. Objects are stored in fixed-size global arrays.
 */

/* ============================================================================
 * Embedded Default Emscripten Shell HTML
 *
 * This is a minimal shell file for Emscripten WASM output. It provides:
 * - A canvas element for graphical applications
 * - A text area for stdout/stderr output
 * - Basic module loading and error handling
 * - Responsive layout
 *
 * Users can override this by calling ape_builder_set_shell_file() with
 * a path to their own shell HTML file.
 *
 * The {{{ SCRIPT }}} placeholder is required by emcc and gets replaced
 * with the generated JavaScript glue code at link time.
 * ============================================================================ */

APEBUILD_PRIVATE const char ape_emcc_default_shell_html[] = "<!doctype html>\n"
							    "<html lang=\"en-us\">\n"
							    "<head>\n"
							    "  <meta charset=\"utf-8\">\n"
							    "  <meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n"
							    "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
							    "  <title>Emscripten Application</title>\n"
							    "  <style>\n"
							    "    * { margin: 0; padding: 0; box-sizing: border-box; }\n"
							    "    body {\n"
							    "      font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, "
							    "sans-serif;\n"
							    "      background: #1a1a2e; color: #e0e0e0;\n"
							    "      display: flex; flex-direction: column; align-items: center;\n"
							    "      min-height: 100vh; padding: 20px;\n"
							    "    }\n"
							    "    h1 { margin-bottom: 16px; font-size: 1.4rem; color: #c0c0c0; }\n"
							    "    #canvas-container {\n"
							    "      border: 1px solid #333; border-radius: 4px;\n"
							    "      overflow: hidden; margin-bottom: 16px;\n"
							    "      background: #000;\n"
							    "    }\n"
							    "    canvas.emscripten { display: block; }\n"
							    "    #output-container {\n"
							    "      width: 100%%; max-width: 800px;\n"
							    "    }\n"
							    "    #output {\n"
							    "      width: 100%%; height: 200px;\n"
							    "      font-family: 'Consolas', 'Monaco', monospace;\n"
							    "      font-size: 13px; line-height: 1.4;\n"
							    "      background: #0d0d1a; color: #a0ffa0;\n"
							    "      border: 1px solid #333; border-radius: 4px;\n"
							    "      padding: 8px; resize: vertical;\n"
							    "    }\n"
							    "    #status {\n"
							    "      margin-top: 8px; font-size: 0.9rem;\n"
							    "      color: #888; text-align: center;\n"
							    "    }\n"
							    "    .spinner {\n"
							    "      display: inline-block; width: 16px; height: 16px;\n"
							    "      border: 2px solid #444; border-top-color: #a0ffa0;\n"
							    "      border-radius: 50%%;\n"
							    "      animation: spin 0.8s linear infinite;\n"
							    "      vertical-align: middle; margin-right: 6px;\n"
							    "    }\n"
							    "    @keyframes spin { to { transform: rotate(360deg); } }\n"
							    "  </style>\n"
							    "</head>\n"
							    "<body>\n"
							    "  <h1>Emscripten Application</h1>\n"
							    "  <div id=\"canvas-container\">\n"
							    "    <canvas class=\"emscripten\" id=\"canvas\"\n"
							    "            oncontextmenu=\"event.preventDefault()\"\n"
							    "            tabindex=\"-1\" width=\"800\" height=\"600\"></canvas>\n"
							    "  </div>\n"
							    "  <div id=\"output-container\">\n"
							    "    <textarea id=\"output\" rows=\"8\" readonly></textarea>\n"
							    "  </div>\n"
							    "  <div id=\"status\"><span class=\"spinner\"></span>Loading...</div>\n"
							    "  <script type='text/javascript'>\n"
							    "    var statusElement = document.getElementById('status');\n"
							    "    var outputElement = document.getElementById('output');\n"
							    "    var Module = {\n"
							    "      print: (function() {\n"
							    "        return function(text) {\n"
							    "          if (arguments.length > 1)\n"
							    "            text = Array.prototype.slice.call(arguments).join(' ');\n"
							    "          console.log(text);\n"
							    "          if (outputElement) {\n"
							    "            outputElement.value += text + '\\n';\n"
							    "            outputElement.scrollTop = outputElement.scrollHeight;\n"
							    "          }\n"
							    "        };\n"
							    "      })(),\n"
							    "      printErr: function(text) {\n"
							    "        if (arguments.length > 1)\n"
							    "          text = Array.prototype.slice.call(arguments).join(' ');\n"
							    "        console.error(text);\n"
							    "        if (outputElement) {\n"
							    "          outputElement.value += 'ERR: ' + text + '\\n';\n"
							    "          outputElement.scrollTop = outputElement.scrollHeight;\n"
							    "        }\n"
							    "      },\n"
							    "      canvas: (function() { return document.getElementById('canvas'); })(),\n"
							    "      setStatus: function(text) {\n"
							    "        if (statusElement) {\n"
							    "          if (text) statusElement.innerHTML = '<span "
							    "class=\"spinner\"></span>' + text;\n"
							    "          else statusElement.innerHTML = 'Ready.';\n"
							    "        }\n"
							    "      },\n"
							    "      onRuntimeInitialized: function() {\n"
							    "        if (statusElement) statusElement.innerHTML = 'Ready.';\n"
							    "      },\n"
							    "      totalDependencies: 0,\n"
							    "      monitorRunDependencies: function(left) {\n"
							    "        this.totalDependencies = Math.max(this.totalDependencies, left);\n"
							    "        Module.setStatus(left\n"
							    "          ? 'Preparing... (' + (this.totalDependencies-left) + '/' + "
							    "this.totalDependencies + ')'\n"
							    "          : '');\n"
							    "      }\n"
							    "    };\n"
							    "    Module.setStatus('Downloading...');\n"
							    "    window.onerror = function() {\n"
							    "      Module.setStatus('Error occurred. See console.');\n"
							    "    };\n"
							    "  </script>\n"
							    "  {{{ SCRIPT }}}\n"
							    "</body>\n"
							    "</html>\n";

/* ============================================================================
 * Global Storage Arrays
 * ============================================================================ */

APEBUILD_PRIVATE ApeToolchain ape_toolchain_storage[APE_MAX_TOOLCHAINS];
APEBUILD_PRIVATE ApeBuilder ape_builder_storage[APE_MAX_BUILDERS];
APEBUILD_PRIVATE ApeTask ape_task_storage[APE_MAX_TASKS];
APEBUILD_PRIVATE int ape_build_initialized = 0;

/* ============================================================================
 * Initialization and Shutdown
 * ============================================================================ */

APEBUILD_DEF void ape_build_init(void) {
	if (ape_build_initialized) return;

	memset(ape_toolchain_storage, 0, sizeof(ape_toolchain_storage));
	memset(ape_builder_storage, 0, sizeof(ape_builder_storage));
	memset(ape_task_storage, 0, sizeof(ape_task_storage));

	ape_build_initialized = 1;
}

APEBUILD_PRIVATE void ape_toolchain_clear(ApeToolchain *tc) {
	APEBUILD_FREE(tc->name);
	APEBUILD_FREE(tc->cc);
	APEBUILD_FREE(tc->cxx);
	APEBUILD_FREE(tc->ld);
	APEBUILD_FREE(tc->ar);
	APEBUILD_FREE(tc->obj_ext);
	APEBUILD_FREE(tc->exe_ext);
	APEBUILD_FREE(tc->static_lib_ext);
	APEBUILD_FREE(tc->shared_lib_ext);
	APEBUILD_FREE(tc->lib_prefix);
	ape_sl_free(&tc->default_cflags);
	ape_sl_free(&tc->default_ldflags);
	memset(tc, 0, sizeof(ApeToolchain));
}

APEBUILD_PRIVATE void ape_task_clear(ApeTask *task) {
	APEBUILD_FREE(task->name);
	APEBUILD_FREE(task->input);
	APEBUILD_FREE(task->output);
	ape_sl_free(&task->inputs);
	ape_cmd_free(&task->cmd);
	memset(task, 0, sizeof(ApeTask));
}

APEBUILD_PRIVATE void ape_builder_clear(ApeBuilder *builder) {
	APEBUILD_FREE(builder->name);
	APEBUILD_FREE(builder->output_dir);
	APEBUILD_FREE(builder->output_name);
	APEBUILD_FREE(builder->shell_file);
	ape_sl_free(&builder->sources);
	ape_sl_free(&builder->cflags);
	ape_sl_free(&builder->include_dirs);
	ape_sl_free(&builder->defines);
	ape_sl_free(&builder->ldflags);
	ape_sl_free(&builder->lib_dirs);
	ape_sl_free(&builder->libs);
	ape_sl_free(&builder->exported_functions);
	ape_sl_free(&builder->preload_files);
	ape_sl_free(&builder->embed_files);
	memset(builder, 0, sizeof(ApeBuilder));
}

APEBUILD_DEF void ape_build_shutdown(void) {
	if (!ape_build_initialized) return;

	/* Free all toolchains */
	for (int i = 0; i < APE_MAX_TOOLCHAINS; i++) {
		if (ape_toolchain_storage[i].in_use) { ape_toolchain_clear(&ape_toolchain_storage[i]); }
	}

	/* Free all tasks */
	for (int i = 0; i < APE_MAX_TASKS; i++) {
		if (ape_task_storage[i].in_use) { ape_task_clear(&ape_task_storage[i]); }
	}

	/* Free all builders */
	for (int i = 0; i < APE_MAX_BUILDERS; i++) {
		if (ape_builder_storage[i].in_use) { ape_builder_clear(&ape_builder_storage[i]); }
	}

	ape_build_initialized = 0;
}

APEBUILD_DEF void ape_build_reset(void) {
	ape_build_shutdown();
	ape_build_init();
}

/* ============================================================================
 * Toolchain Implementation
 * ============================================================================ */

APEBUILD_DEF ApeToolchainHandle ape_toolchain_new(const char *name) {
	ape_build_init();

	for (int i = 0; i < APE_MAX_TOOLCHAINS; i++) {
		if (!ape_toolchain_storage[i].in_use) {
			ApeToolchain *tc = &ape_toolchain_storage[i];
			memset(tc, 0, sizeof(ApeToolchain));

			tc->in_use = 1;
			tc->name = ape_str_dup(name);
			tc->cc = ape_str_dup("cc");
			tc->cxx = ape_str_dup("c++");
			tc->ld = ape_str_dup("cc");
			tc->ar = ape_str_dup("ar");
			tc->obj_ext = ape_str_dup(".o");
			tc->exe_ext = ape_str_dup("");
			tc->static_lib_ext = ape_str_dup(".a");
			tc->shared_lib_ext = ape_str_dup(".so");
			tc->lib_prefix = ape_str_dup("lib");
			ape_sl_init(&tc->default_cflags);
			ape_sl_init(&tc->default_ldflags);

			return (ApeToolchainHandle)i;
		}
	}
	return APE_INVALID_TOOLCHAIN;
}

APEBUILD_DEF void ape_toolchain_free(ApeToolchainHandle handle) {
	ApeToolchain *tc = ape_toolchain_get(handle);
	if (tc) { ape_toolchain_clear(tc); }
}

APEBUILD_DEF ApeToolchain *ape_toolchain_get(ApeToolchainHandle handle) {
	if (handle < 0 || handle >= APE_MAX_TOOLCHAINS) return NULL;
	if (!ape_toolchain_storage[handle].in_use) return NULL;
	return &ape_toolchain_storage[handle];
}

APEBUILD_DEF int ape_toolchain_valid(ApeToolchainHandle handle) { return ape_toolchain_get(handle) != NULL; }

APEBUILD_DEF ApeToolchainHandle ape_toolchain_clone(ApeToolchainHandle handle) {
	ApeToolchain *src = ape_toolchain_get(handle);
	if (!src) return APE_INVALID_TOOLCHAIN;

	ApeToolchainHandle new_handle = ape_toolchain_new(src->name);
	ApeToolchain *tc = ape_toolchain_get(new_handle);
	if (!tc) return APE_INVALID_TOOLCHAIN;

	APEBUILD_FREE(tc->cc);
	APEBUILD_FREE(tc->cxx);
	APEBUILD_FREE(tc->ld);
	APEBUILD_FREE(tc->ar);
	APEBUILD_FREE(tc->obj_ext);
	APEBUILD_FREE(tc->exe_ext);
	APEBUILD_FREE(tc->static_lib_ext);
	APEBUILD_FREE(tc->shared_lib_ext);
	APEBUILD_FREE(tc->lib_prefix);

	tc->cc = ape_str_dup(src->cc);
	tc->cxx = ape_str_dup(src->cxx);
	tc->ld = ape_str_dup(src->ld);
	tc->ar = ape_str_dup(src->ar);
	tc->obj_ext = ape_str_dup(src->obj_ext);
	tc->exe_ext = ape_str_dup(src->exe_ext);
	tc->static_lib_ext = ape_str_dup(src->static_lib_ext);
	tc->shared_lib_ext = ape_str_dup(src->shared_lib_ext);
	tc->lib_prefix = ape_str_dup(src->lib_prefix);
	tc->default_cflags = ape_sl_clone(&src->default_cflags);
	tc->default_ldflags = ape_sl_clone(&src->default_ldflags);

	return new_handle;
}

APEBUILD_DEF ApeToolchainHandle ape_toolchain_gcc(void) {
	ApeToolchainHandle handle = ape_toolchain_new("gcc");
	ApeToolchain *tc = ape_toolchain_get(handle);
	if (!tc) return APE_INVALID_TOOLCHAIN;

	APEBUILD_FREE(tc->cc);
	APEBUILD_FREE(tc->cxx);
	APEBUILD_FREE(tc->ld);
	tc->cc = ape_str_dup("gcc");
	tc->cxx = ape_str_dup("g++");
	tc->ld = ape_str_dup("gcc");

	return handle;
}

APEBUILD_DEF ApeToolchainHandle ape_toolchain_clang(void) {
	ApeToolchainHandle handle = ape_toolchain_new("clang");
	ApeToolchain *tc = ape_toolchain_get(handle);
	if (!tc) return APE_INVALID_TOOLCHAIN;

	APEBUILD_FREE(tc->cc);
	APEBUILD_FREE(tc->cxx);
	APEBUILD_FREE(tc->ld);
	tc->cc = ape_str_dup("clang");
	tc->cxx = ape_str_dup("clang++");
	tc->ld = ape_str_dup("clang");

	return handle;
}

APEBUILD_DEF ApeToolchainHandle ape_toolchain_emcc(void) {
	ApeToolchainHandle handle = ape_toolchain_new("emcc");
	ApeToolchain *tc = ape_toolchain_get(handle);
	if (!tc) return APE_INVALID_TOOLCHAIN;

	APEBUILD_FREE(tc->cc);
	APEBUILD_FREE(tc->cxx);
	APEBUILD_FREE(tc->ld);
	APEBUILD_FREE(tc->ar);
	APEBUILD_FREE(tc->exe_ext);
	APEBUILD_FREE(tc->shared_lib_ext);
	tc->cc = ape_str_dup("emcc");
	tc->cxx = ape_str_dup("em++");
	tc->ld = ape_str_dup("emcc");
	tc->ar = ape_str_dup("emar");
	tc->exe_ext = ape_str_dup(".html");
	tc->shared_lib_ext = ape_str_dup(".js");

	return handle;
}

APEBUILD_DEF void ape_toolchain_set_cc(ApeToolchainHandle handle, const char *cc) {
	ApeToolchain *tc = ape_toolchain_get(handle);
	if (!tc) return;
	APEBUILD_FREE(tc->cc);
	tc->cc = ape_str_dup(cc);
}

APEBUILD_DEF void ape_toolchain_set_cxx(ApeToolchainHandle handle, const char *cxx) {
	ApeToolchain *tc = ape_toolchain_get(handle);
	if (!tc) return;
	APEBUILD_FREE(tc->cxx);
	tc->cxx = ape_str_dup(cxx);
}

APEBUILD_DEF void ape_toolchain_set_ld(ApeToolchainHandle handle, const char *ld) {
	ApeToolchain *tc = ape_toolchain_get(handle);
	if (!tc) return;
	APEBUILD_FREE(tc->ld);
	tc->ld = ape_str_dup(ld);
}

APEBUILD_DEF void ape_toolchain_set_ar(ApeToolchainHandle handle, const char *ar) {
	ApeToolchain *tc = ape_toolchain_get(handle);
	if (!tc) return;
	APEBUILD_FREE(tc->ar);
	tc->ar = ape_str_dup(ar);
}

APEBUILD_DEF void ape_toolchain_add_cflag(ApeToolchainHandle handle, const char *flag) {
	ApeToolchain *tc = ape_toolchain_get(handle);
	if (!tc) return;
	ape_sl_append_dup(&tc->default_cflags, flag);
}

APEBUILD_DEF void ape_toolchain_add_ldflag(ApeToolchainHandle handle, const char *flag) {
	ApeToolchain *tc = ape_toolchain_get(handle);
	if (!tc) return;
	ape_sl_append_dup(&tc->default_ldflags, flag);
}

/* ============================================================================
 * Task Implementation
 * ============================================================================ */

APEBUILD_DEF ApeTaskHandle ape_task_new(ApeBuilderHandle builder_handle, ApeTaskType type, const char *name) {
	ape_build_init();

	for (int i = 0; i < APE_MAX_TASKS; i++) {
		if (!ape_task_storage[i].in_use) {
			ApeTask *task = &ape_task_storage[i];
			memset(task, 0, sizeof(ApeTask));

			task->in_use = 1;
			task->type = type;
			task->status = APE_TASK_PENDING;
			task->name = ape_str_dup(name);
			task->builder = builder_handle;
			task->proc = APE_INVALID_HANDLE;
			task->exit_code = -1;
			ape_sl_init(&task->inputs);
			ape_cmd_init(&task->cmd);
			task->deps.count = 0;

			return (ApeTaskHandle)i;
		}
	}
	return APE_INVALID_TASK;
}

APEBUILD_DEF void ape_task_free(ApeTaskHandle handle) {
	ApeTask *task = ape_task_get(handle);
	if (task) { ape_task_clear(task); }
}

APEBUILD_DEF ApeTask *ape_task_get(ApeTaskHandle handle) {
	if (handle < 0 || handle >= APE_MAX_TASKS) return NULL;
	if (!ape_task_storage[handle].in_use) return NULL;
	return &ape_task_storage[handle];
}

APEBUILD_DEF int ape_task_valid(ApeTaskHandle handle) { return ape_task_get(handle) != NULL; }

APEBUILD_DEF void ape_task_set_input(ApeTaskHandle handle, const char *input) {
	ApeTask *task = ape_task_get(handle);
	if (!task) return;
	APEBUILD_FREE(task->input);
	task->input = ape_str_dup(input);
}

APEBUILD_DEF void ape_task_set_output(ApeTaskHandle handle, const char *output) {
	ApeTask *task = ape_task_get(handle);
	if (!task) return;
	APEBUILD_FREE(task->output);
	task->output = ape_str_dup(output);
}

APEBUILD_DEF void ape_task_add_input(ApeTaskHandle handle, const char *input) {
	ApeTask *task = ape_task_get(handle);
	if (!task) return;
	ape_sl_append_dup(&task->inputs, input);
}

APEBUILD_DEF void ape_task_add_dep(ApeTaskHandle handle, ApeTaskHandle dep) {
	ApeTask *task = ape_task_get(handle);
	if (!task) return;
	if (task->deps.count >= APE_MAX_TASK_DEPS) return;
	task->deps.items[task->deps.count++] = dep;
}

APEBUILD_DEF void ape_task_set_cmd(ApeTaskHandle handle, ApeCmd cmd) {
	ApeTask *task = ape_task_get(handle);
	if (!task) return;
	ape_cmd_free(&task->cmd);
	task->cmd = cmd;
}

APEBUILD_DEF int ape_task_needs_rebuild(ApeTaskHandle handle) {
	ApeTask *task = ape_task_get(handle);
	if (!task) return APEBUILD_FALSE;

	/* No output means always run */
	if (!task->output) return APEBUILD_TRUE;

	/* Output doesn't exist */
	if (!ape_fs_exists(task->output)) return APEBUILD_TRUE;

	/* Check primary input */
	if (task->input) {
		if (ape_fs_is_newer(task->input, task->output)) return APEBUILD_TRUE;
	}

	/* Check additional inputs */
	for (size_t i = 0; i < task->inputs.count; i++) {
		if (ape_fs_is_newer(task->inputs.items[i], task->output)) return APEBUILD_TRUE;
	}

	return APEBUILD_FALSE;
}

APEBUILD_DEF int ape_task_ready(ApeTaskHandle handle) {
	ApeTask *task = ape_task_get(handle);
	if (!task) return APEBUILD_FALSE;

	for (size_t i = 0; i < task->deps.count; i++) {
		ApeTask *dep = ape_task_get(task->deps.items[i]);
		if (!dep) continue;
		if (dep->status != APE_TASK_COMPLETED && dep->status != APE_TASK_SKIPPED) { return APEBUILD_FALSE; }
	}
	return APEBUILD_TRUE;
}

/* ============================================================================
 * Builder Implementation
 * ============================================================================ */

APEBUILD_DEF ApeBuilderHandle ape_builder_new(const char *name) {
	ape_build_init();

	for (int i = 0; i < APE_MAX_BUILDERS; i++) {
		if (!ape_builder_storage[i].in_use) {
			ApeBuilder *builder = &ape_builder_storage[i];
			memset(builder, 0, sizeof(ApeBuilder));

			builder->in_use = 1;
			builder->name = ape_str_dup(name);
			builder->type = APE_TARGET_EXECUTABLE;
			builder->toolchain = APE_INVALID_TOOLCHAIN;

			ape_sl_init(&builder->sources);
			ape_sl_init(&builder->cflags);
			ape_sl_init(&builder->include_dirs);
			ape_sl_init(&builder->defines);
			ape_sl_init(&builder->ldflags);
			ape_sl_init(&builder->lib_dirs);
			ape_sl_init(&builder->libs);

			/* Emscripten/WASM defaults */
			builder->shell_file = NULL;
			builder->wasm_html_output = 1; /* Default to .html output */
			ape_sl_init(&builder->exported_functions);
			ape_sl_init(&builder->preload_files);
			ape_sl_init(&builder->embed_files);
			builder->wasm_initial_memory = 0;
			builder->wasm_max_memory = 0;

			builder->deps.count = 0;
			builder->tasks.count = 0;

			return (ApeBuilderHandle)i;
		}
	}
	return APE_INVALID_BUILDER;
}

APEBUILD_DEF void ape_builder_free(ApeBuilderHandle handle) {
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder) return;

	/* Free associated tasks */
	for (size_t i = 0; i < builder->tasks.count; i++) { ape_task_free(builder->tasks.items[i]); }

	ape_builder_clear(builder);
}

APEBUILD_DEF ApeBuilder *ape_builder_get(ApeBuilderHandle handle) {
	if (handle < 0 || handle >= APE_MAX_BUILDERS) return NULL;
	if (!ape_builder_storage[handle].in_use) return NULL;
	return &ape_builder_storage[handle];
}

APEBUILD_DEF int ape_builder_valid(ApeBuilderHandle handle) { return ape_builder_get(handle) != NULL; }

APEBUILD_DEF ApeBuilderHandle ape_builder_find(const char *name) {
	for (int i = 0; i < APE_MAX_BUILDERS; i++) {
		if (ape_builder_storage[i].in_use && ape_str_eq(ape_builder_storage[i].name, name)) { return (ApeBuilderHandle)i; }
	}
	return APE_INVALID_BUILDER;
}

APEBUILD_DEF void ape_builder_set_type(ApeBuilderHandle handle, ApeTargetType type) {
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder) return;
	builder->type = type;
}

APEBUILD_DEF void ape_builder_set_toolchain(ApeBuilderHandle handle, ApeToolchainHandle tc) {
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder) return;
	builder->toolchain = tc;
}

APEBUILD_DEF void ape_builder_set_output_dir(ApeBuilderHandle handle, const char *dir) {
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder) return;
	APEBUILD_FREE(builder->output_dir);
	builder->output_dir = ape_str_dup(dir);
}

APEBUILD_DEF void ape_builder_set_output_name(ApeBuilderHandle handle, const char *name) {
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder) return;
	APEBUILD_FREE(builder->output_name);
	builder->output_name = ape_str_dup(name);
}

/* Source management */

APEBUILD_DEF void ape_builder_add_source(ApeBuilderHandle handle, const char *path) {
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder) return;
	ape_sl_append_dup(&builder->sources, path);
}

APEBUILD_DEF void ape_builder_add_sources(ApeBuilderHandle handle, const char **paths, size_t count) {
	for (size_t i = 0; i < count; i++) { ape_builder_add_source(handle, paths[i]); }
}

typedef struct {
	ApeBuilderHandle builder;
} BuilderAddSourceCtx;

APEBUILD_PRIVATE void ape_builder_add_source_callback(const char *path, const ApeDirEntry *entry, void *userdata) {
	BuilderAddSourceCtx *ctx = (BuilderAddSourceCtx *)userdata;
	ApeBuilder *builder = ape_builder_get(ctx->builder);
	if (!builder) return;

	if (!entry->is_file) return;

	/* Only add C/C++ source files */
	if (ape_str_ends_with(entry->name, ".c") || ape_str_ends_with(entry->name, ".cpp") || ape_str_ends_with(entry->name, ".cc") ||
	    ape_str_ends_with(entry->name, ".cxx")) {
		char *fullpath = ape_fs_join(path, entry->name);
		ape_sl_append(&builder->sources, fullpath);
	}
}

APEBUILD_DEF void ape_builder_add_source_dir(ApeBuilderHandle handle, const char *dir) {
	BuilderAddSourceCtx ctx = { .builder = handle };
	ape_fs_iterdir(dir, ape_builder_add_source_callback, &ctx);
}

APEBUILD_DEF void ape_builder_add_source_dir_r(ApeBuilderHandle handle, const char *dir) {
	BuilderAddSourceCtx ctx = { .builder = handle };
	ape_fs_iterdir_r(dir, ape_builder_add_source_callback, &ctx);
}

APEBUILD_DEF void ape_builder_add_source_glob(ApeBuilderHandle handle, const char *pattern) {
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder) return;

	ApeStrList files = ape_fs_glob(pattern);
	for (size_t i = 0; i < files.count; i++) {
		/* Transfer ownership */
		ape_sl_append(&builder->sources, files.items[i]);
	}
	/* Free list but not strings (ownership transferred) */
	ape_sl_free_shallow(&files);
}

/* Compiler flags */

APEBUILD_DEF void ape_builder_add_cflag(ApeBuilderHandle handle, const char *flag) {
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder) return;
	ape_sl_append_dup(&builder->cflags, flag);
}

APEBUILD_DEF void ape_builder_add_include(ApeBuilderHandle handle, const char *dir) {
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder) return;
	ape_sl_append_dup(&builder->include_dirs, dir);
}

APEBUILD_DEF void ape_builder_add_define(ApeBuilderHandle handle, const char *define) {
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder) return;
	ape_sl_append_dup(&builder->defines, define);
}

APEBUILD_DEF void ape_builder_add_define_value(ApeBuilderHandle handle, const char *name, const char *value) {
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder) return;

	ApeStrBuilder sb = ape_sb_new();
	ape_sb_append_str(&sb, name);
	ape_sb_append_char(&sb, '=');
	ape_sb_append_str(&sb, value);
	ape_sl_append(&builder->defines, ape_sb_to_str_dup(&sb));
	ape_sb_free(&sb);
}

/* Linker flags */

APEBUILD_DEF void ape_builder_add_ldflag(ApeBuilderHandle handle, const char *flag) {
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder) return;
	ape_sl_append_dup(&builder->ldflags, flag);
}

APEBUILD_DEF void ape_builder_add_lib_dir(ApeBuilderHandle handle, const char *dir) {
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder) return;
	ape_sl_append_dup(&builder->lib_dirs, dir);
}

APEBUILD_DEF void ape_builder_add_lib(ApeBuilderHandle handle, const char *lib) {
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder) return;
	ape_sl_append_dup(&builder->libs, lib);
}

/* Emscripten/WASM configuration */

APEBUILD_DEF void ape_builder_set_shell_file(ApeBuilderHandle handle, const char *path) {
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder) return;
	APEBUILD_FREE(builder->shell_file);
	builder->shell_file = ape_str_dup(path);
}

APEBUILD_DEF void ape_builder_set_wasm_output_mode(ApeBuilderHandle handle, int html) {
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder) return;
	builder->wasm_html_output = html;
}

APEBUILD_DEF void ape_builder_add_exported_function(ApeBuilderHandle handle, const char *func) {
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder) return;
	ape_sl_append_dup(&builder->exported_functions, func);
}

APEBUILD_DEF void ape_builder_add_preload_file(ApeBuilderHandle handle, const char *path) {
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder) return;
	ape_sl_append_dup(&builder->preload_files, path);
}

APEBUILD_DEF void ape_builder_add_embed_file(ApeBuilderHandle handle, const char *path) {
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder) return;
	ape_sl_append_dup(&builder->embed_files, path);
}

APEBUILD_DEF void ape_builder_set_wasm_memory(ApeBuilderHandle handle, int initial_mb, int max_mb) {
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder) return;
	builder->wasm_initial_memory = initial_mb;
	builder->wasm_max_memory = max_mb;
}

APEBUILD_DEF const char *ape_emcc_default_shell(void) { return ape_emcc_default_shell_html; }

/* Dependencies */

APEBUILD_DEF void ape_builder_depends_on(ApeBuilderHandle handle, ApeBuilderHandle dep) {
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder) return;
	if (builder->deps.count >= APE_MAX_BUILDER_DEPS) return;
	builder->deps.items[builder->deps.count++] = dep;
}

APEBUILD_DEF void ape_builder_link_with(ApeBuilderHandle handle, ApeBuilderHandle lib_builder_handle) {
	ApeBuilder *builder = ape_builder_get(handle);
	ApeBuilder *lib_builder = ape_builder_get(lib_builder_handle);
	if (!builder || !lib_builder) return;

	ape_builder_depends_on(handle, lib_builder_handle);

	/* Add library to link */
	char *output = ape_builder_output_path(lib_builder_handle);
	if (output) {
		char *dir = ape_fs_dirname(output);
		ape_builder_add_lib_dir(handle, dir);
		APEBUILD_FREE(dir);

		/* Extract library name */
		char *stem = ape_fs_stem(output);

		/* Remove lib prefix if present */
		ApeToolchain *tc = ape_toolchain_get(lib_builder->toolchain);
		if (tc && tc->lib_prefix && ape_str_starts_with(stem, tc->lib_prefix)) {
			char *lib_name = ape_str_dup(stem + strlen(tc->lib_prefix));
			ape_builder_add_lib(handle, lib_name);
			APEBUILD_FREE(lib_name);
		} else {
			ape_builder_add_lib(handle, stem);
		}

		APEBUILD_FREE(stem);
		APEBUILD_FREE(output);
	}
}

/* Build output path */

APEBUILD_DEF char *ape_builder_output_path(ApeBuilderHandle handle) {
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder) return NULL;

	ApeToolchain *tc = ape_toolchain_get(builder->toolchain);
	if (!tc) return NULL;

	const char *output_dir = builder->output_dir;
	if (!output_dir) output_dir = "build";

	const char *name = builder->output_name ? builder->output_name : builder->name;

	ApeStrBuilder sb = ape_sb_new();
	ape_sb_append_str(&sb, output_dir);
	ape_sb_append_char(&sb, '/');

	switch (builder->type) {
	case APE_TARGET_EXECUTABLE:
		ape_sb_append_str(&sb, name);
		ape_sb_append_str(&sb, tc->exe_ext);
		break;
	case APE_TARGET_STATIC_LIB:
		ape_sb_append_str(&sb, tc->lib_prefix);
		ape_sb_append_str(&sb, name);
		ape_sb_append_str(&sb, tc->static_lib_ext);
		break;
	case APE_TARGET_SHARED_LIB:
		ape_sb_append_str(&sb, tc->lib_prefix);
		ape_sb_append_str(&sb, name);
		ape_sb_append_str(&sb, tc->shared_lib_ext);
		break;
	case APE_TARGET_OBJECT:
		ape_sb_append_str(&sb, name);
		ape_sb_append_str(&sb, tc->obj_ext);
		break;
	case APE_TARGET_WASM:
		ape_sb_append_str(&sb, name);
		if (builder->wasm_html_output) {
			ape_sb_append_str(&sb, ".html");
		} else {
			ape_sb_append_str(&sb, ".js");
		}
		break;
	}

	char *result = ape_sb_to_str_dup(&sb);
	ape_sb_free(&sb);
	return result;
}

/* Task generation */

APEBUILD_DEF ApeTaskHandle ape_builder_add_compile_task(ApeBuilderHandle handle, const char *source) {
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder) return APE_INVALID_TASK;

	char *obj_path = ape_build_obj_path(NULL, handle, source);
	char *base = ape_fs_basename(source);

	ApeStrBuilder name_sb = ape_sb_new();
	ape_sb_append_str(&name_sb, "Compile ");
	ape_sb_append_str(&name_sb, base);
	char *task_name = ape_sb_to_str_dup(&name_sb);
	ape_sb_free(&name_sb);

	ApeTaskHandle task_handle = ape_task_new(handle, APE_TASK_TYPE_COMPILE, task_name);
	APEBUILD_FREE(task_name);
	APEBUILD_FREE(base);

	ApeTask *task = ape_task_get(task_handle);
	if (!task) {
		APEBUILD_FREE(obj_path);
		return APE_INVALID_TASK;
	}

	ape_task_set_input(task_handle, source);
	ape_task_set_output(task_handle, obj_path);

	/* Build compile command */
	ApeToolchain *tc = ape_toolchain_get(builder->toolchain);
	if (!tc) {
		APEBUILD_FREE(obj_path);
		return APE_INVALID_TASK;
	}

	ApeCmd cmd = ape_cmd_new();

	/* Compiler */
	if (ape_str_ends_with(source, ".cpp") || ape_str_ends_with(source, ".cc") || ape_str_ends_with(source, ".cxx")) {
		ape_cmd_append(&cmd, tc->cxx);
	} else {
		ape_cmd_append(&cmd, tc->cc);
	}

	/* Default flags from toolchain */
	for (size_t i = 0; i < tc->default_cflags.count; i++) { ape_cmd_append(&cmd, tc->default_cflags.items[i]); }

	/* Builder-specific flags */
	for (size_t i = 0; i < builder->cflags.count; i++) { ape_cmd_append(&cmd, builder->cflags.items[i]); }

	/* Include directories */
	for (size_t i = 0; i < builder->include_dirs.count; i++) {
		ApeStrBuilder inc = ape_sb_new();
		ape_sb_append_str(&inc, "-I");
		ape_sb_append_str(&inc, builder->include_dirs.items[i]);
		ape_cmd_append(&cmd, ape_sb_to_str_dup(&inc));
		ape_sb_free(&inc);
	}

	/* Defines */
	for (size_t i = 0; i < builder->defines.count; i++) {
		ApeStrBuilder def = ape_sb_new();
		ape_sb_append_str(&def, "-D");
		ape_sb_append_str(&def, builder->defines.items[i]);
		ape_cmd_append(&cmd, ape_sb_to_str_dup(&def));
		ape_sb_free(&def);
	}

	/* PIC for shared libraries */
	if (builder->type == APE_TARGET_SHARED_LIB) { ape_cmd_append(&cmd, "-fPIC"); }

	/* Compile only */
	ape_cmd_append(&cmd, "-c");
	ape_cmd_append(&cmd, source);
	ape_cmd_append(&cmd, "-o");
	ape_cmd_append(&cmd, obj_path);

	ape_task_set_cmd(task_handle, cmd);

	/* Add to builder's task list */
	if (builder->tasks.count < APE_MAX_TASKS) { builder->tasks.items[builder->tasks.count++] = task_handle; }

	return task_handle;
}

APEBUILD_DEF ApeTaskHandle ape_builder_add_link_task(ApeBuilderHandle handle) {
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder) return APE_INVALID_TASK;

	char *output = ape_builder_output_path(handle);
	if (!output) return APE_INVALID_TASK;

	ApeStrBuilder name_sb = ape_sb_new();
	ape_sb_append_str(&name_sb, "Link ");
	ape_sb_append_str(&name_sb, builder->name);
	char *task_name = ape_sb_to_str_dup(&name_sb);
	ape_sb_free(&name_sb);

	ApeTaskHandle task_handle = ape_task_new(handle, APE_TASK_TYPE_LINK, task_name);
	APEBUILD_FREE(task_name);

	ApeTask *task = ape_task_get(task_handle);
	if (!task) {
		APEBUILD_FREE(output);
		return APE_INVALID_TASK;
	}

	ape_task_set_output(task_handle, output);

	/* Build link command */
	ApeToolchain *tc = ape_toolchain_get(builder->toolchain);
	if (!tc) {
		APEBUILD_FREE(output);
		return APE_INVALID_TASK;
	}

	ApeCmd cmd = ape_cmd_new();
	ape_cmd_append(&cmd, tc->ld);

	/* Default linker flags */
	for (size_t i = 0; i < tc->default_ldflags.count; i++) { ape_cmd_append(&cmd, tc->default_ldflags.items[i]); }

	/* Builder-specific linker flags */
	for (size_t i = 0; i < builder->ldflags.count; i++) { ape_cmd_append(&cmd, builder->ldflags.items[i]); }

	/* Shared library flags */
	if (builder->type == APE_TARGET_SHARED_LIB) { ape_cmd_append(&cmd, "-shared"); }

	/* Output */
	ape_cmd_append(&cmd, "-o");
	ape_cmd_append(&cmd, output);

	/* Object files (from compile tasks) */
	for (size_t i = 0; i < builder->tasks.count; i++) {
		ApeTaskHandle compile_handle = builder->tasks.items[i];
		ApeTask *compile_task = ape_task_get(compile_handle);
		if (compile_task && compile_task->type == APE_TASK_TYPE_COMPILE) {
			ape_cmd_append(&cmd, compile_task->output);
			ape_task_add_input(task_handle, compile_task->output);
			ape_task_add_dep(task_handle, compile_handle);
		}
	}

	/* Emscripten/WASM-specific linker flags */
	if (builder->type == APE_TARGET_WASM) {
		/* Shell file: use custom if set, otherwise write default to build dir */
		if (builder->shell_file) {
			ape_cmd_append(&cmd, "--shell-file");
			ape_cmd_append(&cmd, builder->shell_file);
			ape_task_add_input(task_handle, builder->shell_file);
		} else if (builder->wasm_html_output) {
			/* Write the embedded default shell to the output directory */
			const char *output_dir_str = builder->output_dir ? builder->output_dir : "build";
			ApeStrBuilder shell_path_sb = ape_sb_new();
			ape_sb_append_str(&shell_path_sb, output_dir_str);
			ape_sb_append_str(&shell_path_sb, "/ape_shell_default.html");
			char *shell_path = ape_sb_to_str_dup(&shell_path_sb);
			ape_sb_free(&shell_path_sb);

			ape_fs_mkdir_p(output_dir_str);
			ape_fs_write_file(shell_path, ape_emcc_default_shell_html, strlen(ape_emcc_default_shell_html));
			ape_cmd_append(&cmd, "--shell-file");
			ape_cmd_append(&cmd, shell_path);
			/* shell_path ownership: leaked intentionally as cmd holds a reference */
		}

		/* Exported functions */
		if (builder->exported_functions.count > 0) {
			ApeStrBuilder exports_sb = ape_sb_new();
			ape_sb_append_str(&exports_sb, "-sEXPORTED_FUNCTIONS=");
			for (size_t i = 0; i < builder->exported_functions.count; i++) {
				if (i > 0) ape_sb_append_char(&exports_sb, ',');
				ape_sb_append_str(&exports_sb, builder->exported_functions.items[i]);
			}
			ape_cmd_append(&cmd, ape_sb_to_str_dup(&exports_sb));
			ape_sb_free(&exports_sb);
		}

		/* Preload files */
		for (size_t i = 0; i < builder->preload_files.count; i++) {
			ape_cmd_append(&cmd, "--preload-file");
			ape_cmd_append(&cmd, builder->preload_files.items[i]);
		}

		/* Embed files */
		for (size_t i = 0; i < builder->embed_files.count; i++) {
			ape_cmd_append(&cmd, "--embed-file");
			ape_cmd_append(&cmd, builder->embed_files.items[i]);
		}

		/* Memory settings */
		if (builder->wasm_initial_memory > 0) {
			ApeStrBuilder mem_sb = ape_sb_new();
			ape_sb_append_fmt(&mem_sb, "-sINITIAL_MEMORY=%d", builder->wasm_initial_memory * 1024 * 1024);
			ape_cmd_append(&cmd, ape_sb_to_str_dup(&mem_sb));
			ape_sb_free(&mem_sb);
		}
		if (builder->wasm_max_memory > 0) {
			ape_cmd_append(&cmd, "-sALLOW_MEMORY_GROWTH=1");
			ApeStrBuilder mem_sb = ape_sb_new();
			ape_sb_append_fmt(&mem_sb, "-sMAXIMUM_MEMORY=%d", builder->wasm_max_memory * 1024 * 1024);
			ape_cmd_append(&cmd, ape_sb_to_str_dup(&mem_sb));
			ape_sb_free(&mem_sb);
		}
	}

	/* Library directories */
	for (size_t i = 0; i < builder->lib_dirs.count; i++) {
		ApeStrBuilder lib_dir = ape_sb_new();
		ape_sb_append_str(&lib_dir, "-L");
		ape_sb_append_str(&lib_dir, builder->lib_dirs.items[i]);
		ape_cmd_append(&cmd, ape_sb_to_str_dup(&lib_dir));
		ape_sb_free(&lib_dir);
	}

	/* Libraries */
	for (size_t i = 0; i < builder->libs.count; i++) {
		ApeStrBuilder lib = ape_sb_new();
		ape_sb_append_str(&lib, "-l");
		ape_sb_append_str(&lib, builder->libs.items[i]);
		ape_cmd_append(&cmd, ape_sb_to_str_dup(&lib));
		ape_sb_free(&lib);
	}

	ape_task_set_cmd(task_handle, cmd);

	/* Add to builder's task list */
	if (builder->tasks.count < APE_MAX_TASKS) { builder->tasks.items[builder->tasks.count++] = task_handle; }

	return task_handle;
}

APEBUILD_DEF ApeTaskHandle ape_builder_add_archive_task(ApeBuilderHandle handle) {
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder) return APE_INVALID_TASK;

	char *output = ape_builder_output_path(handle);
	if (!output) return APE_INVALID_TASK;

	ApeStrBuilder name_sb = ape_sb_new();
	ape_sb_append_str(&name_sb, "Archive ");
	ape_sb_append_str(&name_sb, builder->name);
	char *task_name = ape_sb_to_str_dup(&name_sb);
	ape_sb_free(&name_sb);

	ApeTaskHandle task_handle = ape_task_new(handle, APE_TASK_TYPE_ARCHIVE, task_name);
	APEBUILD_FREE(task_name);

	ApeTask *task = ape_task_get(task_handle);
	if (!task) {
		APEBUILD_FREE(output);
		return APE_INVALID_TASK;
	}

	ape_task_set_output(task_handle, output);

	/* Build archive command */
	ApeToolchain *tc = ape_toolchain_get(builder->toolchain);
	if (!tc) {
		APEBUILD_FREE(output);
		return APE_INVALID_TASK;
	}

	ApeCmd cmd = ape_cmd_new();
	ape_cmd_append(&cmd, tc->ar);
	ape_cmd_append(&cmd, "rcs");
	ape_cmd_append(&cmd, output);

	/* Object files */
	for (size_t i = 0; i < builder->tasks.count; i++) {
		ApeTaskHandle compile_handle = builder->tasks.items[i];
		ApeTask *compile_task = ape_task_get(compile_handle);
		if (compile_task && compile_task->type == APE_TASK_TYPE_COMPILE) {
			ape_cmd_append(&cmd, compile_task->output);
			ape_task_add_input(task_handle, compile_task->output);
			ape_task_add_dep(task_handle, compile_handle);
		}
	}

	ape_task_set_cmd(task_handle, cmd);

	/* Add to builder's task list */
	if (builder->tasks.count < APE_MAX_TASKS) { builder->tasks.items[builder->tasks.count++] = task_handle; }

	return task_handle;
}

APEBUILD_DEF ApeTaskHandle ape_builder_add_command_task(ApeBuilderHandle handle, const char *name, ApeCmd cmd) {
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder) return APE_INVALID_TASK;

	ApeTaskHandle task_handle = ape_task_new(handle, APE_TASK_TYPE_COMMAND, name);
	ape_task_set_cmd(task_handle, cmd);

	/* Add to builder's task list */
	if (builder->tasks.count < APE_MAX_TASKS) { builder->tasks.items[builder->tasks.count++] = task_handle; }

	return task_handle;
}

APEBUILD_DEF void ape_builder_generate_tasks(ApeBuilderHandle handle) {
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder) return;

	/* Clear existing tasks */
	for (size_t i = 0; i < builder->tasks.count; i++) { ape_task_free(builder->tasks.items[i]); }
	builder->tasks.count = 0;

	/* Create compile tasks for each source */
	for (size_t i = 0; i < builder->sources.count; i++) { ape_builder_add_compile_task(handle, builder->sources.items[i]); }

	/* Create link/archive task if not object-only */
	if (builder->type != APE_TARGET_OBJECT) {
		if (builder->type == APE_TARGET_STATIC_LIB) {
			ape_builder_add_archive_task(handle);
		} else {
			/* EXECUTABLE, SHARED_LIB, and WASM all use link task */
			ape_builder_add_link_task(handle);
		}
	}
}

/* ============================================================================
 * Build Context Implementation
 * ============================================================================ */

APEBUILD_DEF void ape_ctx_init(ApeBuildCtx *ctx) {
	ape_build_init();
	memset(ctx, 0, sizeof(ApeBuildCtx));

	ctx->toolchain = ape_toolchain_gcc();
	ctx->output_dir = ape_str_dup("build");
	ctx->parallel_jobs = 0; /* Auto-detect */
	ctx->verbosity = APE_VERBOSE_NORMAL;
}

APEBUILD_DEF void ape_ctx_cleanup(ApeBuildCtx *ctx) {
	if (!ctx) return;

	/* Note: We don't free the toolchain here since it's in global storage */
	APEBUILD_FREE(ctx->output_dir);
	memset(ctx, 0, sizeof(ApeBuildCtx));
}

APEBUILD_DEF void ape_ctx_set_toolchain(ApeBuildCtx *ctx, ApeToolchainHandle tc) { ctx->toolchain = tc; }

APEBUILD_DEF void ape_ctx_set_output_dir(ApeBuildCtx *ctx, const char *dir) {
	APEBUILD_FREE(ctx->output_dir);
	ctx->output_dir = ape_str_dup(dir);
}

APEBUILD_DEF void ape_ctx_set_parallel(ApeBuildCtx *ctx, int jobs) { ctx->parallel_jobs = jobs; }

APEBUILD_DEF void ape_ctx_set_verbosity(ApeBuildCtx *ctx, ApeVerbosity level) { ctx->verbosity = level; }

APEBUILD_DEF void ape_ctx_set_force_rebuild(ApeBuildCtx *ctx, int force) { ctx->force_rebuild = force; }

APEBUILD_DEF void ape_ctx_set_dry_run(ApeBuildCtx *ctx, int dry_run) { ctx->dry_run = dry_run; }

APEBUILD_DEF void ape_ctx_set_keep_going(ApeBuildCtx *ctx, int keep_going) { ctx->keep_going = keep_going; }

APEBUILD_DEF ApeToolchainHandle ape_ctx_get_toolchain(ApeBuildCtx *ctx) { return ctx->toolchain; }

/* ============================================================================
 * Build Operations
 * ============================================================================ */

APEBUILD_PRIVATE int ape_builder_run_tasks(ApeBuilderHandle handle, ApeBuildCtx *ctx) {
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder) return APEBUILD_FALSE;

	ApeVerbosity verbosity = ctx ? ctx->verbosity : APE_VERBOSE_NORMAL;
	int max_parallel = ctx ? ctx->parallel_jobs : 0;
	if (max_parallel <= 0) max_parallel = ape_build_get_cpu_count();

	/* Simple task scheduler using handle lists */
	ApeTaskHandleList pending = { 0 };
	ApeTaskHandleList running = { 0 };
	int completed = 0, failed = 0, skipped = 0;

	/* Add all tasks to pending */
	for (size_t i = 0; i < builder->tasks.count; i++) { pending.items[pending.count++] = builder->tasks.items[i]; }

	while (pending.count > 0 || running.count > 0) {
		/* Check for completed tasks */
		for (size_t i = 0; i < running.count;) {
			ApeTask *task = ape_task_get(running.items[i]);
			if (!task) {
				/* Remove invalid task */
				for (size_t j = i; j < running.count - 1; j++) running.items[j] = running.items[j + 1];
				running.count--;
				continue;
			}

			if (ape_proc_poll(task->proc)) {
				ApeProcResult result = ape_proc_result(task->proc);
				task->exit_code = result.exit_code;

				if (result.status == APE_PROC_COMPLETED && result.exit_code == 0) {
					task->status = APE_TASK_COMPLETED;
					completed++;
				} else {
					task->status = APE_TASK_FAILED;
					failed++;
					if (verbosity >= APE_VERBOSE_NORMAL) {
						ape_log_failure("%s failed (exit code %d)", task->name, task->exit_code);
					}
				}

				ape_proc_handle_release(task->proc);
				task->proc = APE_INVALID_HANDLE;

				/* Remove from running */
				for (size_t j = i; j < running.count - 1; j++) running.items[j] = running.items[j + 1];
				running.count--;

				/* If not keep_going and a task failed, exit early after draining running tasks */
				if (failed > 0 && (!ctx || !ctx->keep_going)) {
					/* Wait for remaining running tasks to complete */
					while (running.count > 0) {
						for (size_t k = 0; k < running.count;) {
							ApeTask *rt = ape_task_get(running.items[k]);
							if (rt && ape_proc_poll(rt->proc)) {
								ape_proc_handle_release(rt->proc);
								for (size_t m = k; m < running.count - 1; m++)
									running.items[m] = running.items[m + 1];
								running.count--;
							} else {
								k++;
							}
						}
						usleep(10000);
					}
					/* Mark all pending tasks as failed */
					for (size_t p = 0; p < pending.count; p++) {
						ApeTask *pt = ape_task_get(pending.items[p]);
						if (pt && pt->status == APE_TASK_PENDING) { pt->status = APE_TASK_FAILED; }
					}
					if (verbosity >= APE_VERBOSE_NORMAL) {
						ape_log_failure("%s: %d failed, %d compiled, %d skipped", builder->name, failed, completed,
								skipped);
					}
					return APEBUILD_FALSE;
				}
			} else {
				i++;
			}
		}

		/* Start new tasks */
		while ((int)running.count < max_parallel && pending.count > 0) {
			/* Find a ready task */
			ApeTaskHandle ready_handle = APE_INVALID_TASK;
			size_t ready_idx = 0;
			for (size_t i = 0; i < pending.count; i++) {
				if (ape_task_ready(pending.items[i])) {
					ready_handle = pending.items[i];
					ready_idx = i;
					break;
				}
			}

			if (ready_handle == APE_INVALID_TASK) break;

			ApeTask *task = ape_task_get(ready_handle);
			if (!task) break;

			/* Check if rebuild needed */
			if (!ctx->force_rebuild && !ape_task_needs_rebuild(ready_handle)) {
				task->status = APE_TASK_SKIPPED;
				skipped++;
				if (verbosity >= APE_VERBOSE_VERBOSE) { ape_log_debug("Skipping %s (up to date)", task->name); }
				/* Remove from pending */
				for (size_t j = ready_idx; j < pending.count - 1; j++) pending.items[j] = pending.items[j + 1];
				pending.count--;
				continue;
			}

			/* Ensure output directory exists */
			if (task->output) {
				char *dir = ape_fs_dirname(task->output);
				ape_fs_mkdir_p(dir);
				APEBUILD_FREE(dir);
			}

			/* Log command */
			if (verbosity >= APE_VERBOSE_VERBOSE) {
				char *cmd_str = ape_cmd_render_quoted(&task->cmd);
				ape_log_cmd("%s", cmd_str);
				APEBUILD_FREE(cmd_str);
			} else if (verbosity >= APE_VERBOSE_NORMAL) {
				ape_log_info("%s", task->name);
			}

			/* Dry run */
			if (ctx && ctx->dry_run) {
				task->status = APE_TASK_COMPLETED;
				task->exit_code = 0;
				completed++;
				/* Remove from pending */
				for (size_t j = ready_idx; j < pending.count - 1; j++) pending.items[j] = pending.items[j + 1];
				pending.count--;
				continue;
			}

			/* Start process */
			task->proc = ape_cmd_start(&task->cmd);
			if (task->proc == APE_INVALID_HANDLE) {
				task->status = APE_TASK_FAILED;
				task->exit_code = -1;
				failed++;
				if (verbosity >= APE_VERBOSE_NORMAL) { ape_log_failure("%s failed to start", task->name); }
				if (!ctx || !ctx->keep_going) {
					/* Wait for running tasks */
					while (running.count > 0) {
						for (size_t i = 0; i < running.count;) {
							ApeTask *t = ape_task_get(running.items[i]);
							if (t && ape_proc_poll(t->proc)) {
								ape_proc_handle_release(t->proc);
								for (size_t j = i; j < running.count - 1; j++)
									running.items[j] = running.items[j + 1];
								running.count--;
							} else {
								i++;
							}
						}
						usleep(10000);
					}
					return APEBUILD_FALSE;
				}
			} else {
				task->status = APE_TASK_RUNNING;
				running.items[running.count++] = ready_handle;
			}

			/* Remove from pending */
			for (size_t j = ready_idx; j < pending.count - 1; j++) pending.items[j] = pending.items[j + 1];
			pending.count--;
		}

		/* Short sleep if tasks are running */
		if (running.count > 0) { usleep(10000); }
	}

	if (verbosity >= APE_VERBOSE_NORMAL) {
		if (failed == 0) {
			ape_log_success("%s: %d compiled, %d skipped", builder->name, completed, skipped);
		} else {
			ape_log_failure("%s: %d failed, %d compiled, %d skipped", builder->name, failed, completed, skipped);
		}
	}

	return failed == 0 ? APEBUILD_TRUE : APEBUILD_FALSE;
}

APEBUILD_DEF int ape_builder_build(ApeBuilderHandle handle) {
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder) return APEBUILD_FALSE;

	if (builder->built) return builder->build_failed ? APEBUILD_FALSE : APEBUILD_TRUE;

	/* We need a context for output_dir - create a temporary one if builder has no toolchain set */
	ApeBuildCtx temp_ctx;
	ApeBuildCtx *ctx = &temp_ctx;
	ape_ctx_init(ctx);

	/* Use builder's toolchain if set, otherwise use context's default */
	if (ape_toolchain_valid(builder->toolchain)) { ctx->toolchain = builder->toolchain; }

	/* Use builder's output_dir if set */
	if (builder->output_dir) {
		APEBUILD_FREE(ctx->output_dir);
		ctx->output_dir = ape_str_dup(builder->output_dir);
	}

	ApeVerbosity verbosity = ctx->verbosity;

	/* Build dependencies first */
	for (size_t i = 0; i < builder->deps.count; i++) {
		if (!ape_builder_build(builder->deps.items[i])) {
			builder->built = 1;
			builder->build_failed = 1;
			ape_ctx_cleanup(ctx);
			return APEBUILD_FALSE;
		}
	}

	/* Ensure output directory exists */
	const char *output_dir = builder->output_dir ? builder->output_dir : ctx->output_dir;
	if (!output_dir) output_dir = "build";
	ape_fs_mkdir_p(output_dir);

	/* Generate tasks */
	ape_builder_generate_tasks(handle);

	if (builder->tasks.count == 0) {
		if (verbosity >= APE_VERBOSE_NORMAL) { ape_log_info("Nothing to build for %s", builder->name); }
		builder->built = 1;
		ape_ctx_cleanup(ctx);
		return APEBUILD_TRUE;
	}

	if (verbosity >= APE_VERBOSE_NORMAL) { ape_log_build("Building %s...", builder->name); }

	int result = ape_builder_run_tasks(handle, ctx);

	builder->built = 1;
	builder->build_failed = !result;

	ape_ctx_cleanup(ctx);
	return result;
}

APEBUILD_DEF int ape_ctx_build(ApeBuildCtx *ctx, ApeBuilderHandle handle) {
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder) return APEBUILD_FALSE;

	/* Set builder's toolchain from context if not already set */
	if (!ape_toolchain_valid(builder->toolchain)) { builder->toolchain = ctx->toolchain; }

	/* Set builder's output_dir from context if not already set */
	if (!builder->output_dir && ctx->output_dir) { builder->output_dir = ape_str_dup(ctx->output_dir); }

	return ape_builder_build(handle);
}

APEBUILD_DEF int ape_builder_clean(ApeBuilderHandle handle) {
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder) return APEBUILD_FALSE;

	/* Remove output */
	char *output = ape_builder_output_path(handle);
	if (output && ape_fs_exists(output)) { ape_fs_remove(output); }
	APEBUILD_FREE(output);

	/* Remove object files */
	for (size_t i = 0; i < builder->sources.count; i++) {
		char *obj = ape_build_obj_path(NULL, handle, builder->sources.items[i]);
		if (obj && ape_fs_exists(obj)) { ape_fs_remove(obj); }
		APEBUILD_FREE(obj);
	}

	builder->built = 0;
	builder->build_failed = 0;

	return APEBUILD_TRUE;
}

APEBUILD_DEF int ape_ctx_clean(ApeBuildCtx *ctx, ApeBuilderHandle handle) {
	(void)ctx;
	return ape_builder_clean(handle);
}

APEBUILD_DEF int ape_builder_rebuild(ApeBuilderHandle handle) {
	ape_builder_clean(handle);
	ApeBuilder *builder = ape_builder_get(handle);
	if (builder) builder->built = 0;
	return ape_builder_build(handle);
}

APEBUILD_DEF int ape_ctx_rebuild(ApeBuildCtx *ctx, ApeBuilderHandle handle) {
	ape_ctx_clean(ctx, handle);
	return ape_ctx_build(ctx, handle);
}

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

APEBUILD_DEF char *ape_build_obj_path(ApeBuildCtx *ctx, ApeBuilderHandle handle, const char *source) {
	ApeBuilder *builder = ape_builder_get(handle);

	ApeToolchain *tc = NULL;
	if (builder) tc = ape_toolchain_get(builder->toolchain);
	if (!tc && ctx) tc = ape_toolchain_get(ctx->toolchain);
	if (!tc) return NULL;

	const char *output_dir = NULL;
	if (builder) output_dir = builder->output_dir;
	if (!output_dir && ctx) output_dir = ctx->output_dir;
	if (!output_dir) output_dir = "build";

	char *stem = ape_fs_stem(source);
	char *dir = ape_fs_dirname(source);

	ApeStrBuilder sb = ape_sb_new();
	ape_sb_append_str(&sb, output_dir);
	ape_sb_append_char(&sb, '/');

	/* Include relative path to avoid collisions */
	if (dir && strcmp(dir, ".") != 0) {
		char *rel = ape_str_replace_all(dir, "/", "_");
		ape_sb_append_str(&sb, rel);
		ape_sb_append_char(&sb, '_');
		APEBUILD_FREE(rel);
	}

	ape_sb_append_str(&sb, stem);
	ape_sb_append_str(&sb, tc->obj_ext);

	APEBUILD_FREE(stem);
	APEBUILD_FREE(dir);

	char *result = ape_sb_to_str_dup(&sb);
	ape_sb_free(&sb);
	return result;
}

APEBUILD_DEF char *ape_build_output_path(ApeBuildCtx *ctx, ApeBuilderHandle handle) {
	(void)ctx;
	return ape_builder_output_path(handle);
}

APEBUILD_DEF int ape_build_get_cpu_count(void) {
#ifdef _SC_NPROCESSORS_ONLN
	long count = sysconf(_SC_NPROCESSORS_ONLN);
	if (count > 0) return (int)count;
#endif
	return 4; /* Default fallback */
}

/* ============================================================================
 * Auto-rebuild Support
 * ============================================================================ */

APEBUILD_DEF int ape_self_needs_rebuild(const char *binary, const char *source) { return ape_fs_needs_rebuild1(binary, source); }

APEBUILD_DEF int ape_self_rebuild(int argc, char **argv, const char *source) {
	const char *binary = argv[0];

	ape_log_info("Build script changed, rebuilding...");

	/* Rename current binary */
	char *old_binary = ape_str_concat(binary, ".old");
	if (!ape_fs_rename(binary, old_binary)) {
		ape_log_error("Failed to rename current binary to %s", old_binary);
		APEBUILD_FREE(old_binary);
		return 1;
	}

	/* Rebuild */
	ApeCmd cmd = ape_cmd_new();
	ape_cmd_append(&cmd, "cc");
	ape_cmd_append(&cmd, "-o");
	ape_cmd_append(&cmd, binary);
	ape_cmd_append(&cmd, source);

	int result = ape_cmd_run_status(&cmd);
	ape_cmd_free(&cmd);

	if (result != 0) {
		ape_log_error("Failed to rebuild build script (exit code: %d)", result);
		/* Restore old binary */
		if (ape_fs_exists(old_binary)) {
			if (ape_fs_rename(old_binary, binary)) {
				ape_log_info("Restored old binary from %s", old_binary);
			} else {
				ape_log_error("Failed to restore old binary from %s", old_binary);
			}
		} else {
			ape_log_error("Old binary %s does not exist, cannot restore", old_binary);
		}
		APEBUILD_FREE(old_binary);
		return 1;
	}

	APEBUILD_FREE(old_binary);

	/* Re-execute */
	ape_log_info("Re-executing build script...");

	ApeCmd run_cmd = ape_cmd_new();
	for (int i = 0; i < argc; i++) { ape_cmd_append(&run_cmd, argv[i]); }

	result = ape_cmd_run_status(&run_cmd);
	ape_cmd_free(&run_cmd);

	return result;
}
/* END ape_build.c */

/* BEGIN ape_cmd.c */
/*
 * ape_cmd.c - Command execution module implementation
 */

/* ============================================================================
 * Process Handle Management
 * ============================================================================ */

#define APE_MAX_PROCS 256

typedef struct {
	pid_t pid;
	ApeProcStatus status;
	int exit_code;
	int signal;
	int in_use;
} ApeProcEntry;

APEBUILD_PRIVATE ApeProcEntry ape_proc_table[APE_MAX_PROCS];
APEBUILD_PRIVATE int ape_proc_table_initialized = 0;

APEBUILD_PRIVATE void ape_proc_table_init(void) {
	if (ape_proc_table_initialized) return;
	for (int i = 0; i < APE_MAX_PROCS; i++) { ape_proc_table[i].in_use = 0; }
	ape_proc_table_initialized = 1;
}

APEBUILD_PRIVATE ApeProcHandle ape_proc_alloc(pid_t pid) {
	ape_proc_table_init();
	for (int i = 0; i < APE_MAX_PROCS; i++) {
		if (!ape_proc_table[i].in_use) {
			ape_proc_table[i].pid = pid;
			ape_proc_table[i].status = APE_PROC_RUNNING;
			ape_proc_table[i].exit_code = 0;
			ape_proc_table[i].signal = 0;
			ape_proc_table[i].in_use = 1;
			return (ApeProcHandle)i;
		}
	}
	return APE_INVALID_HANDLE;
}

APEBUILD_PRIVATE ApeProcEntry *ape_proc_get(ApeProcHandle handle) {
	if (handle < 0 || handle >= APE_MAX_PROCS) return NULL;
	if (!ape_proc_table[handle].in_use) return NULL;
	return &ape_proc_table[handle];
}

/* ============================================================================
 * Process Pool
 * ============================================================================ */

struct ApeProcPool {
	int max_parallel;
	ApeProcHandle *handles;
	int count;
	int capacity;
};

/* ============================================================================
 * Command Building
 * ============================================================================ */

APEBUILD_DEF void ape_cmd_init(ApeCmd *cmd) {
	cmd->capacity = 0;
	cmd->count = 0;
	cmd->items = NULL;
	cmd->cwd = NULL;
	ape_sl_init(&cmd->env);
}

APEBUILD_DEF ApeCmd ape_cmd_new(void) {
	ApeCmd cmd;
	ape_cmd_init(&cmd);
	return cmd;
}

APEBUILD_DEF ApeCmd ape_cmd_from(const char *program) {
	ApeCmd cmd = ape_cmd_new();
	ape_cmd_append(&cmd, program);
	return cmd;
}

APEBUILD_DEF void ape_cmd_append(ApeCmd *cmd, const char *arg) { ape_da_append(cmd, arg); }

APEBUILD_DEF void ape_cmd_append_many(ApeCmd *cmd, const char **args, size_t count) { ape_da_append_many(cmd, args, count); }

APEBUILD_DEF void ape_cmd_prepend(ApeCmd *cmd, const char *arg) { ape_da_prepend(cmd, arg); }

APEBUILD_DEF ApeCmd ape_cmd_clone(const ApeCmd *cmd) {
	ApeCmd result = ape_cmd_new();
	for (size_t i = 0; i < cmd->count; i++) { ape_cmd_append(&result, cmd->items[i]); }
	if (cmd->cwd) { result.cwd = ape_str_dup(cmd->cwd); }
	result.env = ape_sl_clone(&cmd->env);
	return result;
}

APEBUILD_DEF void ape_cmd_free(ApeCmd *cmd) {
	APEBUILD_FREE(cmd->items);
	APEBUILD_FREE(cmd->cwd);
	ape_sl_free(&cmd->env);
	ape_cmd_init(cmd);
}

APEBUILD_DEF void ape_cmd_clear(ApeCmd *cmd) {
	cmd->count = 0;
	APEBUILD_FREE(cmd->cwd);
	cmd->cwd = NULL;
	ape_sl_clear(&cmd->env);
}

/* ============================================================================
 * Command Display
 * ============================================================================ */

APEBUILD_DEF char *ape_cmd_render(const ApeCmd *cmd) {
	ApeStrBuilder sb = ape_sb_new();
	for (size_t i = 0; i < cmd->count; i++) {
		if (i > 0) ape_sb_append_char(&sb, ' ');
		ape_sb_append_str(&sb, cmd->items[i]);
	}
	char *result = ape_sb_to_str_dup(&sb);
	ape_sb_free(&sb);
	ape_log_info("CMD: %s", result);
	return result;
}

APEBUILD_DEF char *ape_cmd_render_quoted(const ApeCmd *cmd) {
	ApeStrBuilder sb = ape_sb_new();
	for (size_t i = 0; i < cmd->count; i++) {
		const char *arg = cmd->items[i];
		if (i > 0) ape_sb_append_char(&sb, ' ');

		/* Check if quoting is needed */
		int needs_quote = 0;
		for (const char *p = arg; *p; p++) {
			if (*p == ' ' || *p == '\t' || *p == '"' || *p == '\'' || *p == '\\' || *p == '$' || *p == '`') {
				needs_quote = 1;
				break;
			}
		}

		if (needs_quote) {
			ape_sb_append_char(&sb, '\'');
			for (const char *p = arg; *p; p++) {
				if (*p == '\'') {
					ape_sb_append_str(&sb, "'\\''");
				} else {
					ape_sb_append_char(&sb, *p);
				}
			}
			ape_sb_append_char(&sb, '\'');
		} else {
			ape_sb_append_str(&sb, arg);
		}
	}
	char *result = ape_sb_to_str_dup(&sb);
	ape_sb_free(&sb);
	return result;
}

APEBUILD_DEF void ape_cmd_print(const ApeCmd *cmd) {
	char *rendered = ape_cmd_render_quoted(cmd);
	fprintf(stderr, "%s\n", rendered);
	APEBUILD_FREE(rendered);
}

APEBUILD_DEF void ape_cmd_log(const ApeCmd *cmd, const char *prefix) {
	char *rendered = ape_cmd_render_quoted(cmd);
	fprintf(stderr, "%s%s\n", prefix ? prefix : "", rendered);
	APEBUILD_FREE(rendered);
}

/* ============================================================================
 * Synchronous Execution
 * ============================================================================ */

APEBUILD_DEF int ape_cmd_run(ApeCmd *cmd) { return ape_cmd_run_status(cmd) == 0 ? APEBUILD_TRUE : APEBUILD_FALSE; }

APEBUILD_DEF int ape_cmd_run_status(ApeCmd *cmd) {
	ApeProcHandle handle = ape_cmd_start(cmd);
	if (handle == APE_INVALID_HANDLE) return -1;

	ape_proc_wait(handle);
	ApeProcResult result = ape_proc_result(handle);
	ape_proc_handle_release(handle);

	if (result.status == APE_PROC_SIGNALED) return 128 + result.signal;
	return result.exit_code;
}

APEBUILD_DEF char *ape_cmd_run_capture(ApeCmd *cmd, int *exit_code) {
	if (cmd->count == 0) return NULL;

	char *rendered = ape_cmd_render(cmd);
	APEBUILD_FREE(rendered);

	int pipefd[2];
	if (pipe(pipefd) == -1) return NULL;

	pid_t pid = fork();
	if (pid < 0) {
		close(pipefd[0]);
		close(pipefd[1]);
		return NULL;
	}

	if (pid == 0) {
		/* Child process */
		close(pipefd[0]);
		dup2(pipefd[1], STDOUT_FILENO);
		close(pipefd[1]);

		if (cmd->cwd) {
			if (chdir(cmd->cwd) != 0) { _exit(127); }
		}

		/* Set environment variables */
		for (size_t i = 0; i < cmd->env.count; i++) { putenv(cmd->env.items[i]); }

		/* Build argv with NULL terminator */
		const char **argv = (const char **)APEBUILD_MALLOC((cmd->count + 1) * sizeof(char *));
		for (size_t i = 0; i < cmd->count; i++) { argv[i] = cmd->items[i]; }
		argv[cmd->count] = NULL;

		execvp(argv[0], (char *const *)argv);
		_exit(127);
	}

	/* Parent process */
	close(pipefd[1]);

	ApeStrBuilder sb = ape_sb_new();
	char buf[4096];
	ssize_t n;
	while ((n = read(pipefd[0], buf, sizeof(buf))) > 0) { ape_sb_append_strn(&sb, buf, n); }
	close(pipefd[0]);

	int status;
	waitpid(pid, &status, 0);

	if (exit_code) {
		if (WIFEXITED(status)) {
			*exit_code = WEXITSTATUS(status);
		} else if (WIFSIGNALED(status)) {
			*exit_code = 128 + WTERMSIG(status);
		} else {
			*exit_code = -1;
		}
	}

	char *result = ape_sb_to_str_dup(&sb);
	ape_sb_free(&sb);
	return result;
}

APEBUILD_DEF int ape_cmds_run_seq(ApeCmdList *cmds) {
	for (size_t i = 0; i < cmds->count; i++) {
		if (!ape_cmd_run(&cmds->items[i])) { return APEBUILD_FALSE; }
	}
	return APEBUILD_TRUE;
}

APEBUILD_DEF int ape_cmds_run_all(ApeCmdList *cmds) {
	int all_success = APEBUILD_TRUE;
	for (size_t i = 0; i < cmds->count; i++) {
		if (!ape_cmd_run(&cmds->items[i])) { all_success = APEBUILD_FALSE; }
	}
	return all_success;
}

/* ============================================================================
 * Asynchronous Execution
 * ============================================================================ */

APEBUILD_DEF ApeProcHandle ape_cmd_start(ApeCmd *cmd) {
	if (cmd->count == 0) return APE_INVALID_HANDLE;

	char *rendered = ape_cmd_render(cmd);
	APEBUILD_FREE(rendered);

	pid_t pid = fork();
	if (pid < 0) { return APE_INVALID_HANDLE; }

	if (pid == 0) {
		/* Child process */
		if (cmd->cwd) {
			if (chdir(cmd->cwd) != 0) { _exit(127); }
		}

		/* Set environment variables */
		for (size_t i = 0; i < cmd->env.count; i++) { putenv(cmd->env.items[i]); }

		/* Build argv with NULL terminator */
		const char **argv = (const char **)APEBUILD_MALLOC((cmd->count + 1) * sizeof(char *));
		for (size_t i = 0; i < cmd->count; i++) { argv[i] = cmd->items[i]; }
		argv[cmd->count] = NULL;

		execvp(argv[0], (char *const *)argv);
		_exit(127);
	}

	return ape_proc_alloc(pid);
}

APEBUILD_DEF int ape_proc_poll(ApeProcHandle handle) {
	ApeProcEntry *entry = ape_proc_get(handle);
	if (!entry) return APEBUILD_FALSE;

	if (entry->status != APE_PROC_RUNNING) return APEBUILD_TRUE;

	int status;
	pid_t result = waitpid(entry->pid, &status, WNOHANG);

	if (result == 0) {
		/* Still running */
		return APEBUILD_FALSE;
	}

	if (result < 0) {
		entry->status = APE_PROC_FAILED;
		return APEBUILD_TRUE;
	}

	if (WIFEXITED(status)) {
		entry->status = APE_PROC_COMPLETED;
		entry->exit_code = WEXITSTATUS(status);
	} else if (WIFSIGNALED(status)) {
		entry->status = APE_PROC_SIGNALED;
		entry->signal = WTERMSIG(status);
	} else {
		entry->status = APE_PROC_UNKNOWN;
	}

	return APEBUILD_TRUE;
}

APEBUILD_DEF int ape_proc_wait(ApeProcHandle handle) {
	ApeProcEntry *entry = ape_proc_get(handle);
	if (!entry) return APEBUILD_FALSE;

	if (entry->status != APE_PROC_RUNNING) return APEBUILD_TRUE;

	int status;
	pid_t result = waitpid(entry->pid, &status, 0);

	if (result < 0) {
		entry->status = APE_PROC_FAILED;
		return APEBUILD_FALSE;
	}

	if (WIFEXITED(status)) {
		entry->status = APE_PROC_COMPLETED;
		entry->exit_code = WEXITSTATUS(status);
	} else if (WIFSIGNALED(status)) {
		entry->status = APE_PROC_SIGNALED;
		entry->signal = WTERMSIG(status);
	} else {
		entry->status = APE_PROC_UNKNOWN;
	}

	return entry->exit_code == 0 ? APEBUILD_TRUE : APEBUILD_FALSE;
}

APEBUILD_DEF int ape_proc_wait_timeout(ApeProcHandle handle, int timeout_ms) {
	ApeProcEntry *entry = ape_proc_get(handle);
	if (!entry) return APEBUILD_FALSE;

	if (entry->status != APE_PROC_RUNNING) return APEBUILD_TRUE;

	/* Simple polling implementation */
	int elapsed = 0;
	int sleep_interval = 10; /* 10ms */

	while (elapsed < timeout_ms) {
		if (ape_proc_poll(handle)) { return APEBUILD_TRUE; }
		usleep(sleep_interval * 1000);
		elapsed += sleep_interval;
	}

	return APEBUILD_FALSE; /* Timeout */
}

APEBUILD_DEF ApeProcStatus ape_proc_status(ApeProcHandle handle) {
	ApeProcEntry *entry = ape_proc_get(handle);
	if (!entry) return APE_PROC_UNKNOWN;

	/* Update status if still running */
	if (entry->status == APE_PROC_RUNNING) { ape_proc_poll(handle); }

	return entry->status;
}

APEBUILD_DEF ApeProcResult ape_proc_result(ApeProcHandle handle) {
	ApeProcResult result = { .status = APE_PROC_UNKNOWN, .exit_code = -1, .signal = 0 };

	ApeProcEntry *entry = ape_proc_get(handle);
	if (!entry) return result;

	result.status = entry->status;
	result.exit_code = entry->exit_code;
	result.signal = entry->signal;
	return result;
}

APEBUILD_DEF int ape_proc_kill(ApeProcHandle handle) {
	ApeProcEntry *entry = ape_proc_get(handle);
	if (!entry) return APEBUILD_FALSE;

	if (entry->status != APE_PROC_RUNNING) return APEBUILD_TRUE;

	if (kill(entry->pid, SIGTERM) == 0) {
		/* Give it a moment to terminate gracefully */
		usleep(100000); /* 100ms */
		if (ape_proc_poll(handle)) { return APEBUILD_TRUE; }
		/* Force kill */
		kill(entry->pid, SIGKILL);
		ape_proc_wait(handle);
		return APEBUILD_TRUE;
	}

	return APEBUILD_FALSE;
}

APEBUILD_DEF int ape_proc_handle_valid(ApeProcHandle handle) { return ape_proc_get(handle) != NULL ? APEBUILD_TRUE : APEBUILD_FALSE; }

APEBUILD_DEF void ape_proc_handle_release(ApeProcHandle handle) {
	ApeProcEntry *entry = ape_proc_get(handle);
	if (entry) { entry->in_use = 0; }
}

/* ============================================================================
 * Parallel Execution
 * ============================================================================ */

APEBUILD_DEF ApeProcPool *ape_pool_new(int max_parallel) {
	ApeProcPool *pool = (ApeProcPool *)APEBUILD_MALLOC(sizeof(ApeProcPool));
	pool->max_parallel = max_parallel > 0 ? max_parallel : 1;
	pool->handles = NULL;
	pool->count = 0;
	pool->capacity = 0;
	return pool;
}

APEBUILD_DEF int ape_pool_submit(ApeProcPool *pool, ApeCmd *cmd) {
	/* Wait if at capacity */
	while (pool->count >= pool->max_parallel) {
		ApeProcHandle finished = ape_pool_wait_any(pool);
		if (finished == APE_INVALID_HANDLE) { return APEBUILD_FALSE; }
	}

	ApeProcHandle handle = ape_cmd_start(cmd);
	if (handle == APE_INVALID_HANDLE) { return APEBUILD_FALSE; }

	/* Add to pool */
	if (pool->count >= pool->capacity) {
		pool->capacity = pool->capacity == 0 ? 16 : pool->capacity * 2;
		pool->handles = (ApeProcHandle *)APEBUILD_REALLOC(pool->handles, pool->capacity * sizeof(ApeProcHandle));
	}
	pool->handles[pool->count++] = handle;

	return APEBUILD_TRUE;
}

APEBUILD_DEF ApeProcHandle ape_pool_wait_any(ApeProcPool *pool) {
	if (pool->count == 0) return APE_INVALID_HANDLE;

	while (1) {
		for (int i = 0; i < pool->count; i++) {
			if (ape_proc_poll(pool->handles[i])) {
				ApeProcHandle handle = pool->handles[i];
				/* Remove from pool */
				for (int j = i; j < pool->count - 1; j++) { pool->handles[j] = pool->handles[j + 1]; }
				pool->count--;
				return handle;
			}
		}
		usleep(10000); /* 10ms */
	}
}

APEBUILD_DEF int ape_pool_wait_all(ApeProcPool *pool) {
	int all_success = APEBUILD_TRUE;

	while (pool->count > 0) {
		ApeProcHandle handle = ape_pool_wait_any(pool);
		if (handle == APE_INVALID_HANDLE) {
			all_success = APEBUILD_FALSE;
			continue;
		}

		ApeProcResult result = ape_proc_result(handle);
		if (result.status != APE_PROC_COMPLETED || result.exit_code != 0) { all_success = APEBUILD_FALSE; }
		ape_proc_handle_release(handle);
	}

	return all_success;
}

APEBUILD_DEF void ape_pool_free(ApeProcPool *pool) {
	if (!pool) return;

	/* Kill any remaining processes */
	for (int i = 0; i < pool->count; i++) {
		ape_proc_kill(pool->handles[i]);
		ape_proc_handle_release(pool->handles[i]);
	}

	APEBUILD_FREE(pool->handles);
	APEBUILD_FREE(pool);
}

APEBUILD_DEF int ape_cmds_run_parallel(ApeCmdList *cmds, int max_parallel) {
	ApeProcPool *pool = ape_pool_new(max_parallel);

	for (size_t i = 0; i < cmds->count; i++) {
		if (!ape_pool_submit(pool, &cmds->items[i])) {
			ape_pool_free(pool);
			return APEBUILD_FALSE;
		}
	}

	int result = ape_pool_wait_all(pool);
	ape_pool_free(pool);
	return result;
}

/* ============================================================================
 * Environment
 * ============================================================================ */

APEBUILD_DEF void ape_cmd_set_env(ApeCmd *cmd, const char *name, const char *value) {
	ApeStrBuilder sb = ape_sb_new();
	ape_sb_append_str(&sb, name);
	ape_sb_append_char(&sb, '=');
	ape_sb_append_str(&sb, value);
	ape_sl_append(&cmd->env, ape_sb_to_str_dup(&sb));
	ape_sb_free(&sb);
}

APEBUILD_DEF void ape_cmd_clear_env(ApeCmd *cmd) { ape_sl_clear(&cmd->env); }

APEBUILD_DEF void ape_cmd_set_cwd(ApeCmd *cmd, const char *cwd) {
	APEBUILD_FREE(cmd->cwd);
	cmd->cwd = cwd ? ape_str_dup(cwd) : NULL;
}

/* ============================================================================
 * Command List Operations
 * ============================================================================ */

APEBUILD_DEF void ape_cmdlist_init(ApeCmdList *list) {
	list->capacity = 0;
	list->count = 0;
	list->items = NULL;
}

APEBUILD_DEF ApeCmdList ape_cmdlist_new(void) {
	ApeCmdList list;
	ape_cmdlist_init(&list);
	return list;
}

APEBUILD_DEF void ape_cmdlist_append(ApeCmdList *list, ApeCmd cmd) { ape_da_append(list, cmd); }

APEBUILD_DEF void ape_cmdlist_free(ApeCmdList *list) {
	for (size_t i = 0; i < list->count; i++) { ape_cmd_free(&list->items[i]); }
	APEBUILD_FREE(list->items);
	ape_cmdlist_init(list);
}
/* END ape_cmd.c */

/* BEGIN ape_fs.c */
/*
 * ape_fs.c - Filesystem module implementation
 */

/* Directory handle structure */
struct ApeDir {
	DIR *dir;
	char *path;
};

/* ============================================================================
 * File Reading/Writing
 * ============================================================================ */

APEBUILD_DEF char *ape_fs_read_file(const char *path, size_t *out_size) {
	FILE *fp = fopen(path, "rb");
	if (!fp) return NULL;

	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	if (size < 0) {
		fclose(fp);
		return NULL;
	}

	char *buffer = (char *)APEBUILD_MALLOC(size + 1);
	if (!buffer) {
		fclose(fp);
		return NULL;
	}

	size_t read_size = fread(buffer, 1, size, fp);
	fclose(fp);

	buffer[read_size] = '\0';
	if (out_size) *out_size = read_size;

	return buffer;
}

APEBUILD_DEF ApeStrList ape_fs_read_file_lines(const char *path) {
	char *content = ape_fs_read_file(path, NULL);
	if (!content) return ape_sl_new();

	ApeStrList result = ape_str_split_lines(content);
	APEBUILD_FREE(content);
	return result;
}

APEBUILD_DEF int ape_fs_write_file(const char *path, const char *data, size_t size) {
	FILE *fp = fopen(path, "wb");
	if (!fp) return APEBUILD_FALSE;

	size_t written = fwrite(data, 1, size, fp);
	fclose(fp);

	return written == size ? APEBUILD_TRUE : APEBUILD_FALSE;
}

APEBUILD_DEF int ape_fs_append_file(const char *path, const char *data, size_t size) {
	FILE *fp = fopen(path, "ab");
	if (!fp) return APEBUILD_FALSE;

	size_t written = fwrite(data, 1, size, fp);
	fclose(fp);

	return written == size ? APEBUILD_TRUE : APEBUILD_FALSE;
}

APEBUILD_DEF int ape_fs_copy_file(const char *src, const char *dst) {
	size_t size;
	char *content = ape_fs_read_file(src, &size);
	if (!content) return APEBUILD_FALSE;

	int result = ape_fs_write_file(dst, content, size);
	APEBUILD_FREE(content);
	return result;
}

APEBUILD_DEF int ape_fs_rename(const char *oldpath, const char *newpath) {
	return rename(oldpath, newpath) == 0 ? APEBUILD_TRUE : APEBUILD_FALSE;
}

/* ============================================================================
 * Directory Operations
 * ============================================================================ */

APEBUILD_DEF int ape_fs_mkdir(const char *path) { return mkdir(path, 0755) == 0 ? APEBUILD_TRUE : APEBUILD_FALSE; }

APEBUILD_DEF int ape_fs_mkdir_p(const char *path) {
	char *tmp = ape_str_dup(path);
	size_t len = strlen(tmp);

	/* Remove trailing slash if present */
	if (len > 0 && tmp[len - 1] == '/') tmp[len - 1] = '\0';

	for (char *p = tmp + 1; *p; p++) {
		if (*p == '/') {
			*p = '\0';
			if (!ape_fs_exists(tmp)) {
				if (!ape_fs_mkdir(tmp)) {
					APEBUILD_FREE(tmp);
					return APEBUILD_FALSE;
				}
			}
			*p = '/';
		}
	}

	int result = APEBUILD_TRUE;
	if (!ape_fs_exists(tmp)) { result = ape_fs_mkdir(tmp); }

	APEBUILD_FREE(tmp);
	return result;
}

APEBUILD_DEF int ape_fs_rmdir(const char *path) { return rmdir(path) == 0 ? APEBUILD_TRUE : APEBUILD_FALSE; }
APEBUILD_PRIVATE void ape_fs_rmdir_r_callback(const char *path, const ApeDirEntry *entry, void *userdata) {
	(void)userdata;
	char *fullpath = ape_fs_join(path, entry->name);

	if (entry->is_dir) {
		ape_fs_rmdir_r(fullpath);
	} else {
		ape_fs_remove(fullpath);
	}

	APEBUILD_FREE(fullpath);
}
APEBUILD_DEF int ape_fs_rmdir_r(const char *path) {
	ape_fs_iterdir(path, ape_fs_rmdir_r_callback, NULL);
	return ape_fs_rmdir(path);
}

APEBUILD_DEF int ape_fs_exists(const char *path) {
	struct stat st;
	return stat(path, &st) == 0 ? APEBUILD_TRUE : APEBUILD_FALSE;
}

APEBUILD_DEF int ape_fs_is_file(const char *path) {
	struct stat st;
	if (stat(path, &st) != 0) return APEBUILD_FALSE;
	return S_ISREG(st.st_mode) ? APEBUILD_TRUE : APEBUILD_FALSE;
}

APEBUILD_DEF int ape_fs_is_dir(const char *path) {
	struct stat st;
	if (stat(path, &st) != 0) return APEBUILD_FALSE;
	return S_ISDIR(st.st_mode) ? APEBUILD_TRUE : APEBUILD_FALSE;
}

APEBUILD_DEF int ape_fs_remove(const char *path) { return unlink(path) == 0 ? APEBUILD_TRUE : APEBUILD_FALSE; }

/* ============================================================================
 * Directory Iteration
 * ============================================================================ */

APEBUILD_DEF ApeDir *ape_fs_opendir(const char *path) {
	DIR *dir = opendir(path);
	if (!dir) return NULL;

	ApeDir *ape_dir = (ApeDir *)APEBUILD_MALLOC(sizeof(ApeDir));
	ape_dir->dir = dir;
	ape_dir->path = ape_str_dup(path);
	return ape_dir;
}

APEBUILD_DEF ApeDirEntry *ape_fs_readdir(ApeDir *dir) {
	if (!dir || !dir->dir) return NULL;

	struct dirent *entry;
	while ((entry = readdir(dir->dir)) != NULL) {
		/* Skip . and .. */
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) { continue; }

		ApeDirEntry *result = (ApeDirEntry *)APEBUILD_MALLOC(sizeof(ApeDirEntry));
		result->name = ape_str_dup(entry->d_name);

		/* Get file type */
		char *fullpath = ape_fs_join(dir->path, entry->d_name);
		struct stat st;
		if (lstat(fullpath, &st) == 0) {
			result->is_dir = S_ISDIR(st.st_mode) ? 1 : 0;
			result->is_file = S_ISREG(st.st_mode) ? 1 : 0;
			result->is_symlink = S_ISLNK(st.st_mode) ? 1 : 0;
		} else {
			result->is_dir = 0;
			result->is_file = 0;
			result->is_symlink = 0;
		}
		APEBUILD_FREE(fullpath);

		return result;
	}

	return NULL;
}

APEBUILD_DEF void ape_fs_closedir(ApeDir *dir) {
	if (!dir) return;
	if (dir->dir) closedir(dir->dir);
	APEBUILD_FREE(dir->path);
	APEBUILD_FREE(dir);
}

APEBUILD_DEF void ape_fs_direntry_free(ApeDirEntry *entry) {
	if (!entry) return;
	APEBUILD_FREE(entry->name);
	APEBUILD_FREE(entry);
}

APEBUILD_DEF int ape_fs_iterdir(const char *path, ApeDirCallback callback, void *userdata) {
	ApeDir *dir = ape_fs_opendir(path);
	if (!dir) return APEBUILD_FALSE;

	ApeDirEntry *entry;
	while ((entry = ape_fs_readdir(dir)) != NULL) {
		callback(path, entry, userdata);
		ape_fs_direntry_free(entry);
	}

	ape_fs_closedir(dir);
	return APEBUILD_TRUE;
}

APEBUILD_PRIVATE int ape_fs_iterdir_r_helper(const char *path, ApeDirCallback callback, void *userdata) {
	ApeDir *dir = ape_fs_opendir(path);
	if (!dir) return APEBUILD_FALSE;

	ApeDirEntry *entry;
	while ((entry = ape_fs_readdir(dir)) != NULL) {
		callback(path, entry, userdata);

		if (entry->is_dir && !entry->is_symlink) {
			char *subpath = ape_fs_join(path, entry->name);
			ape_fs_iterdir_r_helper(subpath, callback, userdata);
			APEBUILD_FREE(subpath);
		}

		ape_fs_direntry_free(entry);
	}

	ape_fs_closedir(dir);
	return APEBUILD_TRUE;
}

APEBUILD_DEF int ape_fs_iterdir_r(const char *path, ApeDirCallback callback, void *userdata) {
	return ape_fs_iterdir_r_helper(path, callback, userdata);
}

typedef struct {
	const char *pattern;
	const char *base_path;
	ApeStrList *results;
} GlobContext;

APEBUILD_PRIVATE void ape_fs_glob_callback(const char *path, const ApeDirEntry *entry, void *userdata) {
	GlobContext *ctx = (GlobContext *)userdata;
	char *fullpath = ape_fs_join(path, entry->name);

	/* Get path relative to base for matching */
	const char *rel_path = fullpath;
	size_t base_len = strlen(ctx->base_path);
	if (strncmp(fullpath, ctx->base_path, base_len) == 0) {
		rel_path = fullpath + base_len;
		if (*rel_path == '/') rel_path++;
	}

	if (fnmatch(ctx->pattern, rel_path, FNM_PATHNAME) == 0) {
		ape_sl_append(ctx->results, fullpath);
	} else {
		APEBUILD_FREE(fullpath);
	}
}

APEBUILD_DEF ApeStrList ape_fs_glob(const char *pattern) {
	ApeStrList results = ape_sl_new();
	if (!pattern) return results;

	/* Find base directory (everything before first wildcard) */
	char *base_path = ape_str_dup(pattern);
	char *wildcard = strpbrk(base_path, "*?[");
	if (wildcard) {
		/* Find last slash before wildcard */
		char *last_slash = wildcard;
		while (last_slash > base_path && *last_slash != '/') last_slash--;
		if (*last_slash == '/') {
			*last_slash = '\0';
		} else {
			APEBUILD_FREE(base_path);
			base_path = ape_str_dup(".");
		}
	}

	if (!ape_fs_is_dir(base_path)) {
		APEBUILD_FREE(base_path);
		return results;
	}

	GlobContext ctx = { .pattern = pattern, .base_path = base_path, .results = &results };

	ape_fs_iterdir_r(base_path, ape_fs_glob_callback, &ctx);

	APEBUILD_FREE(base_path);
	return results;
}

/* ============================================================================
 * File Metadata
 * ============================================================================ */

APEBUILD_DEF int ape_fs_stat(const char *path, ApeFileStat *out) {
	struct stat st;
	if (stat(path, &st) != 0) return APEBUILD_FALSE;

	out->size = st.st_size;
	out->mtime = st.st_mtime;
	out->atime = st.st_atime;
	out->ctime = st.st_ctime;
	out->is_dir = S_ISDIR(st.st_mode) ? 1 : 0;
	out->is_file = S_ISREG(st.st_mode) ? 1 : 0;
	out->is_symlink = S_ISLNK(st.st_mode) ? 1 : 0;
	out->mode = st.st_mode & 0777;

	return APEBUILD_TRUE;
}

APEBUILD_DEF time_t ape_fs_mtime(const char *path) {
	struct stat st;
	if (stat(path, &st) != 0) return 0;
	return st.st_mtime;
}

APEBUILD_DEF size_t ape_fs_size(const char *path) {
	struct stat st;
	if (stat(path, &st) != 0) return 0;
	return st.st_size;
}

APEBUILD_DEF int ape_fs_is_newer(const char *a, const char *b) {
	time_t mtime_a = ape_fs_mtime(a);
	time_t mtime_b = ape_fs_mtime(b);
	return mtime_a > mtime_b ? APEBUILD_TRUE : APEBUILD_FALSE;
}

APEBUILD_DEF int ape_fs_needs_rebuild(const char *output, const char **inputs, size_t input_count) {
	if (!ape_fs_exists(output)) return APEBUILD_TRUE;

	time_t output_mtime = ape_fs_mtime(output);

	for (size_t i = 0; i < input_count; i++) {
		time_t input_mtime = ape_fs_mtime(inputs[i]);
		if (input_mtime > output_mtime) return APEBUILD_TRUE;
	}

	return APEBUILD_FALSE;
}

APEBUILD_DEF int ape_fs_needs_rebuild1(const char *output, const char *input) { return ape_fs_needs_rebuild(output, &input, 1); }

/* ============================================================================
 * Path Manipulation
 * ============================================================================ */

APEBUILD_DEF char *ape_fs_join(const char *a, const char *b) {
	if (!a || !*a) return ape_str_dup(b);
	if (!b || !*b) return ape_str_dup(a);

	size_t len_a = strlen(a);
	int has_slash = (a[len_a - 1] == '/');
	int b_has_slash = (b[0] == '/');

	ApeStrBuilder sb = ape_sb_new();
	ape_sb_append_str(&sb, a);

	if (!has_slash && !b_has_slash) {
		ape_sb_append_char(&sb, '/');
	} else if (has_slash && b_has_slash) {
		b++; /* Skip leading slash of b */
	}

	ape_sb_append_str(&sb, b);
	char *result = ape_sb_to_str_dup(&sb);
	ape_sb_free(&sb);
	return result;
}

APEBUILD_DEF char *ape_fs_dirname(const char *path) {
	if (!path) return NULL;

	int last_slash = ape_str_rfind_char(path, '/');
	if (last_slash < 0) return ape_str_dup(".");
	if (last_slash == 0) return ape_str_dup("/");

	return ape_str_ndup(path, last_slash);
}

APEBUILD_DEF char *ape_fs_basename(const char *path) {
	if (!path) return NULL;

	int last_slash = ape_str_rfind_char(path, '/');
	if (last_slash < 0) return ape_str_dup(path);

	return ape_str_dup(path + last_slash + 1);
}

APEBUILD_DEF char *ape_fs_extension(const char *path) {
	if (!path) return NULL;

	char *basename = ape_fs_basename(path);
	int dot = ape_str_rfind_char(basename, '.');
	if (dot <= 0) { /* No extension or hidden file */
		APEBUILD_FREE(basename);
		return ape_str_dup("");
	}

	char *ext = ape_str_dup(basename + dot);
	APEBUILD_FREE(basename);
	return ext;
}

APEBUILD_DEF char *ape_fs_stem(const char *path) {
	if (!path) return NULL;

	char *basename = ape_fs_basename(path);
	int dot = ape_str_rfind_char(basename, '.');
	if (dot <= 0) { /* No extension or hidden file */
		return basename;
	}

	char *stem = ape_str_ndup(basename, dot);
	APEBUILD_FREE(basename);
	return stem;
}

APEBUILD_DEF char *ape_fs_change_extension(const char *path, const char *new_ext) {
	if (!path) return NULL;

	char *dir = ape_fs_dirname(path);
	char *stem = ape_fs_stem(path);

	ApeStrBuilder sb = ape_sb_new();
	if (strcmp(dir, ".") != 0) {
		ape_sb_append_str(&sb, dir);
		ape_sb_append_char(&sb, '/');
	}
	ape_sb_append_str(&sb, stem);
	if (new_ext && *new_ext) {
		if (*new_ext != '.') ape_sb_append_char(&sb, '.');
		ape_sb_append_str(&sb, new_ext);
	}

	APEBUILD_FREE(dir);
	APEBUILD_FREE(stem);

	char *result = ape_sb_to_str_dup(&sb);
	ape_sb_free(&sb);
	return result;
}

APEBUILD_DEF char *ape_fs_normalize(const char *path) {
	if (!path) return NULL;

	ApeStrList parts = ape_str_split(path, "/");
	ApeStrList result_parts = ape_sl_new();

	int is_absolute = (path[0] == '/');

	for (size_t i = 0; i < parts.count; i++) {
		const char *part = parts.items[i];

		if (strcmp(part, ".") == 0 || *part == '\0') {
			continue;
		} else if (strcmp(part, "..") == 0) {
			if (result_parts.count > 0 && strcmp(ape_sl_get(&result_parts, result_parts.count - 1), "..") != 0) {
				ape_sl_remove(&result_parts, result_parts.count - 1);
			} else if (!is_absolute) {
				ape_sl_append_dup(&result_parts, "..");
			}
		} else {
			ape_sl_append_dup(&result_parts, part);
		}
	}

	char *joined = ape_sl_join(&result_parts, "/");
	ApeStrBuilder sb = ape_sb_new();
	if (is_absolute) ape_sb_append_char(&sb, '/');
	ape_sb_append_str(&sb, joined);

	if (sb.count == 0) ape_sb_append_str(&sb, ".");

	APEBUILD_FREE(joined);
	ape_sl_free(&parts);
	ape_sl_free(&result_parts);

	char *result = ape_sb_to_str_dup(&sb);
	ape_sb_free(&sb);
	return result;
}

APEBUILD_DEF char *ape_fs_absolute(const char *path) {
	if (!path) return NULL;

	if (ape_fs_is_absolute(path)) { return ape_fs_normalize(path); }

	char *cwd = ape_fs_cwd();
	char *joined = ape_fs_join(cwd, path);
	char *result = ape_fs_normalize(joined);

	APEBUILD_FREE(cwd);
	APEBUILD_FREE(joined);
	return result;
}

APEBUILD_DEF char *ape_fs_relative(const char *from, const char *to) {
	if (!from || !to) return NULL;

	char *abs_from = ape_fs_absolute(from);
	char *abs_to = ape_fs_absolute(to);

	ApeStrList from_parts = ape_str_split(abs_from, "/");
	ApeStrList to_parts = ape_str_split(abs_to, "/");

	/* Find common prefix */
	size_t common = 0;
	while (common < from_parts.count && common < to_parts.count && ape_str_eq(from_parts.items[common], to_parts.items[common])) {
		common++;
	}

	ApeStrBuilder sb = ape_sb_new();

	/* Add ".." for each remaining from part */
	for (size_t i = common; i < from_parts.count; i++) {
		if (sb.count > 0) ape_sb_append_char(&sb, '/');
		ape_sb_append_str(&sb, "..");
	}

	/* Add remaining to parts */
	for (size_t i = common; i < to_parts.count; i++) {
		if (sb.count > 0) ape_sb_append_char(&sb, '/');
		ape_sb_append_str(&sb, to_parts.items[i]);
	}

	if (sb.count == 0) ape_sb_append_str(&sb, ".");

	APEBUILD_FREE(abs_from);
	APEBUILD_FREE(abs_to);
	ape_sl_free(&from_parts);
	ape_sl_free(&to_parts);

	char *result = ape_sb_to_str_dup(&sb);
	ape_sb_free(&sb);
	return result;
}

APEBUILD_DEF int ape_fs_is_absolute(const char *path) { return path && path[0] == '/' ? APEBUILD_TRUE : APEBUILD_FALSE; }

/* ============================================================================
 * Temporary Files
 * ============================================================================ */

APEBUILD_DEF const char *ape_fs_temp_dir(void) {
	const char *tmp = getenv("TMPDIR");
	if (tmp) return tmp;
	tmp = getenv("TMP");
	if (tmp) return tmp;
	tmp = getenv("TEMP");
	if (tmp) return tmp;
	return "/tmp";
}

APEBUILD_DEF char *ape_fs_temp_file(const char *prefix) {
	const char *tmpdir = ape_fs_temp_dir();
	if (!prefix) prefix = "ape";

	ApeStrBuilder sb = ape_sb_new();
	ape_sb_append_str(&sb, tmpdir);
	ape_sb_append_char(&sb, '/');
	ape_sb_append_str(&sb, prefix);
	ape_sb_append_str(&sb, "XXXXXX");

	char *template = ape_sb_to_str_dup(&sb);
	ape_sb_free(&sb);

	int fd = mkstemp(template);
	if (fd < 0) {
		APEBUILD_FREE(template);
		return NULL;
	}
	close(fd);

	return template;
}

APEBUILD_DEF char *ape_fs_temp_mkdir(const char *prefix) {
	const char *tmpdir = ape_fs_temp_dir();
	if (!prefix) prefix = "ape";

	ApeStrBuilder sb = ape_sb_new();
	ape_sb_append_str(&sb, tmpdir);
	ape_sb_append_char(&sb, '/');
	ape_sb_append_str(&sb, prefix);
	ape_sb_append_str(&sb, "XXXXXX");

	char *template = ape_sb_to_str_dup(&sb);
	ape_sb_free(&sb);

	if (mkdtemp(template) == NULL) {
		APEBUILD_FREE(template);
		return NULL;
	}

	return template;
}

/* ============================================================================
 * Utility
 * ============================================================================ */

APEBUILD_DEF char *ape_fs_cwd(void) {
	char *buf = (char *)APEBUILD_MALLOC(PATH_MAX);
	if (getcwd(buf, PATH_MAX) == NULL) {
		APEBUILD_FREE(buf);
		return NULL;
	}
	return buf;
}

APEBUILD_DEF int ape_fs_chdir(const char *path) { return chdir(path) == 0 ? APEBUILD_TRUE : APEBUILD_FALSE; }

APEBUILD_DEF char *ape_fs_home(void) {
	const char *home = getenv("HOME");
	if (home) return ape_str_dup(home);
	return NULL;
}
/* END ape_fs.c */

/* BEGIN ape_log.c */
/*
 * ape_log.c - Logging module implementation
 */

/* ============================================================================
 * Global State
 * ============================================================================ */

APEBUILD_PRIVATE ApeLogConfig ape_log_config = { .level = APE_LOG_INFO,
						 .use_colors = 1,
						 .show_timestamps = 0,
						 .show_level = 1,
						 .show_file = 0,
						 .output = NULL, /* Will be set to stderr on first use */
						 .file_output = NULL,
						 .prefix = NULL };

APEBUILD_PRIVATE int ape_log_initialized = 0;

/* ANSI color codes */
APEBUILD_PRIVATE const char *ape_log_colors[] = {
	"\033[90m",   /* TRACE - gray */
	"\033[36m",   /* DEBUG - cyan */
	"\033[32m",   /* INFO  - green */
	"\033[33m",   /* WARN  - yellow */
	"\033[31m",   /* ERROR - red */
	"\033[35;1m", /* FATAL - bold magenta */
};

APEBUILD_PRIVATE const char *ape_log_reset = "\033[0m";

APEBUILD_PRIVATE const char *ape_log_level_names[] = { "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL", "OFF" };

/* ============================================================================
 * Initialization
 * ============================================================================ */

APEBUILD_DEF void ape_log_init(void) {
	if (ape_log_initialized) return;

	ape_log_config.output = stderr;

	/* Auto-detect color support */
	if (isatty(STDERR_FILENO)) {
		const char *term = getenv("TERM");
		if (term && strcmp(term, "dumb") != 0) {
			ape_log_config.use_colors = 1;
		} else {
			ape_log_config.use_colors = 0;
		}
	} else {
		ape_log_config.use_colors = 0;
	}

	ape_log_initialized = 1;
}

APEBUILD_DEF void ape_log_shutdown(void) {
	if (ape_log_config.file_output) {
		fclose(ape_log_config.file_output);
		ape_log_config.file_output = NULL;
	}
	ape_log_initialized = 0;
}

/* ============================================================================
 * Configuration
 * ============================================================================ */

APEBUILD_DEF void ape_log_set_level(ApeLogLevel level) { ape_log_config.level = level; }

APEBUILD_DEF ApeLogLevel ape_log_get_level(void) { return ape_log_config.level; }

APEBUILD_DEF void ape_log_set_output(FILE *fp) { ape_log_config.output = fp; }

APEBUILD_DEF int ape_log_set_file(const char *path) {
	if (ape_log_config.file_output) {
		fclose(ape_log_config.file_output);
		ape_log_config.file_output = NULL;
	}

	if (!path) return APEBUILD_TRUE;

	ape_log_config.file_output = fopen(path, "a");
	return ape_log_config.file_output != NULL ? APEBUILD_TRUE : APEBUILD_FALSE;
}

APEBUILD_DEF void ape_log_set_colors(int enabled) { ape_log_config.use_colors = enabled; }

APEBUILD_DEF void ape_log_set_timestamps(int enabled) { ape_log_config.show_timestamps = enabled; }

APEBUILD_DEF void ape_log_set_show_level(int enabled) { ape_log_config.show_level = enabled; }

APEBUILD_DEF void ape_log_set_show_file(int enabled) { ape_log_config.show_file = enabled; }

APEBUILD_DEF void ape_log_set_prefix(const char *prefix) { ape_log_config.prefix = prefix; }

APEBUILD_DEF void ape_log_set_quiet(int quiet) {
	if (quiet) {
		ape_log_config.output = NULL;
	} else {
		ape_log_config.output = stderr;
	}
}

/* ============================================================================
 * Logging Functions
 * ============================================================================ */

APEBUILD_PRIVATE void ape_log_write_to_file(FILE *fp, ApeLogLevel level, const char *file, int line, const char *fmt, va_list args,
					    int use_colors) {
	/* Timestamp */
	if (ape_log_config.show_timestamps) {
		time_t now = time(NULL);
		struct tm *tm_info = localtime(&now);
		char time_buf[32];
		strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", tm_info);
		fprintf(fp, "[%s] ", time_buf);
	}

	/* Prefix */
	if (ape_log_config.prefix) { fprintf(fp, "%s", ape_log_config.prefix); }

	/* Level */
	if (ape_log_config.show_level) {
		if (use_colors && level < APE_LOG_OFF) {
			fprintf(fp, "%s%-5s%s ", ape_log_colors[level], ape_log_level_names[level], ape_log_reset);
		} else {
			fprintf(fp, "%-5s ", ape_log_level_names[level]);
		}
	}

	/* File:line */
	if (ape_log_config.show_file && file) {
		/* Extract just the filename */
		const char *filename = file;
		const char *slash = strrchr(file, '/');
		if (slash) filename = slash + 1;
		fprintf(fp, "%s:%d: ", filename, line);
	}

	/* Message */
	vfprintf(fp, fmt, args);
	fprintf(fp, "\n");
}

APEBUILD_DEF void ape_log_writev(ApeLogLevel level, const char *file, int line, const char *fmt, va_list args) {
	if (!ape_log_initialized) ape_log_init();

	if (level < ape_log_config.level) return;

	/* Write to main output */
	if (ape_log_config.output) {
		va_list args_copy;
		va_copy(args_copy, args);
		ape_log_write_to_file(ape_log_config.output, level, file, line, fmt, args_copy, ape_log_config.use_colors);
		va_end(args_copy);
	}

	/* Write to file output (no colors) */
	if (ape_log_config.file_output) {
		va_list args_copy;
		va_copy(args_copy, args);
		ape_log_write_to_file(ape_log_config.file_output, level, file, line, fmt, args_copy, 0);
		va_end(args_copy);
	}
}

APEBUILD_DEF void ape_log_write(ApeLogLevel level, const char *file, int line, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	ape_log_writev(level, file, line, fmt, args);
	va_end(args);
}

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

APEBUILD_DEF const char *ape_log_level_name(ApeLogLevel level) {
	if (level < 0 || level > APE_LOG_OFF) return "UNKNOWN";
	return ape_log_level_names[level];
}

APEBUILD_DEF int ape_log_level_from_name(const char *name, ApeLogLevel *out) {
	if (!name || !out) return APEBUILD_FALSE;

	for (int i = 0; i <= APE_LOG_OFF; i++) {
		if (ape_str_eq_nocase(name, ape_log_level_names[i])) {
			*out = (ApeLogLevel)i;
			return APEBUILD_TRUE;
		}
	}

	return APEBUILD_FALSE;
}

APEBUILD_DEF void ape_log_flush(void) {
	if (ape_log_config.output) { fflush(ape_log_config.output); }
	if (ape_log_config.file_output) { fflush(ape_log_config.file_output); }
}

/* ============================================================================
 * Build System Specific Logging
 * ============================================================================ */

APEBUILD_PRIVATE void ape_log_build_msg(const char *prefix, const char *color, const char *fmt, va_list args) {
	if (!ape_log_initialized) ape_log_init();

	FILE *fp = ape_log_config.output;
	if (!fp) return;

	if (ape_log_config.use_colors && color) {
		fprintf(fp, "%s%s%s ", color, prefix, ape_log_reset);
	} else {
		fprintf(fp, "%s ", prefix);
	}

	vfprintf(fp, fmt, args);
	fprintf(fp, "\n");
}

APEBUILD_DEF void ape_log_cmd(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	ape_log_build_msg("CMD:", "\033[36m", fmt, args); /* cyan */
	va_end(args);
}

APEBUILD_DEF void ape_log_build(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	ape_log_build_msg("BUILD:", "\033[34m", fmt, args); /* blue */
	va_end(args);
}

APEBUILD_DEF void ape_log_link(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	ape_log_build_msg("LINK:", "\033[35m", fmt, args); /* magenta */
	va_end(args);
}

APEBUILD_DEF void ape_log_success(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	ape_log_build_msg("OK:", "\033[32m", fmt, args); /* green */
	va_end(args);
}

APEBUILD_DEF void ape_log_failure(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	ape_log_build_msg("FAIL:", "\033[31m", fmt, args); /* red */
	va_end(args);
}
/* END ape_log.c */

/* BEGIN ape_str.c */
/*
 * ape_str.c - String manipulation module implementation
 */

/* ============================================================================
 * String Builder Implementation
 * ============================================================================ */

APEBUILD_DEF void ape_sb_init(ApeStrBuilder *sb) {
	sb->capacity = 0;
	sb->count = 0;
	sb->items = NULL;
}

APEBUILD_DEF ApeStrBuilder ape_sb_new(void) {
	ApeStrBuilder sb;
	ape_sb_init(&sb);
	return sb;
}

APEBUILD_DEF ApeStrBuilder ape_sb_new_cap(size_t cap) {
	ApeStrBuilder sb;
	sb.capacity = cap;
	sb.count = 0;
	sb.items = (char *)APEBUILD_MALLOC(cap);
	return sb;
}

APEBUILD_DEF void ape_sb_free(ApeStrBuilder *sb) {
	APEBUILD_FREE(sb->items);
	ape_sb_init(sb);
}

APEBUILD_DEF void ape_sb_clear(ApeStrBuilder *sb) { sb->count = 0; }

APEBUILD_DEF void ape_sb_append_char(ApeStrBuilder *sb, char c) { ape_da_append(sb, c); }

APEBUILD_DEF void ape_sb_append_str(ApeStrBuilder *sb, const char *str) {
	if (!str) return;
	size_t len = strlen(str);
	ape_da_append_many(sb, str, len);
}

APEBUILD_DEF void ape_sb_append_strn(ApeStrBuilder *sb, const char *str, size_t n) {
	if (!str) return;
	ape_da_append_many(sb, str, n);
}

APEBUILD_DEF void ape_sb_append_sb(ApeStrBuilder *sb, const ApeStrBuilder *other) {
	if (!other || !other->items) return;
	ape_da_append_many(sb, other->items, other->count);
}

APEBUILD_DEF void ape_sb_append_fmtv(ApeStrBuilder *sb, const char *fmt, va_list args) {
	va_list args_copy;
	va_copy(args_copy, args);
	int needed = vsnprintf(NULL, 0, fmt, args_copy);
	va_end(args_copy);

	if (needed < 0) return;

	ape_da_reserve(sb, sb->count + needed + 1);
	vsnprintf(sb->items + sb->count, needed + 1, fmt, args);
	sb->count += needed;
}

APEBUILD_DEF void ape_sb_append_fmt(ApeStrBuilder *sb, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	ape_sb_append_fmtv(sb, fmt, args);
	va_end(args);
}

APEBUILD_DEF void ape_sb_prepend_str(ApeStrBuilder *sb, const char *str) {
	if (!str) return;
	size_t len = strlen(str);
	if (sb->count + len >= sb->capacity) {
		size_t new_cap = sb->capacity == 0 ? APEBUILD_INIT_CAP : sb->capacity;
		while (sb->count + len >= new_cap) new_cap *= 2;
		sb->items = (char *)APEBUILD_REALLOC(sb->items, new_cap);
		sb->capacity = new_cap;
	}
	memmove(sb->items + len, sb->items, sb->count);
	memcpy(sb->items, str, len);
	sb->count += len;
}

APEBUILD_DEF void ape_sb_insert(ApeStrBuilder *sb, size_t pos, const char *str) {
	if (!str) return;
	if (pos > sb->count) pos = sb->count;
	size_t len = strlen(str);
	if (sb->count + len >= sb->capacity) {
		size_t new_cap = sb->capacity == 0 ? APEBUILD_INIT_CAP : sb->capacity;
		while (sb->count + len >= new_cap) new_cap *= 2;
		sb->items = (char *)APEBUILD_REALLOC(sb->items, new_cap);
		sb->capacity = new_cap;
	}
	memmove(sb->items + pos + len, sb->items + pos, sb->count - pos);
	memcpy(sb->items + pos, str, len);
	sb->count += len;
}

APEBUILD_DEF const char *ape_sb_to_str(ApeStrBuilder *sb) {
	ape_da_append(sb, '\0');
	sb->count--; /* Don't count null terminator in length */
	return sb->items;
}

APEBUILD_DEF char *ape_sb_to_str_dup(const ApeStrBuilder *sb) {
	char *result = (char *)APEBUILD_MALLOC(sb->count + 1);
	if (sb->items) memcpy(result, sb->items, sb->count);
	result[sb->count] = '\0';
	return result;
}

APEBUILD_DEF size_t ape_sb_len(const ApeStrBuilder *sb) { return sb->count; }

APEBUILD_DEF size_t ape_sb_capacity(const ApeStrBuilder *sb) { return sb->capacity; }

APEBUILD_DEF void ape_sb_reserve(ApeStrBuilder *sb, size_t cap) { ape_da_reserve(sb, cap); }

APEBUILD_DEF void ape_sb_shrink(ApeStrBuilder *sb) { ape_da_shrink(sb); }

/* ============================================================================
 * String List Implementation
 * ============================================================================ */

APEBUILD_DEF void ape_sl_init(ApeStrList *sl) {
	sl->capacity = 0;
	sl->count = 0;
	sl->items = NULL;
}

APEBUILD_DEF ApeStrList ape_sl_new(void) {
	ApeStrList sl;
	ape_sl_init(&sl);
	return sl;
}

APEBUILD_DEF void ape_sl_free(ApeStrList *sl) {
	for (size_t i = 0; i < sl->count; i++) { APEBUILD_FREE(sl->items[i]); }
	APEBUILD_FREE(sl->items);
	ape_sl_init(sl);
}

APEBUILD_DEF void ape_sl_free_shallow(ApeStrList *sl) {
	APEBUILD_FREE(sl->items);
	ape_sl_init(sl);
}

APEBUILD_DEF void ape_sl_clear(ApeStrList *sl) {
	for (size_t i = 0; i < sl->count; i++) { APEBUILD_FREE(sl->items[i]); }
	sl->count = 0;
}

APEBUILD_DEF void ape_sl_append(ApeStrList *sl, char *str) { ape_da_append(sl, str); }

APEBUILD_DEF void ape_sl_append_dup(ApeStrList *sl, const char *str) { ape_da_append(sl, ape_str_dup(str)); }

APEBUILD_DEF void ape_sl_append_many(ApeStrList *sl, char **strs, size_t n) { ape_da_append_many(sl, strs, n); }

APEBUILD_DEF void ape_sl_prepend(ApeStrList *sl, char *str) { ape_da_prepend(sl, str); }

APEBUILD_DEF void ape_sl_insert(ApeStrList *sl, size_t index, char *str) { ape_da_insert(sl, index, str); }

APEBUILD_DEF void ape_sl_remove(ApeStrList *sl, size_t index) {
	if (index < sl->count) {
		APEBUILD_FREE(sl->items[index]);
		ape_da_remove(sl, index);
	}
}

APEBUILD_DEF char *ape_sl_get(const ApeStrList *sl, size_t index) {
	if (index >= sl->count) return NULL;
	return sl->items[index];
}

APEBUILD_DEF size_t ape_sl_len(const ApeStrList *sl) { return sl->count; }

APEBUILD_DEF int ape_sl_contains(const ApeStrList *sl, const char *str) { return ape_sl_index_of(sl, str) >= 0; }

APEBUILD_DEF int ape_sl_index_of(const ApeStrList *sl, const char *str) {
	for (size_t i = 0; i < sl->count; i++) {
		if (ape_str_eq(sl->items[i], str)) return (int)i;
	}
	return -1;
}

APEBUILD_DEF char *ape_sl_join(const ApeStrList *sl, const char *sep) {
	if (sl->count == 0) return ape_str_dup("");

	ApeStrBuilder sb = ape_sb_new();
	for (size_t i = 0; i < sl->count; i++) {
		if (i > 0 && sep) ape_sb_append_str(&sb, sep);
		ape_sb_append_str(&sb, sl->items[i]);
	}
	char *result = ape_sb_to_str_dup(&sb);
	ape_sb_free(&sb);
	return result;
}

APEBUILD_DEF ApeStrList ape_sl_clone(const ApeStrList *sl) {
	ApeStrList result = ape_sl_new();
	for (size_t i = 0; i < sl->count; i++) { ape_sl_append_dup(&result, sl->items[i]); }
	return result;
}

/* ============================================================================
 * String Utilities Implementation
 * ============================================================================ */

APEBUILD_DEF char *ape_str_dup(const char *str) {
	if (!str) return NULL;
	size_t len = strlen(str);
	char *result = (char *)APEBUILD_MALLOC(len + 1);
	memcpy(result, str, len + 1);
	return result;
}

APEBUILD_DEF char *ape_str_ndup(const char *str, size_t n) {
	if (!str) return NULL;
	size_t len = strlen(str);
	if (n > len) n = len;
	char *result = (char *)APEBUILD_MALLOC(n + 1);
	memcpy(result, str, n);
	result[n] = '\0';
	return result;
}

APEBUILD_DEF char *ape_str_concat(const char *a, const char *b) {
	if (!a) a = "";
	if (!b) b = "";
	size_t len_a = strlen(a);
	size_t len_b = strlen(b);
	char *result = (char *)APEBUILD_MALLOC(len_a + len_b + 1);
	memcpy(result, a, len_a);
	memcpy(result + len_a, b, len_b + 1);
	return result;
}

APEBUILD_DEF char *ape_str_join(const char **strs, size_t count, const char *sep) {
	if (count == 0) return ape_str_dup("");

	ApeStrBuilder sb = ape_sb_new();
	for (size_t i = 0; i < count; i++) {
		if (i > 0 && sep) ape_sb_append_str(&sb, sep);
		ape_sb_append_str(&sb, strs[i]);
	}
	char *result = ape_sb_to_str_dup(&sb);
	ape_sb_free(&sb);
	return result;
}

APEBUILD_DEF ApeStrList ape_str_split(const char *str, const char *delim) {
	ApeStrList result = ape_sl_new();
	if (!str || !delim) return result;

	size_t delim_len = strlen(delim);
	if (delim_len == 0) {
		ape_sl_append_dup(&result, str);
		return result;
	}

	const char *start = str;
	const char *found;
	while ((found = strstr(start, delim)) != NULL) {
		ape_sl_append(&result, ape_str_ndup(start, found - start));
		start = found + delim_len;
	}
	ape_sl_append_dup(&result, start);
	return result;
}

APEBUILD_DEF ApeStrList ape_str_split_lines(const char *str) {
	ApeStrList result = ape_sl_new();
	if (!str) return result;

	const char *start = str;
	const char *p = str;
	while (*p) {
		if (*p == '\n') {
			size_t len = p - start;
			if (len > 0 && start[len - 1] == '\r') len--;
			ape_sl_append(&result, ape_str_ndup(start, len));
			start = p + 1;
		}
		p++;
	}
	if (start < p) { ape_sl_append_dup(&result, start); }
	return result;
}

APEBUILD_DEF int ape_str_eq(const char *a, const char *b) {
	if (a == b) return 1;
	if (!a || !b) return 0;
	return strcmp(a, b) == 0;
}

APEBUILD_DEF int ape_str_eq_nocase(const char *a, const char *b) {
	if (a == b) return 1;
	if (!a || !b) return 0;
	while (*a && *b) {
		if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return 0;
		a++;
		b++;
	}
	return *a == *b;
}

APEBUILD_DEF int ape_str_starts_with(const char *str, const char *prefix) {
	if (!str || !prefix) return 0;
	size_t len_str = strlen(str);
	size_t len_prefix = strlen(prefix);
	if (len_prefix > len_str) return 0;
	return strncmp(str, prefix, len_prefix) == 0;
}

APEBUILD_DEF int ape_str_ends_with(const char *str, const char *suffix) {
	if (!str || !suffix) return 0;
	size_t len_str = strlen(str);
	size_t len_suffix = strlen(suffix);
	if (len_suffix > len_str) return 0;
	return strcmp(str + len_str - len_suffix, suffix) == 0;
}

APEBUILD_DEF int ape_str_contains(const char *str, const char *substr) {
	if (!str || !substr) return 0;
	return strstr(str, substr) != NULL;
}

APEBUILD_DEF int ape_str_is_empty(const char *str) { return !str || *str == '\0'; }

APEBUILD_DEF char *ape_str_trim(const char *str) {
	if (!str) return NULL;
	while (*str && isspace((unsigned char)*str)) str++;
	if (*str == '\0') return ape_str_dup("");
	const char *end = str + strlen(str) - 1;
	while (end > str && isspace((unsigned char)*end)) end--;
	return ape_str_ndup(str, end - str + 1);
}

APEBUILD_DEF char *ape_str_trim_left(const char *str) {
	if (!str) return NULL;
	while (*str && isspace((unsigned char)*str)) str++;
	return ape_str_dup(str);
}

APEBUILD_DEF char *ape_str_trim_right(const char *str) {
	if (!str) return NULL;
	size_t len = strlen(str);
	if (len == 0) return ape_str_dup("");
	const char *end = str + len - 1;
	while (end >= str && isspace((unsigned char)*end)) end--;
	return ape_str_ndup(str, end - str + 1);
}

APEBUILD_DEF char *ape_str_to_lower(const char *str) {
	if (!str) return NULL;
	char *result = ape_str_dup(str);
	for (char *p = result; *p; p++) { *p = (char)tolower((unsigned char)*p); }
	return result;
}

APEBUILD_DEF char *ape_str_to_upper(const char *str) {
	if (!str) return NULL;
	char *result = ape_str_dup(str);
	for (char *p = result; *p; p++) { *p = (char)toupper((unsigned char)*p); }
	return result;
}

APEBUILD_DEF char *ape_str_replace(const char *str, const char *old, const char *new_str) {
	if (!str || !old) return ape_str_dup(str);
	if (!new_str) new_str = "";

	const char *found = strstr(str, old);
	if (!found) return ape_str_dup(str);

	size_t old_len = strlen(old);
	ApeStrBuilder sb = ape_sb_new();
	ape_sb_append_strn(&sb, str, found - str);
	ape_sb_append_str(&sb, new_str);
	ape_sb_append_str(&sb, found + old_len);
	char *result = ape_sb_to_str_dup(&sb);
	ape_sb_free(&sb);
	return result;
}

APEBUILD_DEF char *ape_str_replace_all(const char *str, const char *old, const char *new_str) {
	if (!str || !old || *old == '\0') return ape_str_dup(str);
	if (!new_str) new_str = "";

	ApeStrBuilder sb = ape_sb_new();
	size_t old_len = strlen(old);
	const char *start = str;
	const char *found;

	while ((found = strstr(start, old)) != NULL) {
		ape_sb_append_strn(&sb, start, found - start);
		ape_sb_append_str(&sb, new_str);
		start = found + old_len;
	}
	ape_sb_append_str(&sb, start);

	char *result = ape_sb_to_str_dup(&sb);
	ape_sb_free(&sb);
	return result;
}

APEBUILD_DEF char *ape_str_substr(const char *str, size_t start, size_t len) {
	if (!str) return NULL;
	size_t str_len = strlen(str);
	if (start >= str_len) return ape_str_dup("");
	if (start + len > str_len) len = str_len - start;
	return ape_str_ndup(str + start, len);
}

APEBUILD_DEF int ape_str_find(const char *str, const char *substr) {
	if (!str || !substr) return -1;
	const char *found = strstr(str, substr);
	if (!found) return -1;
	return (int)(found - str);
}

APEBUILD_DEF int ape_str_find_char(const char *str, char c) {
	if (!str) return -1;
	const char *found = strchr(str, c);
	if (!found) return -1;
	return (int)(found - str);
}

APEBUILD_DEF int ape_str_rfind(const char *str, const char *substr) {
	if (!str || !substr) return -1;
	size_t str_len = strlen(str);
	size_t substr_len = strlen(substr);
	if (substr_len > str_len) return -1;

	for (size_t i = str_len - substr_len + 1; i > 0; i--) {
		if (strncmp(str + i - 1, substr, substr_len) == 0) { return (int)(i - 1); }
	}
	return -1;
}

APEBUILD_DEF int ape_str_rfind_char(const char *str, char c) {
	if (!str) return -1;
	const char *found = strrchr(str, c);
	if (!found) return -1;
	return (int)(found - str);
}

APEBUILD_DEF int ape_str_to_int(const char *str, int *out) {
	if (!str || !out) return 0;
	char *end;
	long val = strtol(str, &end, 10);
	if (end == str || *end != '\0') return 0;
	*out = (int)val;
	return 1;
}

APEBUILD_DEF int ape_str_to_long(const char *str, long *out) {
	if (!str || !out) return 0;
	char *end;
	long val = strtol(str, &end, 10);
	if (end == str || *end != '\0') return 0;
	*out = val;
	return 1;
}

APEBUILD_DEF int ape_str_to_float(const char *str, float *out) {
	if (!str || !out) return 0;
	char *end;
	float val = strtof(str, &end);
	if (end == str || *end != '\0') return 0;
	*out = val;
	return 1;
}

APEBUILD_DEF int ape_str_to_double(const char *str, double *out) {
	if (!str || !out) return 0;
	char *end;
	double val = strtod(str, &end);
	if (end == str || *end != '\0') return 0;
	*out = val;
	return 1;
}

APEBUILD_DEF char *ape_str_from_int(int val) {
	char buf[32];
	snprintf(buf, sizeof(buf), "%d", val);
	return ape_str_dup(buf);
}

APEBUILD_DEF char *ape_str_from_long(long val) {
	char buf[32];
	snprintf(buf, sizeof(buf), "%ld", val);
	return ape_str_dup(buf);
}

APEBUILD_DEF char *ape_str_from_float(float val) {
	char buf[64];
	snprintf(buf, sizeof(buf), "%g", val);
	return ape_str_dup(buf);
}
/* END ape_str.c */

#endif

#endif
