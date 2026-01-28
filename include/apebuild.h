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

/* Verbosity levels */
typedef enum {
	APE_VERBOSE_QUIET, /* No output except errors */
	APE_VERBOSE_NORMAL, /* Normal build output */
	APE_VERBOSE_VERBOSE, /* Show commands being run */
	APE_VERBOSE_DEBUG /* Show all details */
} ApeVerbosity;

/* Build target types */
typedef enum {
	APE_TARGET_EXECUTABLE,
	APE_TARGET_STATIC_LIB,
	APE_TARGET_SHARED_LIB,
	APE_TARGET_OBJECT /* Compile only, no link */
} ApeTargetType;

/* Task types */
typedef enum {
	APE_TASK_COMPILE, /* Compile source to object */
	APE_TASK_LINK, /* Link objects to target */
	APE_TASK_ARCHIVE, /* Create static library */
	APE_TASK_COMMAND, /* Run arbitrary command */
	APE_TASK_COPY, /* Copy file */
	APE_TASK_MKDIR /* Create directory */
} ApeTaskType;

/* Task status */
typedef enum {
	APE_TASK_PENDING,
	APE_TASK_RUNNING,
	APE_TASK_COMPLETED,
	APE_TASK_FAILED,
	APE_TASK_SKIPPED /* Already up-to-date */
} ApeTaskStatus;

/* Forward declarations */
typedef struct ApeToolchain ApeToolchain;
typedef struct ApeBuildCtx ApeBuildCtx;
typedef struct ApeBuilder ApeBuilder;
typedef struct ApeTask ApeTask;

/* ----------------------------------------------------------------------------
 * Toolchain - Compiler/linker configuration
 * ---------------------------------------------------------------------------- */

struct ApeToolchain {
	char *name; /* Toolchain name (e.g., "gcc", "clang") */
	char *cc; /* C compiler command */
	char *cxx; /* C++ compiler command */
	char *ld; /* Linker command */
	char *ar; /* Archiver command */
	char *obj_ext; /* Object file extension (e.g., ".o") */
	char *exe_ext; /* Executable extension (e.g., "", ".exe") */
	char *static_lib_ext; /* Static lib extension (e.g., ".a") */
	char *shared_lib_ext; /* Shared lib extension (e.g., ".so") */
	char *lib_prefix; /* Library prefix (e.g., "lib") */
	ApeStrList default_cflags;
	ApeStrList default_ldflags;
};

ApeToolchain *ape_toolchain_new(const char *name);
void ape_toolchain_free(ApeToolchain *tc);
ApeToolchain *ape_toolchain_clone(const ApeToolchain *tc);
ApeToolchain *ape_toolchain_gcc(void);
ApeToolchain *ape_toolchain_clang(void);
void ape_toolchain_set_cc(ApeToolchain *tc, const char *cc);
void ape_toolchain_set_cxx(ApeToolchain *tc, const char *cxx);
void ape_toolchain_set_ld(ApeToolchain *tc, const char *ld);
void ape_toolchain_set_ar(ApeToolchain *tc, const char *ar);
void ape_toolchain_add_cflag(ApeToolchain *tc, const char *flag);
void ape_toolchain_add_ldflag(ApeToolchain *tc, const char *flag);

/* ----------------------------------------------------------------------------
 * Task - Individual build operation
 * ---------------------------------------------------------------------------- */

/* Task dependency list */
typedef struct {
	size_t capacity;
	size_t count;
	ApeTask **items;
} ApeTaskList;

struct ApeTask {
	int id; /* Unique task ID within builder */
	ApeTaskType type;
	ApeTaskStatus status;
	char *name; /* Human-readable name */
	char *input; /* Primary input file (for compile) */
	char *output; /* Output file */
	ApeStrList inputs; /* Additional inputs (for link) */
	ApeCmd cmd; /* Command to execute */
	ApeTaskList deps; /* Tasks that must complete before this one */
	ApeProcHandle proc; /* Process handle when running */
	int exit_code; /* Exit code after completion */
	ApeBuilder *builder; /* Parent builder */
};

ApeTask *ape_task_new(ApeBuilder *builder, ApeTaskType type, const char *name);
void ape_task_free(ApeTask *task);
void ape_task_set_input(ApeTask *task, const char *input);
void ape_task_set_output(ApeTask *task, const char *output);
void ape_task_add_input(ApeTask *task, const char *input);
void ape_task_add_dep(ApeTask *task, ApeTask *dep);
void ape_task_set_cmd(ApeTask *task, ApeCmd cmd);
int ape_task_needs_rebuild(ApeTask *task);
int ape_task_ready(ApeTask *task); /* All deps completed? */

/* ----------------------------------------------------------------------------
 * Builder - Build target with context and tasks
 * ---------------------------------------------------------------------------- */

/* Builder dependency list */
typedef struct {
	size_t capacity;
	size_t count;
	ApeBuilder **items;
} ApeBuilderList;

struct ApeBuilder {
	char *name; /* Target name */
	ApeTargetType type;
	ApeBuildCtx *ctx; /* Parent build context */
	ApeToolchain *toolchain; /* Can override context's toolchain */
	int owns_toolchain; /* Whether to free toolchain */

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

	/* Dependencies */
	ApeBuilderList deps; /* Other builders to build first */

	/* Tasks (generated during build) */
	ApeTaskList tasks;
	int next_task_id;

	/* State */
	int built; /* Has been built this session */
	int build_failed;
};

ApeBuilder *ape_builder_new(ApeBuildCtx *ctx, const char *name);
void ape_builder_free(ApeBuilder *builder);
void ape_builder_set_type(ApeBuilder *builder, ApeTargetType type);
void ape_builder_set_toolchain(ApeBuilder *builder, ApeToolchain *tc);
void ape_builder_set_output_dir(ApeBuilder *builder, const char *dir);
void ape_builder_set_output_name(ApeBuilder *builder, const char *name);

/* Source management */
void ape_builder_add_source(ApeBuilder *builder, const char *path);
void ape_builder_add_sources(ApeBuilder *builder, const char **paths, size_t count);
void ape_builder_add_source_dir(ApeBuilder *builder, const char *dir);
void ape_builder_add_source_dir_r(ApeBuilder *builder, const char *dir);
void ape_builder_add_source_glob(ApeBuilder *builder, const char *pattern);

/* Compiler flags */
void ape_builder_add_cflag(ApeBuilder *builder, const char *flag);
void ape_builder_add_include(ApeBuilder *builder, const char *dir);
void ape_builder_add_define(ApeBuilder *builder, const char *define);
void ape_builder_add_define_value(ApeBuilder *builder, const char *name, const char *value);

/* Linker flags */
void ape_builder_add_ldflag(ApeBuilder *builder, const char *flag);
void ape_builder_add_lib_dir(ApeBuilder *builder, const char *dir);
void ape_builder_add_lib(ApeBuilder *builder, const char *lib);

/* Dependencies */
void ape_builder_depends_on(ApeBuilder *builder, ApeBuilder *dep);
void ape_builder_link_with(ApeBuilder *builder, ApeBuilder *lib_builder);

/* Build operations */
int ape_builder_build(ApeBuilder *builder);
int ape_builder_clean(ApeBuilder *builder);
int ape_builder_rebuild(ApeBuilder *builder);
char *ape_builder_output_path(ApeBuilder *builder);

/* Task generation (internal, but exposed for flexibility) */
ApeTask *ape_builder_add_compile_task(ApeBuilder *builder, const char *source);
ApeTask *ape_builder_add_link_task(ApeBuilder *builder);
ApeTask *ape_builder_add_archive_task(ApeBuilder *builder);
ApeTask *ape_builder_add_command_task(ApeBuilder *builder, const char *name, ApeCmd cmd);
void ape_builder_generate_tasks(ApeBuilder *builder);

/* ----------------------------------------------------------------------------
 * Build Context - Global configuration and builder registry
 * ---------------------------------------------------------------------------- */

struct ApeBuildCtx {
	ApeToolchain *toolchain; /* Default toolchain */
	int owns_toolchain;
	char *output_dir; /* Default output directory */
	int parallel_jobs; /* Max parallel tasks (0 = auto) */
	ApeVerbosity verbosity;
	int force_rebuild; /* Ignore timestamps */
	int dry_run; /* Don't actually run commands */
	int keep_going; /* Continue on errors */

	ApeBuilderList builders; /* Registered builders */
};

ApeBuildCtx *ape_ctx_new(void);
void ape_ctx_free(ApeBuildCtx *ctx);
void ape_ctx_set_toolchain(ApeBuildCtx *ctx, ApeToolchain *tc);
void ape_ctx_set_output_dir(ApeBuildCtx *ctx, const char *dir);
void ape_ctx_set_parallel(ApeBuildCtx *ctx, int jobs);
void ape_ctx_set_verbosity(ApeBuildCtx *ctx, ApeVerbosity level);
void ape_ctx_set_force_rebuild(ApeBuildCtx *ctx, int force);
void ape_ctx_set_dry_run(ApeBuildCtx *ctx, int dry_run);
void ape_ctx_set_keep_going(ApeBuildCtx *ctx, int keep_going);
ApeToolchain *ape_ctx_get_toolchain(ApeBuildCtx *ctx);

/* Builder registration */
void ape_ctx_add_builder(ApeBuildCtx *ctx, ApeBuilder *builder);
ApeBuilder *ape_ctx_get_builder(ApeBuildCtx *ctx, const char *name);

/* Build operations */
int ape_ctx_build(ApeBuildCtx *ctx, const char *target);
int ape_ctx_build_all(ApeBuildCtx *ctx);
int ape_ctx_clean(ApeBuildCtx *ctx, const char *target);
int ape_ctx_clean_all(ApeBuildCtx *ctx);
int ape_ctx_rebuild(ApeBuildCtx *ctx, const char *target);
int ape_ctx_rebuild_all(ApeBuildCtx *ctx);

/* ----------------------------------------------------------------------------
 * Task Scheduler - Parallel task execution with dependency resolution
 * ---------------------------------------------------------------------------- */

typedef struct ApeScheduler ApeScheduler;

ApeScheduler *ape_scheduler_new(ApeBuildCtx *ctx);
void ape_scheduler_free(ApeScheduler *sched);
void ape_scheduler_add_task(ApeScheduler *sched, ApeTask *task);
void ape_scheduler_add_tasks(ApeScheduler *sched, ApeTaskList *tasks);
int ape_scheduler_run(ApeScheduler *sched);
int ape_scheduler_get_completed(ApeScheduler *sched);
int ape_scheduler_get_failed(ApeScheduler *sched);
int ape_scheduler_get_skipped(ApeScheduler *sched);

/* ----------------------------------------------------------------------------
 * Utility functions
 * ---------------------------------------------------------------------------- */

char *ape_build_obj_path(ApeBuildCtx *ctx, ApeBuilder *builder, const char *source);
char *ape_build_output_path(ApeBuildCtx *ctx, ApeBuilder *builder);
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

/* BEGIN ape_build.c */
/*
 * ape_build.c - Core build module implementation
 */

#include "apebuild_internal.h"

#include <string.h>
#include <stdio.h>
#include <unistd.h>

/* ============================================================================
 * Toolchain Implementation
 * ============================================================================ */

APEBUILD_DEF ApeToolchain *ape_toolchain_new(const char *name)
{
	ApeToolchain *tc = (ApeToolchain *)APEBUILD_MALLOC(sizeof(ApeToolchain));
	memset(tc, 0, sizeof(ApeToolchain));

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

	ape_sl_init(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYtc->default_cflags);
	ape_sl_init(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYtc->default_ldflags);

	return tc;
}

APEBUILD_DEF void ape_toolchain_free(ApeToolchain *tc)
{
	if (!tc)
		return;

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
	ape_sl_free(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYtc->default_cflags);
	ape_sl_free(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYtc->default_ldflags);
	APEBUILD_FREE(tc);
}

APEBUILD_DEF ApeToolchain *ape_toolchain_clone(const ApeToolchain *tc)
{
	if (!tc)
		return NULL;

	ApeToolchain *clone = ape_toolchain_new(tc->name);
	APEBUILD_FREE(clone->cc);
	APEBUILD_FREE(clone->cxx);
	APEBUILD_FREE(clone->ld);
	APEBUILD_FREE(clone->ar);
	APEBUILD_FREE(clone->obj_ext);
	APEBUILD_FREE(clone->exe_ext);
	APEBUILD_FREE(clone->static_lib_ext);
	APEBUILD_FREE(clone->shared_lib_ext);
	APEBUILD_FREE(clone->lib_prefix);

	clone->cc = ape_str_dup(tc->cc);
	clone->cxx = ape_str_dup(tc->cxx);
	clone->ld = ape_str_dup(tc->ld);
	clone->ar = ape_str_dup(tc->ar);
	clone->obj_ext = ape_str_dup(tc->obj_ext);
	clone->exe_ext = ape_str_dup(tc->exe_ext);
	clone->static_lib_ext = ape_str_dup(tc->static_lib_ext);
	clone->shared_lib_ext = ape_str_dup(tc->shared_lib_ext);
	clone->lib_prefix = ape_str_dup(tc->lib_prefix);

	clone->default_cflags = ape_sl_clone(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYtc->default_cflags);
	clone->default_ldflags = ape_sl_clone(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYtc->default_ldflags);

	return clone;
}

APEBUILD_DEF ApeToolchain *ape_toolchain_gcc(void)
{
	ApeToolchain *tc = ape_toolchain_new("gcc");
	APEBUILD_FREE(tc->cc);
	APEBUILD_FREE(tc->cxx);
	APEBUILD_FREE(tc->ld);

	tc->cc = ape_str_dup("gcc");
	tc->cxx = ape_str_dup("g++");
	tc->ld = ape_str_dup("gcc");

	return tc;
}

APEBUILD_DEF ApeToolchain *ape_toolchain_clang(void)
{
	ApeToolchain *tc = ape_toolchain_new("clang");
	APEBUILD_FREE(tc->cc);
	APEBUILD_FREE(tc->cxx);
	APEBUILD_FREE(tc->ld);

	tc->cc = ape_str_dup("clang");
	tc->cxx = ape_str_dup("clang++");
	tc->ld = ape_str_dup("clang");

	return tc;
}

APEBUILD_DEF void ape_toolchain_set_cc(ApeToolchain *tc, const char *cc)
{
	APEBUILD_FREE(tc->cc);
	tc->cc = ape_str_dup(cc);
}

APEBUILD_DEF void ape_toolchain_set_cxx(ApeToolchain *tc, const char *cxx)
{
	APEBUILD_FREE(tc->cxx);
	tc->cxx = ape_str_dup(cxx);
}

