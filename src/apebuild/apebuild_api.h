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
			if ((da)->capacity == 0)                                                            \
				(da)->capacity = APEBUILD_INIT_CAP;                                         \
			while ((da)->count + (n) > (da)->capacity)                                          \
				(da)->capacity *= 2;                                                        \
			(da)->items = APEBUILD_REALLOC((da)->items, (da)->capacity * sizeof(*(da)->items)); \
		}                                                                                           \
		for (size_t _i = 0; _i < (n); _i++)                                                         \
			(da)->items[(da)->count++] = (new_items)[_i];                                       \
	} while (0)

#define ape_da_prepend(da, item)                                                                            \
	do {                                                                                                \
		if ((da)->count >= (da)->capacity) {                                                        \
			(da)->capacity = (da)->capacity == 0 ? APEBUILD_INIT_CAP : (da)->capacity * 2;      \
			(da)->items = APEBUILD_REALLOC((da)->items, (da)->capacity * sizeof(*(da)->items)); \
		}                                                                                           \
		for (size_t _i = (da)->count; _i > 0; _i--)                                                 \
			(da)->items[_i] = (da)->items[_i - 1];                                              \
		(da)->items[0] = (item);                                                                    \
		(da)->count++;                                                                              \
	} while (0)

#define ape_da_insert(da, index, item)                                                                      \
	do {                                                                                                \
		if ((da)->count >= (da)->capacity) {                                                        \
			(da)->capacity = (da)->capacity == 0 ? APEBUILD_INIT_CAP : (da)->capacity * 2;      \
			(da)->items = APEBUILD_REALLOC((da)->items, (da)->capacity * sizeof(*(da)->items)); \
		}                                                                                           \
		for (size_t _i = (da)->count; _i > (index); _i--)                                           \
			(da)->items[_i] = (da)->items[_i - 1];                                              \
		(da)->items[index] = (item);                                                                \
		(da)->count++;                                                                              \
	} while (0)

#define ape_da_remove(da, index)                                      \
	do {                                                          \
		for (size_t _i = (index); _i < (da)->count - 1; _i++) \
			(da)->items[_i] = (da)->items[_i + 1];        \
		(da)->count--;                                        \
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

#define APE_REBUILD(argc, argv)                                            \
	do {                                                               \
		if (ape_self_needs_rebuild((argv)[0], __FILE__)) {         \
			return ape_self_rebuild((argc), (argv), __FILE__); \
		}                                                          \
	} while (0)

#if defined(__cplusplus)
}
#endif

#if defined(APEBUILD_STRIP_PREFIX)

#endif

#endif
