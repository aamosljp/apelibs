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