APEBUILD_DEF void ape_toolchain_set_ld(ApeToolchain *tc, const char *ld)
{
	APEBUILD_FREE(tc->ld);
	tc->ld = ape_str_dup(ld);
}

APEBUILD_DEF void ape_toolchain_set_ar(ApeToolchain *tc, const char *ar)
{
	APEBUILD_FREE(tc->ar);
	tc->ar = ape_str_dup(ar);
}

APEBUILD_DEF void ape_toolchain_add_cflag(ApeToolchain *tc, const char *flag)
{
	ape_sl_append_dup(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYtc->default_cflags, flag);
}

APEBUILD_DEF void ape_toolchain_add_ldflag(ApeToolchain *tc, const char *flag)
{
	ape_sl_append_dup(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYtc->default_ldflags, flag);
}

/* ============================================================================
 * Task Implementation
 * ============================================================================ */

APEBUILD_DEF ApeTask *ape_task_new(ApeBuilder *builder, ApeTaskType type, const char *name)
{
	ApeTask *task = (ApeTask *)APEBUILD_MALLOC(sizeof(ApeTask));
	memset(task, 0, sizeof(ApeTask));

	task->id = builder->next_task_id++;
	task->type = type;
	task->status = APE_TASK_PENDING;
	task->name = ape_str_dup(name);
	task->builder = builder;
	task->proc = APE_INVALID_HANDLE;
	task->exit_code = -1;

	ape_sl_init(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYtask->inputs);
	ape_cmd_init(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYtask->cmd);
	task->deps.capacity = 0;
	task->deps.count = 0;
	task->deps.items = NULL;

	return task;
}

APEBUILD_DEF void ape_task_free(ApeTask *task)
{
	if (!task)
		return;

	APEBUILD_FREE(task->name);
	APEBUILD_FREE(task->input);
	APEBUILD_FREE(task->output);
	ape_sl_free(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYtask->inputs);
	ape_cmd_free(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYtask->cmd);
	APEBUILD_FREE(task->deps.items);
	APEBUILD_FREE(task);
}

APEBUILD_DEF void ape_task_set_input(ApeTask *task, const char *input)
{
	APEBUILD_FREE(task->input);
	task->input = ape_str_dup(input);
}

APEBUILD_DEF void ape_task_set_output(ApeTask *task, const char *output)
{
	APEBUILD_FREE(task->output);
	task->output = ape_str_dup(output);
}

APEBUILD_DEF void ape_task_add_input(ApeTask *task, const char *input)
{
	ape_sl_append_dup(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYtask->inputs, input);
}

APEBUILD_DEF void ape_task_add_dep(ApeTask *task, ApeTask *dep)
{
	ape_da_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYtask->deps, dep);
}

APEBUILD_DEF void ape_task_set_cmd(ApeTask *task, ApeCmd cmd)
{
	ape_cmd_free(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYtask->cmd);
	task->cmd = cmd;
}

APEBUILD_DEF int ape_task_needs_rebuild(ApeTask *task)
{
	ApeBuildCtx *ctx = task->builder->ctx;

	/* Force rebuild mode */
	if (ctx #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY ctx->force_rebuild)
		return APEBUILD_TRUE;

	/* No output means always run */
	if (!task->output)
		return APEBUILD_TRUE;

	/* Output doesn't exist */
	if (!ape_fs_exists(task->output))
		return APEBUILD_TRUE;

	/* Check primary input */
	if (task->input) {
		if (ape_fs_is_newer(task->input, task->output))
			return APEBUILD_TRUE;
	}

	/* Check additional inputs */
	for (size_t i = 0; i < task->inputs.count; i++) {
		if (ape_fs_is_newer(task->inputs.items[i], task->output))
			return APEBUILD_TRUE;
	}

	return APEBUILD_FALSE;
}

APEBUILD_DEF int ape_task_ready(ApeTask *task)
{
	for (size_t i = 0; i < task->deps.count; i++) {
		ApeTask *dep = task->deps.items[i];
		if (dep->status !=
		    APE_TASK_COMPLETED #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY dep
			    ->status != APE_TASK_SKIPPED) {
			return APEBUILD_FALSE;
		}
	}
	return APEBUILD_TRUE;
}

/* ============================================================================
 * Builder Implementation
 * ============================================================================ */

APEBUILD_DEF ApeBuilder *ape_builder_new(ApeBuildCtx *ctx, const char *name)
{
	ApeBuilder *builder = (ApeBuilder *)APEBUILD_MALLOC(sizeof(ApeBuilder));
	memset(builder, 0, sizeof(ApeBuilder));

	builder->name = ape_str_dup(name);
	builder->type = APE_TARGET_EXECUTABLE;
	builder->ctx = ctx;

	ape_sl_init(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYbuilder->sources);
	ape_sl_init(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYbuilder->cflags);
	ape_sl_init(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYbuilder->include_dirs);
	ape_sl_init(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYbuilder->defines);
	ape_sl_init(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYbuilder->ldflags);
	ape_sl_init(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYbuilder->lib_dirs);
	ape_sl_init(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYbuilder->libs);

	builder->deps.capacity = 0;
	builder->deps.count = 0;
	builder->deps.items = NULL;

	builder->tasks.capacity = 0;
	builder->tasks.count = 0;
	builder->tasks.items = NULL;

	return builder;
}

APEBUILD_DEF void ape_builder_free(ApeBuilder *builder)
{
	if (!builder)
		return;

	APEBUILD_FREE(builder->name);
	APEBUILD_FREE(builder->output_dir);
	APEBUILD_FREE(builder->output_name);

	if (builder->owns_toolchain)
		ape_toolchain_free(builder->toolchain);

	ape_sl_free(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYbuilder->sources);
	ape_sl_free(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYbuilder->cflags);
	ape_sl_free(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYbuilder->include_dirs);
	ape_sl_free(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYbuilder->defines);
	ape_sl_free(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYbuilder->ldflags);
	ape_sl_free(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYbuilder->lib_dirs);
	ape_sl_free(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYbuilder->libs);

	APEBUILD_FREE(builder->deps.items);

	/* Free tasks */
	for (size_t i = 0; i < builder->tasks.count; i++) {
		ape_task_free(builder->tasks.items[i]);
	}
	APEBUILD_FREE(builder->tasks.items);

	APEBUILD_FREE(builder);
}

APEBUILD_DEF void ape_builder_set_type(ApeBuilder *builder, ApeTargetType type)
{
	builder->type = type;
}

APEBUILD_DEF void ape_builder_set_toolchain(ApeBuilder *builder, ApeToolchain *tc)
{
	if (builder->owns_toolchain)
		ape_toolchain_free(builder->toolchain);
	builder->toolchain = tc;
	builder->owns_toolchain = 0;
}

APEBUILD_DEF void ape_builder_set_output_dir(ApeBuilder *builder, const char *dir)
{
	APEBUILD_FREE(builder->output_dir);
	builder->output_dir = ape_str_dup(dir);
}

APEBUILD_DEF void ape_builder_set_output_name(ApeBuilder *builder, const char *name)
{
	APEBUILD_FREE(builder->output_name);
	builder->output_name = ape_str_dup(name);
}

/* Source management */

APEBUILD_DEF void ape_builder_add_source(ApeBuilder *builder, const char *path)
{
	ape_sl_append_dup(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYbuilder->sources, path);
}

APEBUILD_DEF void ape_builder_add_sources(ApeBuilder *builder, const char **paths, size_t count)
{
	for (size_t i = 0; i < count; i++) {
		ape_builder_add_source(builder, paths[i]);
	}
}

APEBUILD_PRIVATE void ape_builder_add_source_callback(const char *path, const ApeDirEntry *entry, void *userdata)
{
	ApeBuilder *builder = (ApeBuilder *)userdata;

	if (!entry->is_file)
		return;

	/* Only add C/C++ source files */
	if (ape_str_ends_with(entry->name, ".c") || ape_str_ends_with(entry->name, ".cpp") || ape_str_ends_with(entry->name, ".cc") ||
	    ape_str_ends_with(entry->name, ".cxx")) {
		char *fullpath = ape_fs_join(path, entry->name);
		ape_sl_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYbuilder->sources, fullpath);
	}
}

APEBUILD_DEF void ape_builder_add_source_dir(ApeBuilder *builder, const char *dir)
{
	ape_fs_iterdir(dir, ape_builder_add_source_callback, builder);
}

APEBUILD_DEF void ape_builder_add_source_dir_r(ApeBuilder *builder, const char *dir)
{
	ape_fs_iterdir_r(dir, ape_builder_add_source_callback, builder);
}

APEBUILD_DEF void ape_builder_add_source_glob(ApeBuilder *builder, const char *pattern)
{
	ApeStrList files = ape_fs_glob(pattern);
	for (size_t i = 0; i < files.count; i++) {
		/* Transfer ownership */
		ape_sl_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYbuilder->sources, files.items[i]);
	}
	/* Free list but not strings (ownership transferred) */
	ape_sl_free_shallow(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYfiles);
}

/* Compiler flags */

APEBUILD_DEF void ape_builder_add_cflag(ApeBuilder *builder, const char *flag)
{
	ape_sl_append_dup(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYbuilder->cflags, flag);
}

APEBUILD_DEF void ape_builder_add_include(ApeBuilder *builder, const char *dir)
{
	ape_sl_append_dup(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYbuilder->include_dirs, dir);
}

APEBUILD_DEF void ape_builder_add_define(ApeBuilder *builder, const char *define)
{
	ape_sl_append_dup(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYbuilder->defines, define);
}

APEBUILD_DEF void ape_builder_add_define_value(ApeBuilder *builder, const char *name, const char *value)
{
	ApeStrBuilder sb = ape_sb_new();
	ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, name);
	ape_sb_append_char(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, '=');
	ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, value);
	ape_sl_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYbuilder->defines,
		      ape_sb_to_str_dup(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb));
	ape_sb_free(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb);
}

/* Linker flags */

APEBUILD_DEF void ape_builder_add_ldflag(ApeBuilder *builder, const char *flag)
{
	ape_sl_append_dup(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYbuilder->ldflags, flag);
}

APEBUILD_DEF void ape_builder_add_lib_dir(ApeBuilder *builder, const char *dir)
{
	ape_sl_append_dup(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYbuilder->lib_dirs, dir);
}

APEBUILD_DEF void ape_builder_add_lib(ApeBuilder *builder, const char *lib)
{
	ape_sl_append_dup(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYbuilder->libs, lib);
}

/* Dependencies */

APEBUILD_DEF void ape_builder_depends_on(ApeBuilder *builder, ApeBuilder *dep)
{
	ape_da_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYbuilder->deps, dep);
}

APEBUILD_DEF void ape_builder_link_with(ApeBuilder *builder, ApeBuilder *lib_builder)
{
	ape_builder_depends_on(builder, lib_builder);

	/* Add library to link */
	char *output = ape_builder_output_path(lib_builder);
	if (output) {
		char *dir = ape_fs_dirname(output);
		ape_builder_add_lib_dir(builder, dir);
		APEBUILD_FREE(dir);

		/* Extract library name */
		char *base = ape_fs_basename(output);
		char *stem = ape_fs_stem(output);

		/* Remove lib prefix if present */
		ApeToolchain *tc = lib_builder->toolchain;
		if (!tc)
			tc = lib_builder->ctx->toolchain;
		if (tc #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY tc
			    ->lib_prefix #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY
				    ape_str_starts_with(stem, tc->lib_prefix)) {
			char *lib_name = ape_str_dup(stem + strlen(tc->lib_prefix));
			ape_builder_add_lib(builder, lib_name);
			APEBUILD_FREE(lib_name);
		} else {
			ape_builder_add_lib(builder, stem);
		}

		APEBUILD_FREE(base);
		APEBUILD_FREE(stem);
		APEBUILD_FREE(output);
	}
}

/* Build output path */

APEBUILD_DEF char *ape_builder_output_path(ApeBuilder *builder)
{
	ApeToolchain *tc = builder->toolchain;
	if (!tc #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY builder->ctx)
		tc = builder->ctx->toolchain;
	if (!tc)
		return NULL;

	const char *output_dir = builder->output_dir;
	if (!output_dir #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY builder->ctx)
		output_dir = builder->ctx->output_dir;
	if (!output_dir)
		output_dir = "build";

	const char *name = builder->output_name ? builder->output_name : builder->name;

	ApeStrBuilder sb = ape_sb_new();
	ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, output_dir);
	ape_sb_append_char(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, '/');

	switch (builder->type) {
	case APE_TARGET_EXECUTABLE:
		ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, name);
		ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, tc->exe_ext);
		break;
	case APE_TARGET_STATIC_LIB:
		ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, tc->lib_prefix);
		ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, name);
		ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, tc->static_lib_ext);
		break;
	case APE_TARGET_SHARED_LIB:
		ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, tc->lib_prefix);
		ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, name);
		ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, tc->shared_lib_ext);
		break;
	case APE_TARGET_OBJECT:
		ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, name);
		ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, tc->obj_ext);
		break;
	}

	char *result = ape_sb_to_str_dup(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb);
	ape_sb_free(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb);
	return result;
}

/* Task generation */

