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

	ape_sl_init(&tc->default_cflags);
	ape_sl_init(&tc->default_ldflags);

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
	ape_sl_free(&tc->default_cflags);
	ape_sl_free(&tc->default_ldflags);
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

	clone->default_cflags = ape_sl_clone(&tc->default_cflags);
	clone->default_ldflags = ape_sl_clone(&tc->default_ldflags);

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
	ape_sl_append_dup(&tc->default_cflags, flag);
}

APEBUILD_DEF void ape_toolchain_add_ldflag(ApeToolchain *tc, const char *flag)
{
	ape_sl_append_dup(&tc->default_ldflags, flag);
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

	ape_sl_init(&task->inputs);
	ape_cmd_init(&task->cmd);
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
	ape_sl_free(&task->inputs);
	ape_cmd_free(&task->cmd);
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
	ape_sl_append_dup(&task->inputs, input);
}

APEBUILD_DEF void ape_task_add_dep(ApeTask *task, ApeTask *dep)
{
	ape_da_append(&task->deps, dep);
}

APEBUILD_DEF void ape_task_set_cmd(ApeTask *task, ApeCmd cmd)
{
	ape_cmd_free(&task->cmd);
	task->cmd = cmd;
}

APEBUILD_DEF int ape_task_needs_rebuild(ApeTask *task)
{
	ApeBuildCtx *ctx = task->builder->ctx;

	/* Force rebuild mode */
	if (ctx && ctx->force_rebuild)
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
		if (dep->status != APE_TASK_COMPLETED && dep->status != APE_TASK_SKIPPED) {
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

	ape_sl_init(&builder->sources);
	ape_sl_init(&builder->cflags);
	ape_sl_init(&builder->include_dirs);
	ape_sl_init(&builder->defines);
	ape_sl_init(&builder->ldflags);
	ape_sl_init(&builder->lib_dirs);
	ape_sl_init(&builder->libs);

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

	ape_sl_free(&builder->sources);
	ape_sl_free(&builder->cflags);
	ape_sl_free(&builder->include_dirs);
	ape_sl_free(&builder->defines);
	ape_sl_free(&builder->ldflags);
	ape_sl_free(&builder->lib_dirs);
	ape_sl_free(&builder->libs);

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
	ape_sl_append_dup(&builder->sources, path);
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
		ape_sl_append(&builder->sources, fullpath);
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
		ape_sl_append(&builder->sources, files.items[i]);
	}
	/* Free list but not strings (ownership transferred) */
	ape_sl_free_shallow(&files);
}

/* Compiler flags */

APEBUILD_DEF void ape_builder_add_cflag(ApeBuilder *builder, const char *flag)
{
	ape_sl_append_dup(&builder->cflags, flag);
}

APEBUILD_DEF void ape_builder_add_include(ApeBuilder *builder, const char *dir)
{
	ape_sl_append_dup(&builder->include_dirs, dir);
}

APEBUILD_DEF void ape_builder_add_define(ApeBuilder *builder, const char *define)
{
	ape_sl_append_dup(&builder->defines, define);
}

APEBUILD_DEF void ape_builder_add_define_value(ApeBuilder *builder, const char *name, const char *value)
{
	ApeStrBuilder sb = ape_sb_new();
	ape_sb_append_str(&sb, name);
	ape_sb_append_char(&sb, '=');
	ape_sb_append_str(&sb, value);
	ape_sl_append(&builder->defines, ape_sb_to_str_dup(&sb));
	ape_sb_free(&sb);
}

/* Linker flags */

APEBUILD_DEF void ape_builder_add_ldflag(ApeBuilder *builder, const char *flag)
{
	ape_sl_append_dup(&builder->ldflags, flag);
}

APEBUILD_DEF void ape_builder_add_lib_dir(ApeBuilder *builder, const char *dir)
{
	ape_sl_append_dup(&builder->lib_dirs, dir);
}

APEBUILD_DEF void ape_builder_add_lib(ApeBuilder *builder, const char *lib)
{
	ape_sl_append_dup(&builder->libs, lib);
}

/* Dependencies */

APEBUILD_DEF void ape_builder_depends_on(ApeBuilder *builder, ApeBuilder *dep)
{
	ape_da_append(&builder->deps, dep);
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
		if (tc && tc->lib_prefix && ape_str_starts_with(stem, tc->lib_prefix)) {
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
	if (!tc && builder->ctx)
		tc = builder->ctx->toolchain;
	if (!tc)
		return NULL;

	const char *output_dir = builder->output_dir;
	if (!output_dir && builder->ctx)
		output_dir = builder->ctx->output_dir;
	if (!output_dir)
		output_dir = "build";

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
	}

	char *result = ape_sb_to_str_dup(&sb);
	ape_sb_free(&sb);
	return result;
}

/* Task generation */

