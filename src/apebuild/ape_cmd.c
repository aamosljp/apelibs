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