APEBUILD_DEF ApeTask *ape_builder_add_compile_task(ApeBuilder *builder, const char *source)
{
	char *obj_path = ape_build_obj_path(builder->ctx, builder, source);
	char *base = ape_fs_basename(source);

	ApeStrBuilder name_sb = ape_sb_new();
	ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYname_sb, "Compile ");
	ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYname_sb, base);

	ApeTask *task = ape_task_new(builder, APE_TASK_COMPILE, ape_sb_to_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYname_sb));
	ape_sb_free(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYname_sb);
	APEBUILD_FREE(base);

	ape_task_set_input(task, source);
	ape_task_set_output(task, obj_path);

	/* Build compile command */
	ApeToolchain *tc = builder->toolchain;
	if (!tc)
		tc = builder->ctx->toolchain;

	ApeCmd cmd = ape_cmd_new();

	/* Compiler */
	if (ape_str_ends_with(source, ".cpp") || ape_str_ends_with(source, ".cc") || ape_str_ends_with(source, ".cxx")) {
		ape_cmd_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYcmd, tc->cxx);
	} else {
		ape_cmd_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYcmd, tc->cc);
	}

	/* Default flags from toolchain */
	for (size_t i = 0; i < tc->default_cflags.count; i++) {
		ape_cmd_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYcmd, tc->default_cflags.items[i]);
	}

	/* Builder-specific flags */
	for (size_t i = 0; i < builder->cflags.count; i++) {
		ape_cmd_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYcmd, builder->cflags.items[i]);
	}

	/* Include directories */
	for (size_t i = 0; i < builder->include_dirs.count; i++) {
		ApeStrBuilder inc = ape_sb_new();
		ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYinc, "-I");
		ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYinc, builder->include_dirs.items[i]);
		ape_cmd_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYcmd,
			       ape_sb_to_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYinc));
		ape_sb_free(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYinc);
	}

	/* Defines */
	for (size_t i = 0; i < builder->defines.count; i++) {
		ApeStrBuilder def = ape_sb_new();
		ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYdef, "-D");
		ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYdef, builder->defines.items[i]);
		ape_cmd_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYcmd,
			       ape_sb_to_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYdef));
		ape_sb_free(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYdef);
	}

	/* PIC for shared libraries */
	if (builder->type == APE_TARGET_SHARED_LIB) {
		ape_cmd_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYcmd, "-fPIC");
	}

	/* Compile only */
	ape_cmd_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYcmd, "-c");
	ape_cmd_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYcmd, source);
	ape_cmd_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYcmd, "-o");
	ape_cmd_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYcmd, obj_path);

	ape_task_set_cmd(task, cmd);
	ape_da_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYbuilder->tasks, task);

	APEBUILD_FREE(obj_path);
	return task;
}

APEBUILD_DEF ApeTask *ape_builder_add_link_task(ApeBuilder *builder)
{
	char *output = ape_builder_output_path(builder);

	ApeStrBuilder name_sb = ape_sb_new();
	ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYname_sb, "Link ");
	ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYname_sb, builder->name);

	ApeTask *task = ape_task_new(builder, APE_TASK_LINK, ape_sb_to_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYname_sb));
	ape_sb_free(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYname_sb);

	ape_task_set_output(task, output);

	/* Build link command */
	ApeToolchain *tc = builder->toolchain;
	if (!tc)
		tc = builder->ctx->toolchain;

	ApeCmd cmd = ape_cmd_new();
	ape_cmd_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYcmd, tc->ld);

	/* Default linker flags */
	for (size_t i = 0; i < tc->default_ldflags.count; i++) {
		ape_cmd_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYcmd, tc->default_ldflags.items[i]);
	}

	/* Builder-specific linker flags */
	for (size_t i = 0; i < builder->ldflags.count; i++) {
		ape_cmd_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYcmd, builder->ldflags.items[i]);
	}

	/* Shared library flags */
	if (builder->type == APE_TARGET_SHARED_LIB) {
		ape_cmd_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYcmd, "-shared");
	}

	/* Output */
	ape_cmd_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYcmd, "-o");
	ape_cmd_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYcmd, output);

	/* Object files (from compile tasks) */
	for (size_t i = 0; i < builder->tasks.count; i++) {
		ApeTask *compile_task = builder->tasks.items[i];
		if (compile_task->type == APE_TASK_COMPILE) {
			ape_cmd_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYcmd, compile_task->output);
			ape_task_add_input(task, compile_task->output);
			ape_task_add_dep(task, compile_task);
		}
	}

	/* Library directories */
	for (size_t i = 0; i < builder->lib_dirs.count; i++) {
		ApeStrBuilder lib_dir = ape_sb_new();
		ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYlib_dir, "-L");
		ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYlib_dir, builder->lib_dirs.items[i]);
		ape_cmd_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYcmd,
			       ape_sb_to_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYlib_dir));
		ape_sb_free(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYlib_dir);
	}

	/* Libraries */
	for (size_t i = 0; i < builder->libs.count; i++) {
		ApeStrBuilder lib = ape_sb_new();
		ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYlib, "-l");
		ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYlib, builder->libs.items[i]);
		ape_cmd_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYcmd,
			       ape_sb_to_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYlib));
		ape_sb_free(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYlib);
	}

	ape_task_set_cmd(task, cmd);
	ape_da_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYbuilder->tasks, task);

	APEBUILD_FREE(output);
	return task;
}

APEBUILD_DEF ApeTask *ape_builder_add_archive_task(ApeBuilder *builder)
{
	char *output = ape_builder_output_path(builder);

	ApeStrBuilder name_sb = ape_sb_new();
	ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYname_sb, "Archive ");
	ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYname_sb, builder->name);

	ApeTask *task = ape_task_new(builder, APE_TASK_ARCHIVE, ape_sb_to_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYname_sb));
	ape_sb_free(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYname_sb);

	ape_task_set_output(task, output);

	/* Build archive command */
	ApeToolchain *tc = builder->toolchain;
	if (!tc)
		tc = builder->ctx->toolchain;

	ApeCmd cmd = ape_cmd_new();
	ape_cmd_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYcmd, tc->ar);
	ape_cmd_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYcmd, "rcs");
	ape_cmd_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYcmd, output);

	/* Object files */
	for (size_t i = 0; i < builder->tasks.count; i++) {
		ApeTask *compile_task = builder->tasks.items[i];
		if (compile_task->type == APE_TASK_COMPILE) {
			ape_cmd_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYcmd, compile_task->output);
			ape_task_add_input(task, compile_task->output);
			ape_task_add_dep(task, compile_task);
		}
	}

	ape_task_set_cmd(task, cmd);
	ape_da_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYbuilder->tasks, task);

	APEBUILD_FREE(output);
	return task;
}

APEBUILD_DEF ApeTask *ape_builder_add_command_task(ApeBuilder *builder, const char *name, ApeCmd cmd)
{
	ApeTask *task = ape_task_new(builder, APE_TASK_COMMAND, name);
	ape_task_set_cmd(task, cmd);
	ape_da_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYbuilder->tasks, task);
	return task;
}

APEBUILD_DEF void ape_builder_generate_tasks(ApeBuilder *builder)
{
	/* Clear existing tasks */
	for (size_t i = 0; i < builder->tasks.count; i++) {
		ape_task_free(builder->tasks.items[i]);
	}
	builder->tasks.count = 0;
	builder->next_task_id = 0;

	/* Create compile tasks for each source */
	for (size_t i = 0; i < builder->sources.count; i++) {
		ape_builder_add_compile_task(builder, builder->sources.items[i]);
	}

	/* Create link/archive task if not object-only */
	if (builder->type != APE_TARGET_OBJECT) {
		if (builder->type == APE_TARGET_STATIC_LIB) {
			ape_builder_add_archive_task(builder);
		} else {
			ape_builder_add_link_task(builder);
		}
	}
}

/* Build operations */

APEBUILD_DEF int ape_builder_build(ApeBuilder *builder)
{
	if (builder->built)
		return builder->build_failed ? APEBUILD_FALSE : APEBUILD_TRUE;

	ApeBuildCtx *ctx = builder->ctx;
	ApeVerbosity verbosity = ctx ? ctx->verbosity : APE_VERBOSE_NORMAL;

	/* Build dependencies first */
	for (size_t i = 0; i < builder->deps.count; i++) {
		if (!ape_builder_build(builder->deps.items[i])) {
			builder->built = 1;
			builder->build_failed = 1;
			return APEBUILD_FALSE;
		}
	}

	/* Ensure output directory exists */
	const char *output_dir = builder->output_dir;
	if (!output_dir #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY ctx)
		output_dir = ctx->output_dir;
	if (!output_dir)
		output_dir = "build";
	ape_fs_mkdir_p(output_dir);

	/* Generate tasks */
	ape_builder_generate_tasks(builder);

	if (builder->tasks.count == 0) {
		if (verbosity >= APE_VERBOSE_NORMAL) {
			ape_log_info("Nothing to build for %s", builder->name);
		}
		builder->built = 1;
		return APEBUILD_TRUE;
	}

	/* Use scheduler to run tasks */
	ApeScheduler *sched = ape_scheduler_new(ctx);
	ape_scheduler_add_tasks(sched, #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYbuilder->tasks);

	if (verbosity >= APE_VERBOSE_NORMAL) {
		ape_log_build("Building %s...", builder->name);
	}

	int result = ape_scheduler_run(sched);

	if (verbosity >= APE_VERBOSE_NORMAL) {
		int completed = ape_scheduler_get_completed(sched);
		int failed = ape_scheduler_get_failed(sched);
		int skipped = ape_scheduler_get_skipped(sched);

		if (result) {
			ape_log_success("%s: %d compiled, %d skipped", builder->name, completed, skipped);
		} else {
			ape_log_failure("%s: %d failed, %d compiled, %d skipped", builder->name, failed, completed, skipped);
		}
	}

	ape_scheduler_free(sched);

	builder->built = 1;
	builder->build_failed = !result;
	return result;
}

APEBUILD_DEF int ape_builder_clean(ApeBuilder *builder)
{
	/* Remove all object files and output */
	char *output = ape_builder_output_path(builder);
	if (output #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY ape_fs_exists(
		    output)) {
		ape_fs_remove(output);
	}
	APEBUILD_FREE(output);

	/* Remove object files */
	for (size_t i = 0; i < builder->sources.count; i++) {
		char *obj = ape_build_obj_path(builder->ctx, builder, builder->sources.items[i]);
		if (obj #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY ape_fs_exists(
			    obj)) {
			ape_fs_remove(obj);
		}
		APEBUILD_FREE(obj);
	}

	builder->built = 0;
	builder->build_failed = 0;

	return APEBUILD_TRUE;
}

APEBUILD_DEF int ape_builder_rebuild(ApeBuilder *builder)
{
	ape_builder_clean(builder);
	builder->ctx->force_rebuild = 1;
	int result = ape_builder_build(builder);
	builder->ctx->force_rebuild = 0;
	return result;
}

/* ============================================================================
 * Build Context Implementation
 * ============================================================================ */

APEBUILD_DEF ApeBuildCtx *ape_ctx_new(void)
{
	ApeBuildCtx *ctx = (ApeBuildCtx *)APEBUILD_MALLOC(sizeof(ApeBuildCtx));
	memset(ctx, 0, sizeof(ApeBuildCtx));

	ctx->toolchain = ape_toolchain_gcc();
	ctx->owns_toolchain = 1;
	ctx->output_dir = ape_str_dup("build");
	ctx->parallel_jobs = 0; /* Auto-detect */
	ctx->verbosity = APE_VERBOSE_NORMAL;

	ctx->builders.capacity = 0;
	ctx->builders.count = 0;
	ctx->builders.items = NULL;

	return ctx;
}

APEBUILD_DEF void ape_ctx_free(ApeBuildCtx *ctx)
{
	if (!ctx)
		return;

	if (ctx->owns_toolchain)
		ape_toolchain_free(ctx->toolchain);
	APEBUILD_FREE(ctx->output_dir);

	/* Free builders */
	for (size_t i = 0; i < ctx->builders.count; i++) {
		ape_builder_free(ctx->builders.items[i]);
	}
	APEBUILD_FREE(ctx->builders.items);

	APEBUILD_FREE(ctx);
}

APEBUILD_DEF void ape_ctx_set_toolchain(ApeBuildCtx *ctx, ApeToolchain *tc)
{
	if (ctx->owns_toolchain)
		ape_toolchain_free(ctx->toolchain);
	ctx->toolchain = tc;
	ctx->owns_toolchain = 0;
}

APEBUILD_DEF void ape_ctx_set_output_dir(ApeBuildCtx *ctx, const char *dir)
{
	APEBUILD_FREE(ctx->output_dir);
	ctx->output_dir = ape_str_dup(dir);
}

APEBUILD_DEF void ape_ctx_set_parallel(ApeBuildCtx *ctx, int jobs)
{
	ctx->parallel_jobs = jobs;
}

APEBUILD_DEF void ape_ctx_set_verbosity(ApeBuildCtx *ctx, ApeVerbosity level)
{
	ctx->verbosity = level;
}

APEBUILD_DEF void ape_ctx_set_force_rebuild(ApeBuildCtx *ctx, int force)
{
	ctx->force_rebuild = force;
}

APEBUILD_DEF void ape_ctx_set_dry_run(ApeBuildCtx *ctx, int dry_run)
{
	ctx->dry_run = dry_run;
}

APEBUILD_DEF void ape_ctx_set_keep_going(ApeBuildCtx *ctx, int keep_going)
{
	ctx->keep_going = keep_going;
}

APEBUILD_DEF ApeToolchain *ape_ctx_get_toolchain(ApeBuildCtx *ctx)
{
	return ctx->toolchain;
}

APEBUILD_DEF void ape_ctx_add_builder(ApeBuildCtx *ctx, ApeBuilder *builder)
{
	ape_da_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYctx->builders, builder);
}

APEBUILD_DEF ApeBuilder *ape_ctx_get_builder(ApeBuildCtx *ctx, const char *name)
{
	for (size_t i = 0; i < ctx->builders.count; i++) {
		if (ape_str_eq(ctx->builders.items[i]->name, name)) {
			return ctx->builders.items[i];
		}
	}
	return NULL;
}

APEBUILD_DEF int ape_ctx_build(ApeBuildCtx *ctx, const char *target)
{
	ApeBuilder *builder = ape_ctx_get_builder(ctx, target);
	if (!builder) {
		ape_log_error("Unknown target: %s", target);
		return APEBUILD_FALSE;
	}
	return ape_builder_build(builder);
}

APEBUILD_DEF int ape_ctx_build_all(ApeBuildCtx *ctx)
{
	int all_success = APEBUILD_TRUE;
	for (size_t i = 0; i < ctx->builders.count; i++) {
		if (!ape_builder_build(ctx->builders.items[i])) {
			all_success = APEBUILD_FALSE;
			if (!ctx->keep_going)
				break;
		}
	}
	return all_success;
}

APEBUILD_DEF int ape_ctx_clean(ApeBuildCtx *ctx, const char *target)
{
	ApeBuilder *builder = ape_ctx_get_builder(ctx, target);
	if (!builder) {
		ape_log_error("Unknown target: %s", target);
		return APEBUILD_FALSE;
	}
	return ape_builder_clean(builder);
}

APEBUILD_DEF int ape_ctx_clean_all(ApeBuildCtx *ctx)
{
	int all_success = APEBUILD_TRUE;
	for (size_t i = 0; i < ctx->builders.count; i++) {
		if (!ape_builder_clean(ctx->builders.items[i])) {
			all_success = APEBUILD_FALSE;
		}
	}
	return all_success;
}