APEBUILD_DEF ApeTask *ape_builder_add_compile_task(ApeBuilder *builder, const char *source)
{
	char *obj_path = ape_build_obj_path(builder->ctx, builder, source);
	char *base = ape_fs_basename(source);

	ApeStrBuilder name_sb = ape_sb_new();
	ape_sb_append_str(&name_sb, "Compile ");
	ape_sb_append_str(&name_sb, base);

	ApeTask *task = ape_task_new(builder, APE_TASK_COMPILE, ape_sb_to_str(&name_sb));
	ape_sb_free(&name_sb);
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
		ape_cmd_append(&cmd, tc->cxx);
	} else {
		ape_cmd_append(&cmd, tc->cc);
	}

	/* Default flags from toolchain */
	for (size_t i = 0; i < tc->default_cflags.count; i++) {
		ape_cmd_append(&cmd, tc->default_cflags.items[i]);
	}

	/* Builder-specific flags */
	for (size_t i = 0; i < builder->cflags.count; i++) {
		ape_cmd_append(&cmd, builder->cflags.items[i]);
	}

	/* Include directories */
	for (size_t i = 0; i < builder->include_dirs.count; i++) {
		ApeStrBuilder inc = ape_sb_new();
		ape_sb_append_str(&inc, "-I");
		ape_sb_append_str(&inc, builder->include_dirs.items[i]);
		ape_cmd_append(&cmd, ape_sb_to_str(&inc));
		ape_sb_free(&inc);
	}

	/* Defines */
	for (size_t i = 0; i < builder->defines.count; i++) {
		ApeStrBuilder def = ape_sb_new();
		ape_sb_append_str(&def, "-D");
		ape_sb_append_str(&def, builder->defines.items[i]);
		ape_cmd_append(&cmd, ape_sb_to_str(&def));
		ape_sb_free(&def);
	}

	/* PIC for shared libraries */
	if (builder->type == APE_TARGET_SHARED_LIB) {
		ape_cmd_append(&cmd, "-fPIC");
	}

	/* Compile only */
	ape_cmd_append(&cmd, "-c");
	ape_cmd_append(&cmd, source);
	ape_cmd_append(&cmd, "-o");
	ape_cmd_append(&cmd, obj_path);

	ape_task_set_cmd(task, cmd);
	ape_da_append(&builder->tasks, task);

	APEBUILD_FREE(obj_path);
	return task;
}

APEBUILD_DEF ApeTask *ape_builder_add_link_task(ApeBuilder *builder)
{
	char *output = ape_builder_output_path(builder);

	ApeStrBuilder name_sb = ape_sb_new();
	ape_sb_append_str(&name_sb, "Link ");
	ape_sb_append_str(&name_sb, builder->name);

	ApeTask *task = ape_task_new(builder, APE_TASK_LINK, ape_sb_to_str(&name_sb));
	ape_sb_free(&name_sb);

	ape_task_set_output(task, output);

	/* Build link command */
	ApeToolchain *tc = builder->toolchain;
	if (!tc)
		tc = builder->ctx->toolchain;

	ApeCmd cmd = ape_cmd_new();
	ape_cmd_append(&cmd, tc->ld);

	/* Default linker flags */
	for (size_t i = 0; i < tc->default_ldflags.count; i++) {
		ape_cmd_append(&cmd, tc->default_ldflags.items[i]);
	}

	/* Builder-specific linker flags */
	for (size_t i = 0; i < builder->ldflags.count; i++) {
		ape_cmd_append(&cmd, builder->ldflags.items[i]);
	}

	/* Shared library flags */
	if (builder->type == APE_TARGET_SHARED_LIB) {
		ape_cmd_append(&cmd, "-shared");
	}

	/* Output */
	ape_cmd_append(&cmd, "-o");
	ape_cmd_append(&cmd, output);

	/* Object files (from compile tasks) */
	for (size_t i = 0; i < builder->tasks.count; i++) {
		ApeTask *compile_task = builder->tasks.items[i];
		if (compile_task->type == APE_TASK_COMPILE) {
			ape_cmd_append(&cmd, compile_task->output);
			ape_task_add_input(task, compile_task->output);
			ape_task_add_dep(task, compile_task);
		}
	}

	/* Library directories */
	for (size_t i = 0; i < builder->lib_dirs.count; i++) {
		ApeStrBuilder lib_dir = ape_sb_new();
		ape_sb_append_str(&lib_dir, "-L");
		ape_sb_append_str(&lib_dir, builder->lib_dirs.items[i]);
		ape_cmd_append(&cmd, ape_sb_to_str(&lib_dir));
		ape_sb_free(&lib_dir);
	}

	/* Libraries */
	for (size_t i = 0; i < builder->libs.count; i++) {
		ApeStrBuilder lib = ape_sb_new();
		ape_sb_append_str(&lib, "-l");
		ape_sb_append_str(&lib, builder->libs.items[i]);
		ape_cmd_append(&cmd, ape_sb_to_str(&lib));
		ape_sb_free(&lib);
	}

	ape_task_set_cmd(task, cmd);
	ape_da_append(&builder->tasks, task);

	APEBUILD_FREE(output);
	return task;
}