APEBUILD_DEF int ape_ctx_rebuild(ApeBuildCtx *ctx, const char *target)
{
	ape_ctx_clean(ctx, target);
	return ape_ctx_build(ctx, target);
}

APEBUILD_DEF int ape_ctx_rebuild_all(ApeBuildCtx *ctx)
{
	ape_ctx_clean_all(ctx);
	return ape_ctx_build_all(ctx);
}

/* ============================================================================
 * Scheduler Implementation
 * ============================================================================ */

struct ApeScheduler {
	ApeBuildCtx *ctx;
	ApeTaskList pending;
	ApeTaskList running;
	int completed;
	int failed;
	int skipped;
	int max_parallel;
};

APEBUILD_DEF ApeScheduler *ape_scheduler_new(ApeBuildCtx *ctx)
{
	ApeScheduler *sched = (ApeScheduler *)APEBUILD_MALLOC(sizeof(ApeScheduler));
	memset(sched, 0, sizeof(ApeScheduler));

	sched->ctx = ctx;
	sched->max_parallel = ctx->parallel_jobs;
	if (sched->max_parallel <= 0) {
		sched->max_parallel = ape_build_get_cpu_count();
	}

	return sched;
}

APEBUILD_DEF void ape_scheduler_free(ApeScheduler *sched)
{
	if (!sched)
		return;

	APEBUILD_FREE(sched->pending.items);
	APEBUILD_FREE(sched->running.items);
	APEBUILD_FREE(sched);
}

APEBUILD_DEF void ape_scheduler_add_task(ApeScheduler *sched, ApeTask *task)
{
	ape_da_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsched->pending, task);
}

APEBUILD_DEF void ape_scheduler_add_tasks(ApeScheduler *sched, ApeTaskList *tasks)
{
	for (size_t i = 0; i < tasks->count; i++) {
		ape_scheduler_add_task(sched, tasks->items[i]);
	}
}

APEBUILD_PRIVATE ApeTask *ape_scheduler_find_ready(ApeScheduler *sched)
{
	for (size_t i = 0; i < sched->pending.count; i++) {
		ApeTask *task = sched->pending.items[i];
		if (task->status ==
		    APE_TASK_PENDING #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY
			    ape_task_ready(task)) {
			return task;
		}
	}
	return NULL;
}

APEBUILD_PRIVATE void ape_scheduler_remove_pending(ApeScheduler *sched, ApeTask *task)
{
	for (size_t i = 0; i < sched->pending.count; i++) {
		if (sched->pending.items[i] == task) {
			ape_da_remove(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsched->pending, i);
			return;
		}
	}
}

APEBUILD_PRIVATE void ape_scheduler_remove_running(ApeScheduler *sched, ApeTask *task)
{
	for (size_t i = 0; i < sched->running.count; i++) {
		if (sched->running.items[i] == task) {
			ape_da_remove(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsched->running, i);
			return;
		}
	}
}

APEBUILD_PRIVATE int ape_scheduler_start_task(ApeScheduler *sched, ApeTask *task)
{
	ApeBuildCtx *ctx = sched->ctx;
	ApeVerbosity verbosity = ctx ? ctx->verbosity : APE_VERBOSE_NORMAL;

	/* Check if rebuild needed */
	if (!ape_task_needs_rebuild(task)) {
		task->status = APE_TASK_SKIPPED;
		sched->skipped++;
		if (verbosity >= APE_VERBOSE_VERBOSE) {
			ape_log_debug("Skipping %s (up to date)", task->name);
		}
		return APEBUILD_TRUE;
	}

	/* Ensure output directory exists */
	if (task->output) {
		char *dir = ape_fs_dirname(task->output);
		ape_fs_mkdir_p(dir);
		APEBUILD_FREE(dir);
	}

	/* Log command */
	if (verbosity >= APE_VERBOSE_VERBOSE) {
		char *cmd_str = ape_cmd_render_quoted(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYtask->cmd);
		ape_log_cmd("%s", cmd_str);
		APEBUILD_FREE(cmd_str);
	} else if (verbosity >= APE_VERBOSE_NORMAL) {
		ape_log_info("%s", task->name);
	}

	/* Dry run */
	if (ctx #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY ctx->dry_run) {
		task->status = APE_TASK_COMPLETED;
		task->exit_code = 0;
		sched->completed++;
		return APEBUILD_TRUE;
	}

	/* Start process */
	task->proc = ape_cmd_start(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYtask->cmd);
	if (task->proc == APE_INVALID_HANDLE) {
		task->status = APE_TASK_FAILED;
		task->exit_code = -1;
		sched->failed++;
		return APEBUILD_FALSE;
	}

	task->status = APE_TASK_RUNNING;
	ape_scheduler_remove_pending(sched, task);
	ape_da_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsched->running, task);

	return APEBUILD_TRUE;
}

APEBUILD_PRIVATE void ape_scheduler_check_running(ApeScheduler *sched)
{
	ApeBuildCtx *ctx = sched->ctx;
	ApeVerbosity verbosity = ctx ? ctx->verbosity : APE_VERBOSE_NORMAL;

	for (size_t i = 0; i < sched->running.count;) {
		ApeTask *task = sched->running.items[i];

		if (ape_proc_poll(task->proc)) {
			ApeProcResult result = ape_proc_result(task->proc);
			task->exit_code = result.exit_code;

			if (result.status ==
			    APE_PROC_COMPLETED #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY
				    result.exit_code == 0) {
				task->status = APE_TASK_COMPLETED;
				sched->completed++;
			} else {
				task->status = APE_TASK_FAILED;
				sched->failed++;

				if (verbosity >= APE_VERBOSE_NORMAL) {
					ape_log_failure("%s failed (exit code %d)", task->name, task->exit_code);
				}
			}

			ape_proc_handle_release(task->proc);
			task->proc = APE_INVALID_HANDLE;
			ape_scheduler_remove_running(sched, task);
			/* Don't increment i since we removed current element */
		} else {
			i++;
		}
	}
}

APEBUILD_DEF int ape_scheduler_run(ApeScheduler *sched)
{
	ApeBuildCtx *ctx = sched->ctx;

	while (sched->pending.count > 0 || sched->running.count > 0) {
		/* Check for completed tasks */
		ape_scheduler_check_running(sched);

		/* Start new tasks */
		while ((int)sched->running.count < sched->max_parallel) {
			ApeTask *task = ape_scheduler_find_ready(sched);
			if (!task)
				break;

			if (!ape_scheduler_start_task(sched, task)) {
				if (!ctx || !ctx->keep_going) {
					/* Wait for running tasks to finish */
					while (sched->running.count > 0) {
						ape_scheduler_check_running(sched);
						usleep(10000);
					}
					return APEBUILD_FALSE;
				}
			}
		}

		/* Short sleep if tasks are running */
		if (sched->running.count > 0) {
			usleep(10000); /* 10ms */
		}
	}

	return sched->failed == 0 ? APEBUILD_TRUE : APEBUILD_FALSE;
}

APEBUILD_DEF int ape_scheduler_get_completed(ApeScheduler *sched)
{
	return sched->completed;
}

APEBUILD_DEF int ape_scheduler_get_failed(ApeScheduler *sched)
{
	return sched->failed;
}

APEBUILD_DEF int ape_scheduler_get_skipped(ApeScheduler *sched)
{
	return sched->skipped;
}

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

APEBUILD_DEF char *ape_build_obj_path(ApeBuildCtx *ctx, ApeBuilder *builder, const char *source)
{
	ApeToolchain *tc = builder ? builder->toolchain : NULL;
	if (!tc #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY ctx)
		tc = ctx->toolchain;
	if (!tc)
		return NULL;

	const char *output_dir = builder ? builder->output_dir : NULL;
	if (!output_dir #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY ctx)
		output_dir = ctx->output_dir;
	if (!output_dir)
		output_dir = "build";

	char *stem = ape_fs_stem(source);
	char *dir = ape_fs_dirname(source);

	ApeStrBuilder sb = ape_sb_new();
	ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, output_dir);
	ape_sb_append_char(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, '/');

	/* Include relative path to avoid collisions */
	if (dir #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY strcmp(dir, ".") != 0) {
		char *rel = ape_str_replace_all(dir, "/", "_");
		ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, rel);
		ape_sb_append_char(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, '_');
		APEBUILD_FREE(rel);
	}

	ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, stem);
	ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, tc->obj_ext);

	APEBUILD_FREE(stem);
	APEBUILD_FREE(dir);

	char *result = ape_sb_to_str_dup(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb);
	ape_sb_free(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb);
	return result;
}

APEBUILD_DEF char *ape_build_output_path(ApeBuildCtx *ctx, ApeBuilder *builder)
{
	return ape_builder_output_path(builder);
}

APEBUILD_DEF int ape_build_get_cpu_count(void)
{
#ifdef _SC_NPROCESSORS_ONLN
	long count = sysconf(_SC_NPROCESSORS_ONLN);
	if (count > 0)
		return (int)count;
#endif
	return 4; /* Default fallback */
}

/* ============================================================================
 * Auto-rebuild Support
 * ============================================================================ */

APEBUILD_DEF int ape_self_needs_rebuild(const char *binary, const char *source)
{
	return ape_fs_needs_rebuild1(binary, source);
}

APEBUILD_DEF int ape_self_rebuild(int argc, char **argv, const char *source)
{
	const char *binary = argv[0];

	ape_log_info("Build script changed, rebuilding...");

	/* Rename current binary */
	char *old_binary = ape_str_concat(binary, ".old");
	ape_fs_rename(binary, old_binary);

	/* Rebuild */
	ApeCmd cmd = ape_cmd_new();
	ape_cmd_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYcmd, "cc");
	ape_cmd_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYcmd, "-o");
	ape_cmd_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYcmd, binary);
	ape_cmd_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYcmd, source);

	int result = ape_cmd_run_status(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYcmd);
	ape_cmd_free(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYcmd);

	if (result != 0) {
		ape_log_error("Failed to rebuild build script");
		/* Restore old binary */
		ape_fs_rename(old_binary, binary);
		APEBUILD_FREE(old_binary);
		return 1;
	}

	APEBUILD_FREE(old_binary);

	/* Re-execute */
	ape_log_info("Re-executing build script...");

	ApeCmd run_cmd = ape_cmd_new();
	for (int i = 0; i < argc; i++) {
		ape_cmd_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYrun_cmd, argv[i]);
	}

	result = ape_cmd_run_status(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYrun_cmd);
	ape_cmd_free(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYrun_cmd);

	return result;
}
/* END ape_build.c */

/* BEGIN ape_cmd.c */
/*
 * ape_cmd.c - Command execution module implementation
 */

#include "apebuild_internal.h"

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>

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

APEBUILD_PRIVATE void ape_proc_table_init(void)
{
	if (ape_proc_table_initialized)
		return;
	for (int i = 0; i < APE_MAX_PROCS; i++) {
		ape_proc_table[i].in_use = 0;
	}
	ape_proc_table_initialized = 1;
}

APEBUILD_PRIVATE ApeProcHandle ape_proc_alloc(pid_t pid)
{
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

APEBUILD_PRIVATE ApeProcEntry *ape_proc_get(ApeProcHandle handle)
{
	if (handle < 0 || handle >= APE_MAX_PROCS)
		return NULL;
	if (!ape_proc_table[handle].in_use)
		return NULL;
	return #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYape_proc_table[handle];
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

APEBUILD_DEF void ape_cmd_init(ApeCmd *cmd)
{
	cmd->capacity = 0;
	cmd->count = 0;
	cmd->items = NULL;
	cmd->cwd = NULL;
	ape_sl_init(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYcmd->env);
}

APEBUILD_DEF ApeCmd ape_cmd_new(void)
{
	ApeCmd cmd;
	ape_cmd_init(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYcmd);
	return cmd;
}

APEBUILD_DEF ApeCmd ape_cmd_from(const char *program)
{
	ApeCmd cmd = ape_cmd_new();
	ape_cmd_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYcmd, program);
	return cmd;
}

APEBUILD_DEF void ape_cmd_append(ApeCmd *cmd, const char *arg)
{
	ape_da_append(cmd, arg);
}

APEBUILD_DEF void ape_cmd_append_many(ApeCmd *cmd, const char **args, size_t count)
{
	ape_da_append_many(cmd, args, count);
}

APEBUILD_DEF void ape_cmd_prepend(ApeCmd *cmd, const char *arg)
{
	ape_da_prepend(cmd, arg);
}

APEBUILD_DEF ApeCmd ape_cmd_clone(const ApeCmd *cmd)
{
	ApeCmd result = ape_cmd_new();
	for (size_t i = 0; i < cmd->count; i++) {
		ape_cmd_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYresult, cmd->items[i]);
	}
	if (cmd->cwd) {
		result.cwd = ape_str_dup(cmd->cwd);
	}
	result.env = ape_sl_clone(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYcmd->env);
	return result;
}

APEBUILD_DEF void ape_cmd_free(ApeCmd *cmd)
{
	APEBUILD_FREE(cmd->items);
	APEBUILD_FREE(cmd->cwd);
	ape_sl_free(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYcmd->env);
	ape_cmd_init(cmd);
}

APEBUILD_DEF void ape_cmd_clear(ApeCmd *cmd)
{
	cmd->count = 0;
	APEBUILD_FREE(cmd->cwd);
	cmd->cwd = NULL;
	ape_sl_clear(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYcmd->env);
}

/* ============================================================================
 * Command Display
 * ============================================================================ */

APEBUILD_DEF char *ape_cmd_render(const ApeCmd *cmd)
{
	ApeStrBuilder sb = ape_sb_new();
	for (size_t i = 0; i < cmd->count; i++) {
		if (i > 0)
			ape_sb_append_char(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, ' ');
		ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, cmd->items[i]);
	}
	char *result = ape_sb_to_str_dup(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb);
	ape_sb_free(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb);
	return result;
}

APEBUILD_DEF char *ape_cmd_render_quoted(const ApeCmd *cmd)
{
	ApeStrBuilder sb = ape_sb_new();
	for (size_t i = 0; i < cmd->count; i++) {
		const char *arg = cmd->items[i];
		if (i > 0)
			ape_sb_append_char(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, ' ');

		/* Check if quoting is needed */
		int needs_quote = 0;
		for (const char *p = arg; *p; p++) {
			if (*p == ' ' || *p == '\t' || *p == '"' || *p == '\'' || *p == '\' || *p == '$' || *p == '`') {
				needs_quote = 1;
				break;
		}
	}

	if (needs_quote) {
		ape_sb_append_char(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, '\'');
		for (const char *p = arg; *p; p++) {
			if (*p == '\'') {
				ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, "'\''");
			} else {
				ape_sb_append_char(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, *p);
			}
		}
		ape_sb_append_char(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, '\'');
	} else {
		ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, arg);
	}
}
char *result = ape_sb_to_str_dup(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb);
ape_sb_free(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb);
return result;
}

APEBUILD_DEF void ape_cmd_print(const ApeCmd *cmd)
{
	char *rendered = ape_cmd_render_quoted(cmd);
	fprintf(stderr, "%s\n", rendered);
	APEBUILD_FREE(rendered);
}

APEBUILD_DEF void ape_cmd_log(const ApeCmd *cmd, const char *prefix)
{
	char *rendered = ape_cmd_render_quoted(cmd);
	fprintf(stderr, "%s%s\n", prefix ? prefix : "", rendered);
	APEBUILD_FREE(rendered);
}

/* ============================================================================
 * Synchronous Execution
 * ============================================================================ */

APEBUILD_DEF int ape_cmd_run(ApeCmd *cmd)
{
	return ape_cmd_run_status(cmd) == 0 ? APEBUILD_TRUE : APEBUILD_FALSE;
}

APEBUILD_DEF int ape_cmd_run_status(ApeCmd *cmd)
{
	ApeProcHandle handle = ape_cmd_start(cmd);
	if (handle == APE_INVALID_HANDLE)
		return -1;

	ape_proc_wait(handle);
	ApeProcResult result = ape_proc_result(handle);
	ape_proc_handle_release(handle);

	if (result.status == APE_PROC_SIGNALED)
		return 128 + result.signal;
	return result.exit_code;
}

APEBUILD_DEF char *ape_cmd_run_capture(ApeCmd *cmd, int *exit_code)
{
	if (cmd->count == 0)
		return NULL;

	int pipefd[2];
	if (pipe(pipefd) == -1)
		return NULL;

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
			if (chdir(cmd->cwd) != 0) {
				_exit(127);
			}
		}

		/* Set environment variables */
		for (size_t i = 0; i < cmd->env.count; i++) {
			putenv(cmd->env.items[i]);
		}

		/* Build argv with NULL terminator */
		const char **argv = (const char **)APEBUILD_MALLOC((cmd->count + 1) * sizeof(char *));
		for (size_t i = 0; i < cmd->count; i++) {
			argv[i] = cmd->items[i];
		}
		argv[cmd->count] = NULL;

		execvp(argv[0], (char *const *)argv);
		_exit(127);
	}

	/* Parent process */
	close(pipefd[1]);

	ApeStrBuilder sb = ape_sb_new();
	char buf[4096];
	ssize_t n;
	while ((n = read(pipefd[0], buf, sizeof(buf))) > 0) {
		ape_sb_append_strn(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, buf, n);
	}
	close(pipefd[0]);

	int status;
	waitpid(pid, #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYstatus, 0);

	if (exit_code) {
		if (WIFEXITED(status)) {
			*exit_code = WEXITSTATUS(status);
		} else if (WIFSIGNALED(status)) {
			*exit_code = 128 + WTERMSIG(status);
		} else {
			*exit_code = -1;
		}
	}

	char *result = ape_sb_to_str_dup(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb);
	ape_sb_free(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb);
	return result;
}

APEBUILD_DEF int ape_cmds_run_seq(ApeCmdList *cmds)
{
	for (size_t i = 0; i < cmds->count; i++) {
		if (!ape_cmd_run(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYcmds->items[i])) {
			return APEBUILD_FALSE;
		}
	}
	return APEBUILD_TRUE;
}

APEBUILD_DEF int ape_cmds_run_all(ApeCmdList *cmds)
{
	int all_success = APEBUILD_TRUE;
	for (size_t i = 0; i < cmds->count; i++) {
		if (!ape_cmd_run(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYcmds->items[i])) {
			all_success = APEBUILD_FALSE;
		}
	}
	return all_success;
}

/* ============================================================================
 * Asynchronous Execution
 * ============================================================================ */

APEBUILD_DEF ApeProcHandle ape_cmd_start(ApeCmd *cmd)
{
	if (cmd->count == 0)
		return APE_INVALID_HANDLE;

	pid_t pid = fork();
	if (pid < 0) {
		return APE_INVALID_HANDLE;
	}

	if (pid == 0) {
		/* Child process */
		if (cmd->cwd) {
			if (chdir(cmd->cwd) != 0) {
				_exit(127);
			}
		}

		/* Set environment variables */
		for (size_t i = 0; i < cmd->env.count; i++) {
			putenv(cmd->env.items[i]);
		}

		/* Build argv with NULL terminator */
		const char **argv = (const char **)APEBUILD_MALLOC((cmd->count + 1) * sizeof(char *));
		for (size_t i = 0; i < cmd->count; i++) {
			argv[i] = cmd->items[i];
		}
		argv[cmd->count] = NULL;

		execvp(argv[0], (char *const *)argv);
		_exit(127);
	}

	return ape_proc_alloc(pid);
}

APEBUILD_DEF int ape_proc_poll(ApeProcHandle handle)
{
	ApeProcEntry *entry = ape_proc_get(handle);
	if (!entry)
		return APEBUILD_FALSE;

	if (entry->status != APE_PROC_RUNNING)
		return APEBUILD_TRUE;

	int status;
	pid_t result = waitpid(entry->pid, #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYstatus, WNOHANG);

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

APEBUILD_DEF int ape_proc_wait(ApeProcHandle handle)
{
	ApeProcEntry *entry = ape_proc_get(handle);
	if (!entry)
		return APEBUILD_FALSE;

	if (entry->status != APE_PROC_RUNNING)
		return APEBUILD_TRUE;

	int status;
	pid_t result = waitpid(entry->pid, #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYstatus, 0);

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

APEBUILD_DEF int ape_proc_wait_timeout(ApeProcHandle handle, int timeout_ms)
{
	ApeProcEntry *entry = ape_proc_get(handle);
	if (!entry)
		return APEBUILD_FALSE;

	if (entry->status != APE_PROC_RUNNING)
		return APEBUILD_TRUE;

	/* Simple polling implementation */
	int elapsed = 0;
	int sleep_interval = 10; /* 10ms */

	while (elapsed < timeout_ms) {
		if (ape_proc_poll(handle)) {
			return APEBUILD_TRUE;
		}
		usleep(sleep_interval * 1000);
		elapsed += sleep_interval;
	}

	return APEBUILD_FALSE; /* Timeout */
}

APEBUILD_DEF ApeProcStatus ape_proc_status(ApeProcHandle handle)
{
	ApeProcEntry *entry = ape_proc_get(handle);
	if (!entry)
		return APE_PROC_UNKNOWN;

	/* Update status if still running */
	if (entry->status == APE_PROC_RUNNING) {
		ape_proc_poll(handle);
	}

	return entry->status;
}

APEBUILD_DEF ApeProcResult ape_proc_result(ApeProcHandle handle)
{
	ApeProcResult result = { .status = APE_PROC_UNKNOWN, .exit_code = -1, .signal = 0 };

	ApeProcEntry *entry = ape_proc_get(handle);
	if (!entry)
		return result;

	result.status = entry->status;
	result.exit_code = entry->exit_code;
	result.signal = entry->signal;
	return result;
}

APEBUILD_DEF int ape_proc_kill(ApeProcHandle handle)
{
	ApeProcEntry *entry = ape_proc_get(handle);
	if (!entry)
		return APEBUILD_FALSE;

	if (entry->status != APE_PROC_RUNNING)
		return APEBUILD_TRUE;

	if (kill(entry->pid, SIGTERM) == 0) {
		/* Give it a moment to terminate gracefully */
		usleep(100000); /* 100ms */
		if (ape_proc_poll(handle)) {
			return APEBUILD_TRUE;
		}
		/* Force kill */
		kill(entry->pid, SIGKILL);
		ape_proc_wait(handle);
		return APEBUILD_TRUE;
	}

	return APEBUILD_FALSE;
}

APEBUILD_DEF int ape_proc_handle_valid(ApeProcHandle handle)
{
	return ape_proc_get(handle) != NULL ? APEBUILD_TRUE : APEBUILD_FALSE;
}

APEBUILD_DEF void ape_proc_handle_release(ApeProcHandle handle)
{
	ApeProcEntry *entry = ape_proc_get(handle);
	if (entry) {
		entry->in_use = 0;
	}
}

/* ============================================================================
 * Parallel Execution
 * ============================================================================ */

APEBUILD_DEF ApeProcPool *ape_pool_new(int max_parallel)
{
	ApeProcPool *pool = (ApeProcPool *)APEBUILD_MALLOC(sizeof(ApeProcPool));
	pool->max_parallel = max_parallel > 0 ? max_parallel : 1;
	pool->handles = NULL;
	pool->count = 0;
	pool->capacity = 0;
	return pool;
}

APEBUILD_DEF int ape_pool_submit(ApeProcPool *pool, ApeCmd *cmd)
{
	/* Wait if at capacity */
	while (pool->count >= pool->max_parallel) {
		ApeProcHandle finished = ape_pool_wait_any(pool);
		if (finished == APE_INVALID_HANDLE) {
			return APEBUILD_FALSE;
		}
	}

	ApeProcHandle handle = ape_cmd_start(cmd);
	if (handle == APE_INVALID_HANDLE) {
		return APEBUILD_FALSE;
	}

	/* Add to pool */
	if (pool->count >= pool->capacity) {
		pool->capacity = pool->capacity == 0 ? 16 : pool->capacity * 2;
		pool->handles = (ApeProcHandle *)APEBUILD_REALLOC(pool->handles, pool->capacity * sizeof(ApeProcHandle));
	}
	pool->handles[pool->count++] = handle;

	return APEBUILD_TRUE;
}

APEBUILD_DEF ApeProcHandle ape_pool_wait_any(ApeProcPool *pool)
{
	if (pool->count == 0)
		return APE_INVALID_HANDLE;

	while (1) {
		for (int i = 0; i < pool->count; i++) {
			if (ape_proc_poll(pool->handles[i])) {
				ApeProcHandle handle = pool->handles[i];
				/* Remove from pool */
				for (int j = i; j < pool->count - 1; j++) {
					pool->handles[j] = pool->handles[j + 1];
				}
				pool->count--;
				return handle;
			}
		}
		usleep(10000); /* 10ms */
	}
}

APEBUILD_DEF int ape_pool_wait_all(ApeProcPool *pool)
{
	int all_success = APEBUILD_TRUE;

	while (pool->count > 0) {
		ApeProcHandle handle = ape_pool_wait_any(pool);
		if (handle == APE_INVALID_HANDLE) {
			all_success = APEBUILD_FALSE;
			continue;
		}

		ApeProcResult result = ape_proc_result(handle);
		if (result.status != APE_PROC_COMPLETED || result.exit_code != 0) {
			all_success = APEBUILD_FALSE;
		}
		ape_proc_handle_release(handle);
	}

	return all_success;
}

APEBUILD_DEF void ape_pool_free(ApeProcPool *pool)
{
	if (!pool)
		return;

	/* Kill any remaining processes */
	for (int i = 0; i < pool->count; i++) {
		ape_proc_kill(pool->handles[i]);
		ape_proc_handle_release(pool->handles[i]);
	}

	APEBUILD_FREE(pool->handles);
	APEBUILD_FREE(pool);
}

APEBUILD_DEF int ape_cmds_run_parallel(ApeCmdList *cmds, int max_parallel)
{
	ApeProcPool *pool = ape_pool_new(max_parallel);

	for (size_t i = 0; i < cmds->count; i++) {
		if (!ape_pool_submit(pool, #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYcmds->items[i])) {
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

APEBUILD_DEF void ape_cmd_set_env(ApeCmd *cmd, const char *name, const char *value)
{
	ApeStrBuilder sb = ape_sb_new();
	ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, name);
	ape_sb_append_char(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, '=');
	ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, value);
	ape_sl_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYcmd->env,
		      ape_sb_to_str_dup(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb));
	ape_sb_free(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb);
}

APEBUILD_DEF void ape_cmd_clear_env(ApeCmd *cmd)
{
	ape_sl_clear(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYcmd->env);
}

APEBUILD_DEF void ape_cmd_set_cwd(ApeCmd *cmd, const char *cwd)
{
	APEBUILD_FREE(cmd->cwd);
	cmd->cwd = cwd ? ape_str_dup(cwd) : NULL;
}

/* ============================================================================
 * Command List Operations
 * ============================================================================ */

APEBUILD_DEF void ape_cmdlist_init(ApeCmdList *list)
{
	list->capacity = 0;
	list->count = 0;
	list->items = NULL;
}

APEBUILD_DEF ApeCmdList ape_cmdlist_new(void)
{
	ApeCmdList list;
	ape_cmdlist_init(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYlist);
	return list;
}

APEBUILD_DEF void ape_cmdlist_append(ApeCmdList *list, ApeCmd cmd)
{
	ape_da_append(list, cmd);
}

APEBUILD_DEF void ape_cmdlist_free(ApeCmdList *list)
{
	for (size_t i = 0; i < list->count; i++) {
		ape_cmd_free(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYlist->items[i]);
	}
	APEBUILD_FREE(list->items);
	ape_cmdlist_init(list);
}
/* END ape_cmd.c */

/* BEGIN ape_fs.c */
/*
 * ape_fs.c - Filesystem module implementation
 */

#include "apebuild_internal.h"

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <limits.h>
#include <fnmatch.h>

/* Directory handle structure */
struct ApeDir {
	DIR *dir;
	char *path;
};

/* ============================================================================
 * File Reading/Writing
 * ============================================================================ */

APEBUILD_DEF char *ape_fs_read_file(const char *path, size_t *out_size)
{
	FILE *fp = fopen(path, "rb");
	if (!fp)
		return NULL;

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
	if (out_size)
		*out_size = read_size;

	return buffer;
}

APEBUILD_DEF ApeStrList ape_fs_read_file_lines(const char *path)
{
	char *content = ape_fs_read_file(path, NULL);
	if (!content)
		return ape_sl_new();

	ApeStrList result = ape_str_split_lines(content);
	APEBUILD_FREE(content);
	return result;
}

APEBUILD_DEF int ape_fs_write_file(const char *path, const char *data, size_t size)
{
	FILE *fp = fopen(path, "wb");
	if (!fp)
		return APEBUILD_FALSE;

	size_t written = fwrite(data, 1, size, fp);
	fclose(fp);

	return written == size ? APEBUILD_TRUE : APEBUILD_FALSE;
}

APEBUILD_DEF int ape_fs_append_file(const char *path, const char *data, size_t size)
{
	FILE *fp = fopen(path, "ab");
	if (!fp)
		return APEBUILD_FALSE;

	size_t written = fwrite(data, 1, size, fp);
	fclose(fp);

	return written == size ? APEBUILD_TRUE : APEBUILD_FALSE;
}

APEBUILD_DEF int ape_fs_copy_file(const char *src, const char *dst)
{
	size_t size;
	char *content = ape_fs_read_file(src, #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsize);
	if (!content)
		return APEBUILD_FALSE;

	int result = ape_fs_write_file(dst, content, size);
	APEBUILD_FREE(content);
	return result;
}

APEBUILD_DEF int ape_fs_rename(const char *oldpath, const char *newpath)
{
	return rename(oldpath, newpath) == 0 ? APEBUILD_TRUE : APEBUILD_FALSE;
}

/* ============================================================================
 * Directory Operations
 * ============================================================================ */

APEBUILD_DEF int ape_fs_mkdir(const char *path)
{
	return mkdir(path, 0755) == 0 ? APEBUILD_TRUE : APEBUILD_FALSE;
}

APEBUILD_DEF int ape_fs_mkdir_p(const char *path)
{
	char *tmp = ape_str_dup(path);
	size_t len = strlen(tmp);

	/* Remove trailing slash if present */
	if (len > 0 #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY tmp[len - 1] == '/')
		tmp[len - 1] = '\0';

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
	if (!ape_fs_exists(tmp)) {
		result = ape_fs_mkdir(tmp);
	}

	APEBUILD_FREE(tmp);
	return result;
}

APEBUILD_DEF int ape_fs_rmdir(const char *path)
{
	return rmdir(path) == 0 ? APEBUILD_TRUE : APEBUILD_FALSE;
}
APEBUILD_PRIVATE void ape_fs_rmdir_r_callback(const char *path, const ApeDirEntry *entry, void *userdata)
{
	(void)userdata;
	char *fullpath = ape_fs_join(path, entry->name);

	if (entry->is_dir) {
		ape_fs_rmdir_r(fullpath);
	} else {
		ape_fs_remove(fullpath);
	}

	APEBUILD_FREE(fullpath);
}
APEBUILD_DEF int ape_fs_rmdir_r(const char *path)
{
	ape_fs_iterdir(path, ape_fs_rmdir_r_callback, NULL);
	return ape_fs_rmdir(path);
}

APEBUILD_DEF int ape_fs_exists(const char *path)
{
	struct stat st;
	return stat(path, #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYst) == 0 ? APEBUILD_TRUE : APEBUILD_FALSE;
}

APEBUILD_DEF int ape_fs_is_file(const char *path)
{
	struct stat st;
	if (stat(path, #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYst) != 0)
		return APEBUILD_FALSE;
	return S_ISREG(st.st_mode) ? APEBUILD_TRUE : APEBUILD_FALSE;
}

APEBUILD_DEF int ape_fs_is_dir(const char *path)
{
	struct stat st;
	if (stat(path, #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYst) != 0)
		return APEBUILD_FALSE;
	return S_ISDIR(st.st_mode) ? APEBUILD_TRUE : APEBUILD_FALSE;
}

APEBUILD_DEF int ape_fs_remove(const char *path)
{
	return unlink(path) == 0 ? APEBUILD_TRUE : APEBUILD_FALSE;
}

/* ============================================================================
 * Directory Iteration
 * ============================================================================ */

APEBUILD_DEF ApeDir *ape_fs_opendir(const char *path)
{
	DIR *dir = opendir(path);
	if (!dir)
		return NULL;

	ApeDir *ape_dir = (ApeDir *)APEBUILD_MALLOC(sizeof(ApeDir));
	ape_dir->dir = dir;
	ape_dir->path = ape_str_dup(path);
	return ape_dir;
}

APEBUILD_DEF ApeDirEntry *ape_fs_readdir(ApeDir *dir)
{
	if (!dir || !dir->dir)
		return NULL;

	struct dirent *entry;
	while ((entry = readdir(dir->dir)) != NULL) {
		/* Skip . and .. */
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
			continue;
		}

		ApeDirEntry *result = (ApeDirEntry *)APEBUILD_MALLOC(sizeof(ApeDirEntry));
		result->name = ape_str_dup(entry->d_name);

		/* Get file type */
		char *fullpath = ape_fs_join(dir->path, entry->d_name);
		struct stat st;
		if (lstat(fullpath, #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYst) == 0) {
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

APEBUILD_DEF void ape_fs_closedir(ApeDir *dir)
{
	if (!dir)
		return;
	if (dir->dir)
		closedir(dir->dir);
	APEBUILD_FREE(dir->path);
	APEBUILD_FREE(dir);
}

APEBUILD_DEF void ape_fs_direntry_free(ApeDirEntry *entry)
{
	if (!entry)
		return;
	APEBUILD_FREE(entry->name);
	APEBUILD_FREE(entry);
}

APEBUILD_DEF int ape_fs_iterdir(const char *path, ApeDirCallback callback, void *userdata)
{
	ApeDir *dir = ape_fs_opendir(path);
	if (!dir)
		return APEBUILD_FALSE;

	ApeDirEntry *entry;
	while ((entry = ape_fs_readdir(dir)) != NULL) {
		callback(path, entry, userdata);
		ape_fs_direntry_free(entry);
	}

	ape_fs_closedir(dir);
	return APEBUILD_TRUE;
}

APEBUILD_PRIVATE int ape_fs_iterdir_r_helper(const char *path, ApeDirCallback callback, void *userdata)
{
	ApeDir *dir = ape_fs_opendir(path);
	if (!dir)
		return APEBUILD_FALSE;

	ApeDirEntry *entry;
	while ((entry = ape_fs_readdir(dir)) != NULL) {
		callback(path, entry, userdata);

		if (entry->is_dir #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY !entry
			    ->is_symlink) {
			char *subpath = ape_fs_join(path, entry->name);
			ape_fs_iterdir_r_helper(subpath, callback, userdata);
			APEBUILD_FREE(subpath);
		}

		ape_fs_direntry_free(entry);
	}

	ape_fs_closedir(dir);
	return APEBUILD_TRUE;
}

APEBUILD_DEF int ape_fs_iterdir_r(const char *path, ApeDirCallback callback, void *userdata)
{
	return ape_fs_iterdir_r_helper(path, callback, userdata);
}

typedef struct {
	const char *pattern;
	const char *base_path;
	ApeStrList *results;
} GlobContext;

APEBUILD_PRIVATE void ape_fs_glob_callback(const char *path, const ApeDirEntry *entry, void *userdata)
{
	GlobContext *ctx = (GlobContext *)userdata;
	char *fullpath = ape_fs_join(path, entry->name);

	/* Get path relative to base for matching */
	const char *rel_path = fullpath;
	size_t base_len = strlen(ctx->base_path);
	if (strncmp(fullpath, ctx->base_path, base_len) == 0) {
		rel_path = fullpath + base_len;
		if (*rel_path == '/')
			rel_path++;
	}

	if (fnmatch(ctx->pattern, rel_path, FNM_PATHNAME) == 0) {
		ape_sl_append(ctx->results, fullpath);
	} else {
		APEBUILD_FREE(fullpath);
	}
}

APEBUILD_DEF ApeStrList ape_fs_glob(const char *pattern)
{
	ApeStrList results = ape_sl_new();
	if (!pattern)
		return results;

	/* Find base directory (everything before first wildcard) */
	char *base_path = ape_str_dup(pattern);
	char *wildcard = strpbrk(base_path, "*?[");
	if (wildcard) {
		/* Find last slash before wildcard */
		char *last_slash = wildcard;
		while (last_slash >
			       base_path #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY *
				       last_slash !=
		       '/')
			last_slash--;
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

	GlobContext ctx = { .pattern = pattern,
			    .base_path = base_path,
			    .results = #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYresults };

	ape_fs_iterdir_r(base_path, ape_fs_glob_callback, #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYctx);

	APEBUILD_FREE(base_path);
	return results;
}

/* ============================================================================
 * File Metadata
 * ============================================================================ */

APEBUILD_DEF int ape_fs_stat(const char *path, ApeFileStat *out)
{
	struct stat st;
	if (stat(path, #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYst) != 0)
		return APEBUILD_FALSE;

	out->size = st.st_size;
	out->mtime = st.st_mtime;
	out->atime = st.st_atime;
	out->ctime = st.st_ctime;
	out->is_dir = S_ISDIR(st.st_mode) ? 1 : 0;
	out->is_file = S_ISREG(st.st_mode) ? 1 : 0;
	out->is_symlink = S_ISLNK(st.st_mode) ? 1 : 0;
	out->mode = st.st_mode #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY 0777;

	return APEBUILD_TRUE;
}

APEBUILD_DEF time_t ape_fs_mtime(const char *path)
{
	struct stat st;
	if (stat(path, #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYst) != 0)
		return 0;
	return st.st_mtime;
}

APEBUILD_DEF size_t ape_fs_size(const char *path)
{
	struct stat st;
	if (stat(path, #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYst) != 0)
		return 0;
	return st.st_size;
}

APEBUILD_DEF int ape_fs_is_newer(const char *a, const char *b)
{
	time_t mtime_a = ape_fs_mtime(a);
	time_t mtime_b = ape_fs_mtime(b);
	return mtime_a > mtime_b ? APEBUILD_TRUE : APEBUILD_FALSE;
}

APEBUILD_DEF int ape_fs_needs_rebuild(const char *output, const char **inputs, size_t input_count)
{
	if (!ape_fs_exists(output))
		return APEBUILD_TRUE;

	time_t output_mtime = ape_fs_mtime(output);

	for (size_t i = 0; i < input_count; i++) {
		time_t input_mtime = ape_fs_mtime(inputs[i]);
		if (input_mtime > output_mtime)
			return APEBUILD_TRUE;
	}

	return APEBUILD_FALSE;
}

APEBUILD_DEF int ape_fs_needs_rebuild1(const char *output, const char *input)
{
	return ape_fs_needs_rebuild(output, #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYinput, 1);
}

/* ============================================================================
 * Path Manipulation
 * ============================================================================ */

APEBUILD_DEF char *ape_fs_join(const char *a, const char *b)
{
	if (!a || !*a)
		return ape_str_dup(b);
	if (!b || !*b)
		return ape_str_dup(a);

	size_t len_a = strlen(a);
	int has_slash = (a[len_a - 1] == '/');
	int b_has_slash = (b[0] == '/');

	ApeStrBuilder sb = ape_sb_new();
	ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, a);

	if (!has_slash #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY !b_has_slash) {
		ape_sb_append_char(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, '/');
	} else if (has_slash #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY b_has_slash) {
		b++; /* Skip leading slash of b */
	}

	ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, b);
	char *result = ape_sb_to_str_dup(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb);
	ape_sb_free(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb);
	return result;
}

APEBUILD_DEF char *ape_fs_dirname(const char *path)
{
	if (!path)
		return NULL;

	int last_slash = ape_str_rfind_char(path, '/');
	if (last_slash < 0)
		return ape_str_dup(".");
	if (last_slash == 0)
		return ape_str_dup("/");

	return ape_str_ndup(path, last_slash);
}

APEBUILD_DEF char *ape_fs_basename(const char *path)
{
	if (!path)
		return NULL;

	int last_slash = ape_str_rfind_char(path, '/');
	if (last_slash < 0)
		return ape_str_dup(path);

	return ape_str_dup(path + last_slash + 1);
}

APEBUILD_DEF char *ape_fs_extension(const char *path)
{
	if (!path)
		return NULL;

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

APEBUILD_DEF char *ape_fs_stem(const char *path)
{
	if (!path)
		return NULL;

	char *basename = ape_fs_basename(path);
	int dot = ape_str_rfind_char(basename, '.');
	if (dot <= 0) { /* No extension or hidden file */
		return basename;
	}

	char *stem = ape_str_ndup(basename, dot);
	APEBUILD_FREE(basename);
	return stem;
}

APEBUILD_DEF char *ape_fs_change_extension(const char *path, const char *new_ext)
{
	if (!path)
		return NULL;

	char *dir = ape_fs_dirname(path);
	char *stem = ape_fs_stem(path);

	ApeStrBuilder sb = ape_sb_new();
	if (strcmp(dir, ".") != 0) {
		ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, dir);
		ape_sb_append_char(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, '/');
	}
	ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, stem);
	if (new_ext #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY * new_ext) {
		if (*new_ext != '.')
			ape_sb_append_char(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, '.');
		ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, new_ext);
	}

	APEBUILD_FREE(dir);
	APEBUILD_FREE(stem);

	char *result = ape_sb_to_str_dup(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb);
	ape_sb_free(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb);
	return result;
}

APEBUILD_DEF char *ape_fs_normalize(const char *path)
{
	if (!path)
		return NULL;

	ApeStrList parts = ape_str_split(path, "/");
	ApeStrList result_parts = ape_sl_new();

	int is_absolute = (path[0] == '/');

	for (size_t i = 0; i < parts.count; i++) {
		const char *part = parts.items[i];

		if (strcmp(part, ".") == 0 || *part == '\0') {
			continue;
		} else if (strcmp(part, "..") == 0) {
			if (result_parts.count >
				    0 #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY
					    strcmp(ape_sl_get(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYresult_parts,
							      result_parts.count - 1),
						   "..") !=
			    0) {
				ape_sl_remove(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYresult_parts, result_parts.count - 1);
			} else if (!is_absolute) {
				ape_sl_append_dup(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYresult_parts, "..");
			}
		} else {
			ape_sl_append_dup(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYresult_parts, part);
		}
	}

	char *joined = ape_sl_join(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYresult_parts, "/");
	ApeStrBuilder sb = ape_sb_new();
	if (is_absolute)
		ape_sb_append_char(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, '/');
	ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, joined);

	if (sb.count == 0)
		ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, ".");

	APEBUILD_FREE(joined);
	ape_sl_free(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYparts);
	ape_sl_free(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYresult_parts);

	char *result = ape_sb_to_str_dup(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb);
	ape_sb_free(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb);
	return result;
}

APEBUILD_DEF char *ape_fs_absolute(const char *path)
{
	if (!path)
		return NULL;

	if (ape_fs_is_absolute(path)) {
		return ape_fs_normalize(path);
	}

	char *cwd = ape_fs_cwd();
	char *joined = ape_fs_join(cwd, path);
	char *result = ape_fs_normalize(joined);

	APEBUILD_FREE(cwd);
	APEBUILD_FREE(joined);
	return result;
}

APEBUILD_DEF char *ape_fs_relative(const char *from, const char *to)
{
	if (!from || !to)
		return NULL;

	char *abs_from = ape_fs_absolute(from);
	char *abs_to = ape_fs_absolute(to);

	ApeStrList from_parts = ape_str_split(abs_from, "/");
	ApeStrList to_parts = ape_str_split(abs_to, "/");

	/* Find common prefix */
	size_t common = 0;
	while (common <
	       from_parts.count #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY common <
	       to_parts.count #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY ape_str_eq(
		       from_parts.items[common], to_parts.items[common])) {
		common++;
	}

	ApeStrBuilder sb = ape_sb_new();

	/* Add ".." for each remaining from part */
	for (size_t i = common; i < from_parts.count; i++) {
		if (sb.count > 0)
			ape_sb_append_char(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, '/');
		ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, "..");
	}

	/* Add remaining to parts */
	for (size_t i = common; i < to_parts.count; i++) {
		if (sb.count > 0)
			ape_sb_append_char(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, '/');
		ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, to_parts.items[i]);
	}

	if (sb.count == 0)
		ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, ".");

	APEBUILD_FREE(abs_from);
	APEBUILD_FREE(abs_to);
	ape_sl_free(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYfrom_parts);
	ape_sl_free(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYto_parts);

	char *result = ape_sb_to_str_dup(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb);
	ape_sb_free(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb);
	return result;
}

APEBUILD_DEF int ape_fs_is_absolute(const char *path)
{
	return path #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY path[0] == '/' ?
		       APEBUILD_TRUE :
		       APEBUILD_FALSE;
}

/* ============================================================================
 * Temporary Files
 * ============================================================================ */

APEBUILD_DEF const char *ape_fs_temp_dir(void)
{
	const char *tmp = getenv("TMPDIR");
	if (tmp)
		return tmp;
	tmp = getenv("TMP");
	if (tmp)
		return tmp;
	tmp = getenv("TEMP");
	if (tmp)
		return tmp;
	return "/tmp";
}

APEBUILD_DEF char *ape_fs_temp_file(const char *prefix)
{
	const char *tmpdir = ape_fs_temp_dir();
	if (!prefix)
		prefix = "ape";

	ApeStrBuilder sb = ape_sb_new();
	ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, tmpdir);
	ape_sb_append_char(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, '/');
	ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, prefix);
	ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, "XXXXXX");

	char *template = ape_sb_to_str_dup(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb);
	ape_sb_free(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb);

	int fd = mkstemp(template);
	if (fd < 0) {
		APEBUILD_FREE(template);
		return NULL;
	}
	close(fd);

	return template;
}

APEBUILD_DEF char *ape_fs_temp_mkdir(const char *prefix)
{
	const char *tmpdir = ape_fs_temp_dir();
	if (!prefix)
		prefix = "ape";

	ApeStrBuilder sb = ape_sb_new();
	ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, tmpdir);
	ape_sb_append_char(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, '/');
	ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, prefix);
	ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, "XXXXXX");

	char *template = ape_sb_to_str_dup(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb);
	ape_sb_free(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb);

	if (mkdtemp(template) == NULL) {
		APEBUILD_FREE(template);
		return NULL;
	}

	return template;
}

/* ============================================================================
 * Utility
 * ============================================================================ */

APEBUILD_DEF char *ape_fs_cwd(void)
{
	char *buf = (char *)APEBUILD_MALLOC(PATH_MAX);
	if (getcwd(buf, PATH_MAX) == NULL) {
		APEBUILD_FREE(buf);
		return NULL;
	}
	return buf;
}

APEBUILD_DEF int ape_fs_chdir(const char *path)
{
	return chdir(path) == 0 ? APEBUILD_TRUE : APEBUILD_FALSE;
}

APEBUILD_DEF char *ape_fs_home(void)
{
	const char *home = getenv("HOME");
	if (home)
		return ape_str_dup(home);
	return NULL;
}
/* END ape_fs.c */

/* BEGIN ape_log.c */
/*
 * ape_log.c - Logging module implementation
 */

#include "apebuild_internal.h"

#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

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
	"\033[90m", /* TRACE - gray */
	"\033[36m", /* DEBUG - cyan */
	"\033[32m", /* INFO  - green */
	"\033[33m", /* WARN  - yellow */
	"\033[31m", /* ERROR - red */
	"\033[35;1m", /* FATAL - bold magenta */
};

APEBUILD_PRIVATE const char *ape_log_reset = "\033[0m";

APEBUILD_PRIVATE const char *ape_log_level_names[] = { "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL", "OFF" };

/* ============================================================================
 * Initialization
 * ============================================================================ */

APEBUILD_DEF void ape_log_init(void)
{
	if (ape_log_initialized)
		return;

	ape_log_config.output = stderr;

	/* Auto-detect color support */
	if (isatty(STDERR_FILENO)) {
		const char *term = getenv("TERM");
		if (term #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY strcmp(
			    term, "dumb") != 0) {
			ape_log_config.use_colors = 1;
		} else {
			ape_log_config.use_colors = 0;
		}
	} else {
		ape_log_config.use_colors = 0;
	}

	ape_log_initialized = 1;
}

APEBUILD_DEF void ape_log_shutdown(void)
{
	if (ape_log_config.file_output) {
		fclose(ape_log_config.file_output);
		ape_log_config.file_output = NULL;
	}
	ape_log_initialized = 0;
}

/* ============================================================================
 * Configuration
 * ============================================================================ */

APEBUILD_DEF void ape_log_set_level(ApeLogLevel level)
{
	ape_log_config.level = level;
}

APEBUILD_DEF ApeLogLevel ape_log_get_level(void)
{
	return ape_log_config.level;
}

APEBUILD_DEF void ape_log_set_output(FILE *fp)
{
	ape_log_config.output = fp;
}

APEBUILD_DEF int ape_log_set_file(const char *path)
{
	if (ape_log_config.file_output) {
		fclose(ape_log_config.file_output);
		ape_log_config.file_output = NULL;
	}

	if (!path)
		return APEBUILD_TRUE;

	ape_log_config.file_output = fopen(path, "a");
	return ape_log_config.file_output != NULL ? APEBUILD_TRUE : APEBUILD_FALSE;
}

APEBUILD_DEF void ape_log_set_colors(int enabled)
{
	ape_log_config.use_colors = enabled;
}

APEBUILD_DEF void ape_log_set_timestamps(int enabled)
{
	ape_log_config.show_timestamps = enabled;
}

APEBUILD_DEF void ape_log_set_show_level(int enabled)
{
	ape_log_config.show_level = enabled;
}

APEBUILD_DEF void ape_log_set_show_file(int enabled)
{
	ape_log_config.show_file = enabled;
}

APEBUILD_DEF void ape_log_set_prefix(const char *prefix)
{
	ape_log_config.prefix = prefix;
}

APEBUILD_DEF void ape_log_set_quiet(int quiet)
{
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
					    int use_colors)
{
	/* Timestamp */
	if (ape_log_config.show_timestamps) {
		time_t now = time(NULL);
		struct tm *tm_info = localtime(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYnow);
		char time_buf[32];
		strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", tm_info);
		fprintf(fp, "[%s] ", time_buf);
	}

	/* Prefix */
	if (ape_log_config.prefix) {
		fprintf(fp, "%s", ape_log_config.prefix);
	}

	/* Level */
	if (ape_log_config.show_level) {
		if (use_colors #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY level <
		    APE_LOG_OFF) {
			fprintf(fp, "%s%-5s%s ", ape_log_colors[level], ape_log_level_names[level], ape_log_reset);
		} else {
			fprintf(fp, "%-5s ", ape_log_level_names[level]);
		}
	}

	/* File:line */
	if (ape_log_config.show_file #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY file) {
		/* Extract just the filename */
		const char *filename = file;
		const char *slash = strrchr(file, '/');
		if (slash)
			filename = slash + 1;
		fprintf(fp, "%s:%d: ", filename, line);
	}

	/* Message */
	vfprintf(fp, fmt, args);
	fprintf(fp, "\n");
}

APEBUILD_DEF void ape_log_writev(ApeLogLevel level, const char *file, int line, const char *fmt, va_list args)
{
	if (!ape_log_initialized)
		ape_log_init();

	if (level < ape_log_config.level)
		return;

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

APEBUILD_DEF void ape_log_write(ApeLogLevel level, const char *file, int line, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	ape_log_writev(level, file, line, fmt, args);
	va_end(args);
}

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

APEBUILD_DEF const char *ape_log_level_name(ApeLogLevel level)
{
	if (level < 0 || level > APE_LOG_OFF)
		return "UNKNOWN";
	return ape_log_level_names[level];
}

APEBUILD_DEF int ape_log_level_from_name(const char *name, ApeLogLevel *out)
{
	if (!name || !out)
		return APEBUILD_FALSE;

	for (int i = 0; i <= APE_LOG_OFF; i++) {
		if (ape_str_eq_nocase(name, ape_log_level_names[i])) {
			*out = (ApeLogLevel)i;
			return APEBUILD_TRUE;
		}
	}

	return APEBUILD_FALSE;
}

APEBUILD_DEF void ape_log_flush(void)
{
	if (ape_log_config.output) {
		fflush(ape_log_config.output);
	}
	if (ape_log_config.file_output) {
		fflush(ape_log_config.file_output);
	}
}

/* ============================================================================
 * Build System Specific Logging
 * ============================================================================ */

APEBUILD_PRIVATE void ape_log_build_msg(const char *prefix, const char *color, const char *fmt, va_list args)
{
	if (!ape_log_initialized)
		ape_log_init();

	FILE *fp = ape_log_config.output;
	if (!fp)
		return;

	if (ape_log_config
		    .use_colors #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY color) {
		fprintf(fp, "%s%s%s ", color, prefix, ape_log_reset);
	} else {
		fprintf(fp, "%s ", prefix);
	}

	vfprintf(fp, fmt, args);
	fprintf(fp, "\n");
}

APEBUILD_DEF void ape_log_cmd(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	ape_log_build_msg("CMD:", "\033[36m", fmt, args); /* cyan */
	va_end(args);
}

APEBUILD_DEF void ape_log_build(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	ape_log_build_msg("BUILD:", "\033[34m", fmt, args); /* blue */
	va_end(args);
}

APEBUILD_DEF void ape_log_link(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	ape_log_build_msg("LINK:", "\033[35m", fmt, args); /* magenta */
	va_end(args);
}

APEBUILD_DEF void ape_log_success(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	ape_log_build_msg("OK:", "\033[32m", fmt, args); /* green */
	va_end(args);
}

APEBUILD_DEF void ape_log_failure(const char *fmt, ...)
{
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

#include "apebuild_internal.h"

#include <string.h>
#include <stdio.h>
#include <ctype.h>

/* ============================================================================
 * String Builder Implementation
 * ============================================================================ */

APEBUILD_DEF void ape_sb_init(ApeStrBuilder *sb)
{
	sb->capacity = 0;
	sb->count = 0;
	sb->items = NULL;
}

APEBUILD_DEF ApeStrBuilder ape_sb_new(void)
{
	ApeStrBuilder sb;
	ape_sb_init(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb);
	return sb;
}

APEBUILD_DEF ApeStrBuilder ape_sb_new_cap(size_t cap)
{
	ApeStrBuilder sb;
	sb.capacity = cap;
	sb.count = 0;
	sb.items = (char *)APEBUILD_MALLOC(cap);
	return sb;
}

APEBUILD_DEF void ape_sb_free(ApeStrBuilder *sb)
{
	APEBUILD_FREE(sb->items);
	ape_sb_init(sb);
}

APEBUILD_DEF void ape_sb_clear(ApeStrBuilder *sb)
{
	sb->count = 0;
}

APEBUILD_DEF void ape_sb_append_char(ApeStrBuilder *sb, char c)
{
	ape_da_append(sb, c);
}

APEBUILD_DEF void ape_sb_append_str(ApeStrBuilder *sb, const char *str)
{
	if (!str)
		return;
	size_t len = strlen(str);
	ape_da_append_many(sb, str, len);
}

APEBUILD_DEF void ape_sb_append_strn(ApeStrBuilder *sb, const char *str, size_t n)
{
	if (!str)
		return;
	ape_da_append_many(sb, str, n);
}

APEBUILD_DEF void ape_sb_append_sb(ApeStrBuilder *sb, const ApeStrBuilder *other)
{
	if (!other || !other->items)
		return;
	ape_da_append_many(sb, other->items, other->count);
}

APEBUILD_DEF void ape_sb_append_fmtv(ApeStrBuilder *sb, const char *fmt, va_list args)
{
	va_list args_copy;
	va_copy(args_copy, args);
	int needed = vsnprintf(NULL, 0, fmt, args_copy);
	va_end(args_copy);

	if (needed < 0)
		return;

	ape_da_reserve(sb, sb->count + needed + 1);
	vsnprintf(sb->items + sb->count, needed + 1, fmt, args);
	sb->count += needed;
}

APEBUILD_DEF void ape_sb_append_fmt(ApeStrBuilder *sb, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	ape_sb_append_fmtv(sb, fmt, args);
	va_end(args);
}

APEBUILD_DEF void ape_sb_prepend_str(ApeStrBuilder *sb, const char *str)
{
	if (!str)
		return;
	size_t len = strlen(str);
	if (sb->count + len >= sb->capacity) {
		size_t new_cap = sb->capacity == 0 ? APEBUILD_INIT_CAP : sb->capacity;
		while (sb->count + len >= new_cap)
			new_cap *= 2;
		sb->items = (char *)APEBUILD_REALLOC(sb->items, new_cap);
		sb->capacity = new_cap;
	}
	memmove(sb->items + len, sb->items, sb->count);
	memcpy(sb->items, str, len);
	sb->count += len;
}

APEBUILD_DEF void ape_sb_insert(ApeStrBuilder *sb, size_t pos, const char *str)
{
	if (!str)
		return;
	if (pos > sb->count)
		pos = sb->count;
	size_t len = strlen(str);
	if (sb->count + len >= sb->capacity) {
		size_t new_cap = sb->capacity == 0 ? APEBUILD_INIT_CAP : sb->capacity;
		while (sb->count + len >= new_cap)
			new_cap *= 2;
		sb->items = (char *)APEBUILD_REALLOC(sb->items, new_cap);
		sb->capacity = new_cap;
	}
	memmove(sb->items + pos + len, sb->items + pos, sb->count - pos);
	memcpy(sb->items + pos, str, len);
	sb->count += len;
}

APEBUILD_DEF const char *ape_sb_to_str(ApeStrBuilder *sb)
{
	ape_da_append(sb, '\0');
	sb->count--; /* Don't count null terminator in length */
	return sb->items;
}

APEBUILD_DEF char *ape_sb_to_str_dup(const ApeStrBuilder *sb)
{
	char *result = (char *)APEBUILD_MALLOC(sb->count + 1);
	if (sb->items)
		memcpy(result, sb->items, sb->count);
	result[sb->count] = '\0';
	return result;
}

APEBUILD_DEF size_t ape_sb_len(const ApeStrBuilder *sb)
{
	return sb->count;
}

APEBUILD_DEF size_t ape_sb_capacity(const ApeStrBuilder *sb)
{
	return sb->capacity;
}

APEBUILD_DEF void ape_sb_reserve(ApeStrBuilder *sb, size_t cap)
{
	ape_da_reserve(sb, cap);
}

APEBUILD_DEF void ape_sb_shrink(ApeStrBuilder *sb)
{
	ape_da_shrink(sb);
}

/* ============================================================================
 * String List Implementation
 * ============================================================================ */

APEBUILD_DEF void ape_sl_init(ApeStrList *sl)
{
	sl->capacity = 0;
	sl->count = 0;
	sl->items = NULL;
}

APEBUILD_DEF ApeStrList ape_sl_new(void)
{
	ApeStrList sl;
	ape_sl_init(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsl);
	return sl;
}

APEBUILD_DEF void ape_sl_free(ApeStrList *sl)
{
	for (size_t i = 0; i < sl->count; i++) {
		APEBUILD_FREE(sl->items[i]);
	}
	APEBUILD_FREE(sl->items);
	ape_sl_init(sl);
}

APEBUILD_DEF void ape_sl_free_shallow(ApeStrList *sl)
{
	APEBUILD_FREE(sl->items);
	ape_sl_init(sl);
}

APEBUILD_DEF void ape_sl_clear(ApeStrList *sl)
{
	for (size_t i = 0; i < sl->count; i++) {
		APEBUILD_FREE(sl->items[i]);
	}
	sl->count = 0;
}

APEBUILD_DEF void ape_sl_append(ApeStrList *sl, char *str)
{
	ape_da_append(sl, str);
}

APEBUILD_DEF void ape_sl_append_dup(ApeStrList *sl, const char *str)
{
	ape_da_append(sl, ape_str_dup(str));
}

APEBUILD_DEF void ape_sl_append_many(ApeStrList *sl, char **strs, size_t n)
{
	ape_da_append_many(sl, strs, n);
}

APEBUILD_DEF void ape_sl_prepend(ApeStrList *sl, char *str)
{
	ape_da_prepend(sl, str);
}

APEBUILD_DEF void ape_sl_insert(ApeStrList *sl, size_t index, char *str)
{
	ape_da_insert(sl, index, str);
}

APEBUILD_DEF void ape_sl_remove(ApeStrList *sl, size_t index)
{
	if (index < sl->count) {
		APEBUILD_FREE(sl->items[index]);
		ape_da_remove(sl, index);
	}
}

APEBUILD_DEF char *ape_sl_get(const ApeStrList *sl, size_t index)
{
	if (index >= sl->count)
		return NULL;
	return sl->items[index];
}

APEBUILD_DEF size_t ape_sl_len(const ApeStrList *sl)
{
	return sl->count;
}

APEBUILD_DEF int ape_sl_contains(const ApeStrList *sl, const char *str)
{
	return ape_sl_index_of(sl, str) >= 0;
}

APEBUILD_DEF int ape_sl_index_of(const ApeStrList *sl, const char *str)
{
	for (size_t i = 0; i < sl->count; i++) {
		if (ape_str_eq(sl->items[i], str))
			return (int)i;
	}
	return -1;
}

APEBUILD_DEF char *ape_sl_join(const ApeStrList *sl, const char *sep)
{
	if (sl->count == 0)
		return ape_str_dup("");

	ApeStrBuilder sb = ape_sb_new();
	for (size_t i = 0; i < sl->count; i++) {
		if (i > 0 #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY sep)
			ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, sep);
		ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, sl->items[i]);
	}
	char *result = ape_sb_to_str_dup(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb);
	ape_sb_free(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb);
	return result;
}

APEBUILD_DEF ApeStrList ape_sl_clone(const ApeStrList *sl)
{
	ApeStrList result = ape_sl_new();
	for (size_t i = 0; i < sl->count; i++) {
		ape_sl_append_dup(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYresult, sl->items[i]);
	}
	return result;
}

/* ============================================================================
 * String Utilities Implementation
 * ============================================================================ */

APEBUILD_DEF char *ape_str_dup(const char *str)
{
	if (!str)
		return NULL;
	size_t len = strlen(str);
	char *result = (char *)APEBUILD_MALLOC(len + 1);
	memcpy(result, str, len + 1);
	return result;
}

APEBUILD_DEF char *ape_str_ndup(const char *str, size_t n)
{
	if (!str)
		return NULL;
	size_t len = strlen(str);
	if (n > len)
		n = len;
	char *result = (char *)APEBUILD_MALLOC(n + 1);
	memcpy(result, str, n);
	result[n] = '\0';
	return result;
}

APEBUILD_DEF char *ape_str_concat(const char *a, const char *b)
{
	if (!a)
		a = "";
	if (!b)
		b = "";
	size_t len_a = strlen(a);
	size_t len_b = strlen(b);
	char *result = (char *)APEBUILD_MALLOC(len_a + len_b + 1);
	memcpy(result, a, len_a);
	memcpy(result + len_a, b, len_b + 1);
	return result;
}

APEBUILD_DEF char *ape_str_join(const char **strs, size_t count, const char *sep)
{
	if (count == 0)
		return ape_str_dup("");

	ApeStrBuilder sb = ape_sb_new();
	for (size_t i = 0; i < count; i++) {
		if (i > 0 #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY sep)
			ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, sep);
		ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, strs[i]);
	}
	char *result = ape_sb_to_str_dup(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb);
	ape_sb_free(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb);
	return result;
}

APEBUILD_DEF ApeStrList ape_str_split(const char *str, const char *delim)
{
	ApeStrList result = ape_sl_new();
	if (!str || !delim)
		return result;

	size_t delim_len = strlen(delim);
	if (delim_len == 0) {
		ape_sl_append_dup(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYresult, str);
		return result;
	}

	const char *start = str;
	const char *found;
	while ((found = strstr(start, delim)) != NULL) {
		ape_sl_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYresult, ape_str_ndup(start, found - start));
		start = found + delim_len;
	}
	ape_sl_append_dup(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYresult, start);
	return result;
}

APEBUILD_DEF ApeStrList ape_str_split_lines(const char *str)
{
	ApeStrList result = ape_sl_new();
	if (!str)
		return result;

	const char *start = str;
	const char *p = str;
	while (*p) {
		if (*p == '\n') {
			size_t len = p - start;
			if (len > 0 #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY
					    start[len - 1] ==
			    '\r')
				len--;
			ape_sl_append(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYresult, ape_str_ndup(start, len));
			start = p + 1;
		}
		p++;
	}
	if (start < p) {
		ape_sl_append_dup(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYresult, start);
	}
	return result;
}

APEBUILD_DEF int ape_str_eq(const char *a, const char *b)
{
	if (a == b)
		return 1;
	if (!a || !b)
		return 0;
	return strcmp(a, b) == 0;
}

APEBUILD_DEF int ape_str_eq_nocase(const char *a, const char *b)
{
	if (a == b)
		return 1;
	if (!a || !b)
		return 0;
	while (*a #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY * b) {
		if (tolower((unsigned char)*a) != tolower((unsigned char)*b))
			return 0;
		a++;
		b++;
	}
	return *a == *b;
}

APEBUILD_DEF int ape_str_starts_with(const char *str, const char *prefix)
{
	if (!str || !prefix)
		return 0;
	size_t len_str = strlen(str);
	size_t len_prefix = strlen(prefix);
	if (len_prefix > len_str)
		return 0;
	return strncmp(str, prefix, len_prefix) == 0;
}

APEBUILD_DEF int ape_str_ends_with(const char *str, const char *suffix)
{
	if (!str || !suffix)
		return 0;
	size_t len_str = strlen(str);
	size_t len_suffix = strlen(suffix);
	if (len_suffix > len_str)
		return 0;
	return strcmp(str + len_str - len_suffix, suffix) == 0;
}

APEBUILD_DEF int ape_str_contains(const char *str, const char *substr)
{
	if (!str || !substr)
		return 0;
	return strstr(str, substr) != NULL;
}

APEBUILD_DEF int ape_str_is_empty(const char *str)
{
	return !str || *str == '\0';
}

APEBUILD_DEF char *ape_str_trim(const char *str)
{
	if (!str)
		return NULL;
	while (*str #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY isspace(
		(unsigned char)*str))
		str++;
	if (*str == '\0')
		return ape_str_dup("");
	const char *end = str + strlen(str) - 1;
	while (end > str #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY isspace(
			     (unsigned char)*end))
		end--;
	return ape_str_ndup(str, end - str + 1);
}

APEBUILD_DEF char *ape_str_trim_left(const char *str)
{
	if (!str)
		return NULL;
	while (*str #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY isspace(
		(unsigned char)*str))
		str++;
	return ape_str_dup(str);
}

APEBUILD_DEF char *ape_str_trim_right(const char *str)
{
	if (!str)
		return NULL;
	size_t len = strlen(str);
	if (len == 0)
		return ape_str_dup("");
	const char *end = str + len - 1;
	while (end >= str #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY isspace(
			      (unsigned char)*end))
		end--;
	return ape_str_ndup(str, end - str + 1);
}

APEBUILD_DEF char *ape_str_to_lower(const char *str)
{
	if (!str)
		return NULL;
	char *result = ape_str_dup(str);
	for (char *p = result; *p; p++) {
		*p = (char)tolower((unsigned char)*p);
	}
	return result;
}

APEBUILD_DEF char *ape_str_to_upper(const char *str)
{
	if (!str)
		return NULL;
	char *result = ape_str_dup(str);
	for (char *p = result; *p; p++) {
		*p = (char)toupper((unsigned char)*p);
	}
	return result;
}

APEBUILD_DEF char *ape_str_replace(const char *str, const char *old, const char *new_str)
{
	if (!str || !old)
		return ape_str_dup(str);
	if (!new_str)
		new_str = "";

	const char *found = strstr(str, old);
	if (!found)
		return ape_str_dup(str);

	size_t old_len = strlen(old);
	ApeStrBuilder sb = ape_sb_new();
	ape_sb_append_strn(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, str, found - str);
	ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, new_str);
	ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, found + old_len);
	char *result = ape_sb_to_str_dup(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb);
	ape_sb_free(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb);
	return result;
}

APEBUILD_DEF char *ape_str_replace_all(const char *str, const char *old, const char *new_str)
{
	if (!str || !old || *old == '\0')
		return ape_str_dup(str);
	if (!new_str)
		new_str = "";

	ApeStrBuilder sb = ape_sb_new();
	size_t old_len = strlen(old);
	const char *start = str;
	const char *found;

	while ((found = strstr(start, old)) != NULL) {
		ape_sb_append_strn(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, start, found - start);
		ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, new_str);
		start = found + old_len;
	}
	ape_sb_append_str(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb, start);

	char *result = ape_sb_to_str_dup(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb);
	ape_sb_free(#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYsb);
	return result;
}

APEBUILD_DEF char *ape_str_substr(const char *str, size_t start, size_t len)
{
	if (!str)
		return NULL;
	size_t str_len = strlen(str);
	if (start >= str_len)
		return ape_str_dup("");
	if (start + len > str_len)
		len = str_len - start;
	return ape_str_ndup(str + start, len);
}

APEBUILD_DEF int ape_str_find(const char *str, const char *substr)
{
	if (!str || !substr)
		return -1;
	const char *found = strstr(str, substr);
	if (!found)
		return -1;
	return (int)(found - str);
}

APEBUILD_DEF int ape_str_find_char(const char *str, char c)
{
	if (!str)
		return -1;
	const char *found = strchr(str, c);
	if (!found)
		return -1;
	return (int)(found - str);
}

APEBUILD_DEF int ape_str_rfind(const char *str, const char *substr)
{
	if (!str || !substr)
		return -1;
	size_t str_len = strlen(str);
	size_t substr_len = strlen(substr);
	if (substr_len > str_len)
		return -1;

	for (size_t i = str_len - substr_len + 1; i > 0; i--) {
		if (strncmp(str + i - 1, substr, substr_len) == 0) {
			return (int)(i - 1);
		}
	}
	return -1;
}

APEBUILD_DEF int ape_str_rfind_char(const char *str, char c)
{
	if (!str)
		return -1;
	const char *found = strrchr(str, c);
	if (!found)
		return -1;
	return (int)(found - str);
}

APEBUILD_DEF int ape_str_to_int(const char *str, int *out)
{
	if (!str || !out)
		return 0;
	char *end;
	long val = strtol(str, #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYend, 10);
	if (end == str || *end != '\0')
		return 0;
	*out = (int)val;
	return 1;
}

APEBUILD_DEF int ape_str_to_long(const char *str, long *out)
{
	if (!str || !out)
		return 0;
	char *end;
	long val = strtol(str, #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYend, 10);
	if (end == str || *end != '\0')
		return 0;
	*out = val;
	return 1;
}

APEBUILD_DEF int ape_str_to_float(const char *str, float *out)
{
	if (!str || !out)
		return 0;
	char *end;
	float val = strtof(str, #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYend);
	if (end == str || *end != '\0')
		return 0;
	*out = val;
	return 1;
}

APEBUILD_DEF int ape_str_to_double(const char *str, double *out)
{
	if (!str || !out)
		return 0;
	char *end;
	double val = strtod(str, #define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFYend);
	if (end == str || *end != '\0')
		return 0;
	*out = val;
	return 1;
}

APEBUILD_DEF char *ape_str_from_int(int val)
{
	char buf[32];
	snprintf(buf, sizeof(buf), "%d", val);
	return ape_str_dup(buf);
}

APEBUILD_DEF char *ape_str_from_long(long val)
{
	char buf[32];
	snprintf(buf, sizeof(buf), "%ld", val);
	return ape_str_dup(buf);
}

APEBUILD_DEF char *ape_str_from_float(float val)
{
	char buf[64];
	snprintf(buf, sizeof(buf), "%g", val);
	return ape_str_dup(buf);
}
/* END ape_str.c */

#endif

#endif