APEBUILD_DEF ApeTask *ape_builder_add_archive_task(ApeBuilder *builder)
{
	char *output = ape_builder_output_path(builder);

	ApeStrBuilder name_sb = ape_sb_new();
	ape_sb_append_str(&name_sb, "Archive ");
	ape_sb_append_str(&name_sb, builder->name);

	ApeTask *task = ape_task_new(builder, APE_TASK_ARCHIVE, ape_sb_to_str(&name_sb));
	ape_sb_free(&name_sb);

	ape_task_set_output(task, output);

	/* Build archive command */
	ApeToolchain *tc = builder->toolchain;
	if (!tc)
		tc = builder->ctx->toolchain;

	ApeCmd cmd = ape_cmd_new();
	ape_cmd_append(&cmd, tc->ar);
	ape_cmd_append(&cmd, "rcs");
	ape_cmd_append(&cmd, output);

	/* Object files */
	for (size_t i = 0; i < builder->tasks.count; i++) {
		ApeTask *compile_task = builder->tasks.items[i];
		if (compile_task->type == APE_TASK_COMPILE) {
			ape_cmd_append(&cmd, compile_task->output);
			ape_task_add_input(task, compile_task->output);
			ape_task_add_dep(task, compile_task);
		}
	}

	ape_task_set_cmd(task, cmd);
	ape_da_append(&builder->tasks, task);

	APEBUILD_FREE(output);
	return task;
}

APEBUILD_DEF ApeTask *ape_builder_add_command_task(ApeBuilder *builder, const char *name, ApeCmd cmd)
{
	ApeTask *task = ape_task_new(builder, APE_TASK_COMMAND, name);
	ape_task_set_cmd(task, cmd);
	ape_da_append(&builder->tasks, task);
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
	if (!output_dir && ctx)
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
	ape_scheduler_add_tasks(sched, &builder->tasks);

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
	if (output && ape_fs_exists(output)) {
		ape_fs_remove(output);
	}
	APEBUILD_FREE(output);

	/* Remove object files */
	for (size_t i = 0; i < builder->sources.count; i++) {
		char *obj = ape_build_obj_path(builder->ctx, builder, builder->sources.items[i]);
		if (obj && ape_fs_exists(obj)) {
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
	ape_da_append(&ctx->builders, builder);
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
	ape_da_append(&sched->pending, task);
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
		if (task->status == APE_TASK_PENDING && ape_task_ready(task)) {
			return task;
		}
	}
	return NULL;
}

APEBUILD_PRIVATE void ape_scheduler_remove_pending(ApeScheduler *sched, ApeTask *task)
{
	for (size_t i = 0; i < sched->pending.count; i++) {
		if (sched->pending.items[i] == task) {
			ape_da_remove(&sched->pending, i);
			return;
		}
	}
}

APEBUILD_PRIVATE void ape_scheduler_remove_running(ApeScheduler *sched, ApeTask *task)
{
	for (size_t i = 0; i < sched->running.count; i++) {
		if (sched->running.items[i] == task) {
			ape_da_remove(&sched->running, i);
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
		sched->completed++;
		return APEBUILD_TRUE;
	}

	/* Start process */
	task->proc = ape_cmd_start(&task->cmd);
	if (task->proc == APE_INVALID_HANDLE) {
		task->status = APE_TASK_FAILED;
		task->exit_code = -1;
		sched->failed++;
		return APEBUILD_FALSE;
	}

	task->status = APE_TASK_RUNNING;
	ape_scheduler_remove_pending(sched, task);
	ape_da_append(&sched->running, task);

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

			if (result.status == APE_PROC_COMPLETED && result.exit_code == 0) {
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
	if (!tc && ctx)
		tc = ctx->toolchain;
	if (!tc)
		return NULL;

	const char *output_dir = builder ? builder->output_dir : NULL;
	if (!output_dir && ctx)
		output_dir = ctx->output_dir;
	if (!output_dir)
		output_dir = "build";

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
	ape_cmd_append(&cmd, "cc");
	ape_cmd_append(&cmd, "-o");
	ape_cmd_append(&cmd, binary);
	ape_cmd_append(&cmd, source);

	int result = ape_cmd_run_status(&cmd);
	ape_cmd_free(&cmd);

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
		ape_cmd_append(&run_cmd, argv[i]);
	}

	result = ape_cmd_run_status(&run_cmd);
	ape_cmd_free(&run_cmd);

	return result;
}
