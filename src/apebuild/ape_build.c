/*
 * ape_build.c - Core build module implementation
 *
 * Uses handle-based storage for all build objects (toolchains, builders, tasks)
 * instead of pointers. Objects are stored in fixed-size global arrays.
 */

#include "apebuild_internal.h"

#include <string.h>
#include <stdio.h>
#include <unistd.h>

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

APEBUILD_DEF void ape_build_init(void)
{
	if (ape_build_initialized)
		return;

	memset(ape_toolchain_storage, 0, sizeof(ape_toolchain_storage));
	memset(ape_builder_storage, 0, sizeof(ape_builder_storage));
	memset(ape_task_storage, 0, sizeof(ape_task_storage));

	ape_build_initialized = 1;
}

APEBUILD_PRIVATE void ape_toolchain_clear(ApeToolchain *tc)
{
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

APEBUILD_PRIVATE void ape_task_clear(ApeTask *task)
{
	APEBUILD_FREE(task->name);
	APEBUILD_FREE(task->input);
	APEBUILD_FREE(task->output);
	ape_sl_free(&task->inputs);
	ape_cmd_free(&task->cmd);
	memset(task, 0, sizeof(ApeTask));
}

APEBUILD_PRIVATE void ape_builder_clear(ApeBuilder *builder)
{
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

APEBUILD_DEF void ape_build_shutdown(void)
{
	if (!ape_build_initialized)
		return;

	/* Free all toolchains */
	for (int i = 0; i < APE_MAX_TOOLCHAINS; i++) {
		if (ape_toolchain_storage[i].in_use) {
			ape_toolchain_clear(&ape_toolchain_storage[i]);
		}
	}

	/* Free all tasks */
	for (int i = 0; i < APE_MAX_TASKS; i++) {
		if (ape_task_storage[i].in_use) {
			ape_task_clear(&ape_task_storage[i]);
		}
	}

	/* Free all builders */
	for (int i = 0; i < APE_MAX_BUILDERS; i++) {
		if (ape_builder_storage[i].in_use) {
			ape_builder_clear(&ape_builder_storage[i]);
		}
	}

	ape_build_initialized = 0;
}

APEBUILD_DEF void ape_build_reset(void)
{
	ape_build_shutdown();
	ape_build_init();
}

/* ============================================================================
 * Toolchain Implementation
 * ============================================================================ */

APEBUILD_DEF ApeToolchainHandle ape_toolchain_new(const char *name)
{
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

APEBUILD_DEF void ape_toolchain_free(ApeToolchainHandle handle)
{
	ApeToolchain *tc = ape_toolchain_get(handle);
	if (tc) {
		ape_toolchain_clear(tc);
	}
}

APEBUILD_DEF ApeToolchain *ape_toolchain_get(ApeToolchainHandle handle)
{
	if (handle < 0 || handle >= APE_MAX_TOOLCHAINS)
		return NULL;
	if (!ape_toolchain_storage[handle].in_use)
		return NULL;
	return &ape_toolchain_storage[handle];
}

APEBUILD_DEF int ape_toolchain_valid(ApeToolchainHandle handle)
{
	return ape_toolchain_get(handle) != NULL;
}

APEBUILD_DEF ApeToolchainHandle ape_toolchain_clone(ApeToolchainHandle handle)
{
	ApeToolchain *src = ape_toolchain_get(handle);
	if (!src)
		return APE_INVALID_TOOLCHAIN;

	ApeToolchainHandle new_handle = ape_toolchain_new(src->name);
	ApeToolchain *tc = ape_toolchain_get(new_handle);
	if (!tc)
		return APE_INVALID_TOOLCHAIN;

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

APEBUILD_DEF ApeToolchainHandle ape_toolchain_gcc(void)
{
	ApeToolchainHandle handle = ape_toolchain_new("gcc");
	ApeToolchain *tc = ape_toolchain_get(handle);
	if (!tc)
		return APE_INVALID_TOOLCHAIN;

	APEBUILD_FREE(tc->cc);
	APEBUILD_FREE(tc->cxx);
	APEBUILD_FREE(tc->ld);
	tc->cc = ape_str_dup("gcc");
	tc->cxx = ape_str_dup("g++");
	tc->ld = ape_str_dup("gcc");

	return handle;
}

APEBUILD_DEF ApeToolchainHandle ape_toolchain_clang(void)
{
	ApeToolchainHandle handle = ape_toolchain_new("clang");
	ApeToolchain *tc = ape_toolchain_get(handle);
	if (!tc)
		return APE_INVALID_TOOLCHAIN;

	APEBUILD_FREE(tc->cc);
	APEBUILD_FREE(tc->cxx);
	APEBUILD_FREE(tc->ld);
	tc->cc = ape_str_dup("clang");
	tc->cxx = ape_str_dup("clang++");
	tc->ld = ape_str_dup("clang");

	return handle;
}

APEBUILD_DEF ApeToolchainHandle ape_toolchain_emcc(void)
{
	ApeToolchainHandle handle = ape_toolchain_new("emcc");
	ApeToolchain *tc = ape_toolchain_get(handle);
	if (!tc)
		return APE_INVALID_TOOLCHAIN;

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

APEBUILD_DEF void ape_toolchain_set_cc(ApeToolchainHandle handle, const char *cc)
{
	ApeToolchain *tc = ape_toolchain_get(handle);
	if (!tc)
		return;
	APEBUILD_FREE(tc->cc);
	tc->cc = ape_str_dup(cc);
}

APEBUILD_DEF void ape_toolchain_set_cxx(ApeToolchainHandle handle, const char *cxx)
{
	ApeToolchain *tc = ape_toolchain_get(handle);
	if (!tc)
		return;
	APEBUILD_FREE(tc->cxx);
	tc->cxx = ape_str_dup(cxx);
}

APEBUILD_DEF void ape_toolchain_set_ld(ApeToolchainHandle handle, const char *ld)
{
	ApeToolchain *tc = ape_toolchain_get(handle);
	if (!tc)
		return;
	APEBUILD_FREE(tc->ld);
	tc->ld = ape_str_dup(ld);
}

APEBUILD_DEF void ape_toolchain_set_ar(ApeToolchainHandle handle, const char *ar)
{
	ApeToolchain *tc = ape_toolchain_get(handle);
	if (!tc)
		return;
	APEBUILD_FREE(tc->ar);
	tc->ar = ape_str_dup(ar);
}

APEBUILD_DEF void ape_toolchain_add_cflag(ApeToolchainHandle handle, const char *flag)
{
	ApeToolchain *tc = ape_toolchain_get(handle);
	if (!tc)
		return;
	ape_sl_append_dup(&tc->default_cflags, flag);
}

APEBUILD_DEF void ape_toolchain_add_ldflag(ApeToolchainHandle handle, const char *flag)
{
	ApeToolchain *tc = ape_toolchain_get(handle);
	if (!tc)
		return;
	ape_sl_append_dup(&tc->default_ldflags, flag);
}

/* ============================================================================
 * Task Implementation
 * ============================================================================ */

APEBUILD_DEF ApeTaskHandle ape_task_new(ApeBuilderHandle builder_handle, ApeTaskType type, const char *name)
{
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

APEBUILD_DEF void ape_task_free(ApeTaskHandle handle)
{
	ApeTask *task = ape_task_get(handle);
	if (task) {
		ape_task_clear(task);
	}
}

APEBUILD_DEF ApeTask *ape_task_get(ApeTaskHandle handle)
{
	if (handle < 0 || handle >= APE_MAX_TASKS)
		return NULL;
	if (!ape_task_storage[handle].in_use)
		return NULL;
	return &ape_task_storage[handle];
}

APEBUILD_DEF int ape_task_valid(ApeTaskHandle handle)
{
	return ape_task_get(handle) != NULL;
}

APEBUILD_DEF void ape_task_set_input(ApeTaskHandle handle, const char *input)
{
	ApeTask *task = ape_task_get(handle);
	if (!task)
		return;
	APEBUILD_FREE(task->input);
	task->input = ape_str_dup(input);
}

APEBUILD_DEF void ape_task_set_output(ApeTaskHandle handle, const char *output)
{
	ApeTask *task = ape_task_get(handle);
	if (!task)
		return;
	APEBUILD_FREE(task->output);
	task->output = ape_str_dup(output);
}

APEBUILD_DEF void ape_task_add_input(ApeTaskHandle handle, const char *input)
{
	ApeTask *task = ape_task_get(handle);
	if (!task)
		return;
	ape_sl_append_dup(&task->inputs, input);
}

APEBUILD_DEF void ape_task_add_dep(ApeTaskHandle handle, ApeTaskHandle dep)
{
	ApeTask *task = ape_task_get(handle);
	if (!task)
		return;
	if (task->deps.count >= APE_MAX_TASK_DEPS)
		return;
	task->deps.items[task->deps.count++] = dep;
}

APEBUILD_DEF void ape_task_set_cmd(ApeTaskHandle handle, ApeCmd cmd)
{
	ApeTask *task = ape_task_get(handle);
	if (!task)
		return;
	ape_cmd_free(&task->cmd);
	task->cmd = cmd;
}

APEBUILD_DEF int ape_task_needs_rebuild(ApeTaskHandle handle)
{
	ApeTask *task = ape_task_get(handle);
	if (!task)
		return APEBUILD_FALSE;

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

APEBUILD_DEF int ape_task_ready(ApeTaskHandle handle)
{
	ApeTask *task = ape_task_get(handle);
	if (!task)
		return APEBUILD_FALSE;

	for (size_t i = 0; i < task->deps.count; i++) {
		ApeTask *dep = ape_task_get(task->deps.items[i]);
		if (!dep)
			continue;
		if (dep->status != APE_TASK_COMPLETED && dep->status != APE_TASK_SKIPPED) {
			return APEBUILD_FALSE;
		}
	}
	return APEBUILD_TRUE;
}

/* ============================================================================
 * Builder Implementation
 * ============================================================================ */

APEBUILD_DEF ApeBuilderHandle ape_builder_new(const char *name)
{
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

APEBUILD_DEF void ape_builder_free(ApeBuilderHandle handle)
{
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder)
		return;

	/* Free associated tasks */
	for (size_t i = 0; i < builder->tasks.count; i++) {
		ape_task_free(builder->tasks.items[i]);
	}

	ape_builder_clear(builder);
}

APEBUILD_DEF ApeBuilder *ape_builder_get(ApeBuilderHandle handle)
{
	if (handle < 0 || handle >= APE_MAX_BUILDERS)
		return NULL;
	if (!ape_builder_storage[handle].in_use)
		return NULL;
	return &ape_builder_storage[handle];
}

APEBUILD_DEF int ape_builder_valid(ApeBuilderHandle handle)
{
	return ape_builder_get(handle) != NULL;
}

APEBUILD_DEF ApeBuilderHandle ape_builder_find(const char *name)
{
	for (int i = 0; i < APE_MAX_BUILDERS; i++) {
		if (ape_builder_storage[i].in_use && ape_str_eq(ape_builder_storage[i].name, name)) {
			return (ApeBuilderHandle)i;
		}
	}
	return APE_INVALID_BUILDER;
}

APEBUILD_DEF void ape_builder_set_type(ApeBuilderHandle handle, ApeTargetType type)
{
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder)
		return;
	builder->type = type;
}

APEBUILD_DEF void ape_builder_set_toolchain(ApeBuilderHandle handle, ApeToolchainHandle tc)
{
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder)
		return;
	builder->toolchain = tc;
}

APEBUILD_DEF void ape_builder_set_output_dir(ApeBuilderHandle handle, const char *dir)
{
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder)
		return;
	APEBUILD_FREE(builder->output_dir);
	builder->output_dir = ape_str_dup(dir);
}

APEBUILD_DEF void ape_builder_set_output_name(ApeBuilderHandle handle, const char *name)
{
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder)
		return;
	APEBUILD_FREE(builder->output_name);
	builder->output_name = ape_str_dup(name);
}

/* Source management */

APEBUILD_DEF void ape_builder_add_source(ApeBuilderHandle handle, const char *path)
{
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder)
		return;
	ape_sl_append_dup(&builder->sources, path);
}

APEBUILD_DEF void ape_builder_add_sources(ApeBuilderHandle handle, const char **paths, size_t count)
{
	for (size_t i = 0; i < count; i++) {
		ape_builder_add_source(handle, paths[i]);
	}
}

typedef struct {
	ApeBuilderHandle builder;
} BuilderAddSourceCtx;

APEBUILD_PRIVATE void ape_builder_add_source_callback(const char *path, const ApeDirEntry *entry, void *userdata)
{
	BuilderAddSourceCtx *ctx = (BuilderAddSourceCtx *)userdata;
	ApeBuilder *builder = ape_builder_get(ctx->builder);
	if (!builder)
		return;

	if (!entry->is_file)
		return;

	/* Only add C/C++ source files */
	if (ape_str_ends_with(entry->name, ".c") || ape_str_ends_with(entry->name, ".cpp") || ape_str_ends_with(entry->name, ".cc") ||
	    ape_str_ends_with(entry->name, ".cxx")) {
		char *fullpath = ape_fs_join(path, entry->name);
		ape_sl_append(&builder->sources, fullpath);
	}
}

APEBUILD_DEF void ape_builder_add_source_dir(ApeBuilderHandle handle, const char *dir)
{
	BuilderAddSourceCtx ctx = { .builder = handle };
	ape_fs_iterdir(dir, ape_builder_add_source_callback, &ctx);
}

APEBUILD_DEF void ape_builder_add_source_dir_r(ApeBuilderHandle handle, const char *dir)
{
	BuilderAddSourceCtx ctx = { .builder = handle };
	ape_fs_iterdir_r(dir, ape_builder_add_source_callback, &ctx);
}

APEBUILD_DEF void ape_builder_add_source_glob(ApeBuilderHandle handle, const char *pattern)
{
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder)
		return;

	ApeStrList files = ape_fs_glob(pattern);
	for (size_t i = 0; i < files.count; i++) {
		/* Transfer ownership */
		ape_sl_append(&builder->sources, files.items[i]);
	}
	/* Free list but not strings (ownership transferred) */
	ape_sl_free_shallow(&files);
}

/* Compiler flags */

APEBUILD_DEF void ape_builder_add_cflag(ApeBuilderHandle handle, const char *flag)
{
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder)
		return;
	ape_sl_append_dup(&builder->cflags, flag);
}

APEBUILD_DEF void ape_builder_add_include(ApeBuilderHandle handle, const char *dir)
{
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder)
		return;
	ape_sl_append_dup(&builder->include_dirs, dir);
}

APEBUILD_DEF void ape_builder_add_define(ApeBuilderHandle handle, const char *define)
{
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder)
		return;
	ape_sl_append_dup(&builder->defines, define);
}

APEBUILD_DEF void ape_builder_add_define_value(ApeBuilderHandle handle, const char *name, const char *value)
{
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder)
		return;

	ApeStrBuilder sb = ape_sb_new();
	ape_sb_append_str(&sb, name);
	ape_sb_append_char(&sb, '=');
	ape_sb_append_str(&sb, value);
	ape_sl_append(&builder->defines, ape_sb_to_str_dup(&sb));
	ape_sb_free(&sb);
}

/* Linker flags */

APEBUILD_DEF void ape_builder_add_ldflag(ApeBuilderHandle handle, const char *flag)
{
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder)
		return;
	ape_sl_append_dup(&builder->ldflags, flag);
}

APEBUILD_DEF void ape_builder_add_lib_dir(ApeBuilderHandle handle, const char *dir)
{
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder)
		return;
	ape_sl_append_dup(&builder->lib_dirs, dir);
}

APEBUILD_DEF void ape_builder_add_lib(ApeBuilderHandle handle, const char *lib)
{
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder)
		return;
	ape_sl_append_dup(&builder->libs, lib);
}

/* Emscripten/WASM configuration */

APEBUILD_DEF void ape_builder_set_shell_file(ApeBuilderHandle handle, const char *path)
{
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder)
		return;
	APEBUILD_FREE(builder->shell_file);
	builder->shell_file = ape_str_dup(path);
}

APEBUILD_DEF void ape_builder_set_wasm_output_mode(ApeBuilderHandle handle, int html)
{
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder)
		return;
	builder->wasm_html_output = html;
}

APEBUILD_DEF void ape_builder_add_exported_function(ApeBuilderHandle handle, const char *func)
{
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder)
		return;
	ape_sl_append_dup(&builder->exported_functions, func);
}

APEBUILD_DEF void ape_builder_add_preload_file(ApeBuilderHandle handle, const char *path)
{
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder)
		return;
	ape_sl_append_dup(&builder->preload_files, path);
}

APEBUILD_DEF void ape_builder_add_embed_file(ApeBuilderHandle handle, const char *path)
{
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder)
		return;
	ape_sl_append_dup(&builder->embed_files, path);
}

APEBUILD_DEF void ape_builder_set_wasm_memory(ApeBuilderHandle handle, int initial_mb, int max_mb)
{
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder)
		return;
	builder->wasm_initial_memory = initial_mb;
	builder->wasm_max_memory = max_mb;
}

APEBUILD_DEF const char *ape_emcc_default_shell(void)
{
	return ape_emcc_default_shell_html;
}

/* Dependencies */

APEBUILD_DEF void ape_builder_depends_on(ApeBuilderHandle handle, ApeBuilderHandle dep)
{
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder)
		return;
	if (builder->deps.count >= APE_MAX_BUILDER_DEPS)
		return;
	builder->deps.items[builder->deps.count++] = dep;
}

APEBUILD_DEF void ape_builder_link_with(ApeBuilderHandle handle, ApeBuilderHandle lib_builder_handle)
{
	ApeBuilder *builder = ape_builder_get(handle);
	ApeBuilder *lib_builder = ape_builder_get(lib_builder_handle);
	if (!builder || !lib_builder)
		return;

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

APEBUILD_DEF char *ape_builder_output_path(ApeBuilderHandle handle)
{
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder)
		return NULL;

	ApeToolchain *tc = ape_toolchain_get(builder->toolchain);
	if (!tc)
		return NULL;

	const char *output_dir = builder->output_dir;
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

APEBUILD_DEF ApeTaskHandle ape_builder_add_compile_task(ApeBuilderHandle handle, const char *source)
{
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder)
		return APE_INVALID_TASK;

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
	if (builder->type == APE_TARGET_SHARED_LIB) {
		ape_cmd_append(&cmd, "-fPIC");
	}

	/* Compile only */
	ape_cmd_append(&cmd, "-c");
	ape_cmd_append(&cmd, source);
	ape_cmd_append(&cmd, "-o");
	ape_cmd_append(&cmd, obj_path);

	ape_task_set_cmd(task_handle, cmd);

	/* Add to builder's task list */
	if (builder->tasks.count < APE_MAX_TASKS) {
		builder->tasks.items[builder->tasks.count++] = task_handle;
	}

	return task_handle;
}

APEBUILD_DEF ApeTaskHandle ape_builder_add_link_task(ApeBuilderHandle handle)
{
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder)
		return APE_INVALID_TASK;

	char *output = ape_builder_output_path(handle);
	if (!output)
		return APE_INVALID_TASK;

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
				if (i > 0)
					ape_sb_append_char(&exports_sb, ',');
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
	if (builder->tasks.count < APE_MAX_TASKS) {
		builder->tasks.items[builder->tasks.count++] = task_handle;
	}

	return task_handle;
}

APEBUILD_DEF ApeTaskHandle ape_builder_add_archive_task(ApeBuilderHandle handle)
{
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder)
		return APE_INVALID_TASK;

	char *output = ape_builder_output_path(handle);
	if (!output)
		return APE_INVALID_TASK;

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
	if (builder->tasks.count < APE_MAX_TASKS) {
		builder->tasks.items[builder->tasks.count++] = task_handle;
	}

	return task_handle;
}

APEBUILD_DEF ApeTaskHandle ape_builder_add_command_task(ApeBuilderHandle handle, const char *name, ApeCmd cmd)
{
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder)
		return APE_INVALID_TASK;

	ApeTaskHandle task_handle = ape_task_new(handle, APE_TASK_TYPE_COMMAND, name);
	ape_task_set_cmd(task_handle, cmd);

	/* Add to builder's task list */
	if (builder->tasks.count < APE_MAX_TASKS) {
		builder->tasks.items[builder->tasks.count++] = task_handle;
	}

	return task_handle;
}

APEBUILD_DEF void ape_builder_generate_tasks(ApeBuilderHandle handle)
{
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder)
		return;

	/* Clear existing tasks */
	for (size_t i = 0; i < builder->tasks.count; i++) {
		ape_task_free(builder->tasks.items[i]);
	}
	builder->tasks.count = 0;

	/* Create compile tasks for each source */
	for (size_t i = 0; i < builder->sources.count; i++) {
		ape_builder_add_compile_task(handle, builder->sources.items[i]);
	}

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

APEBUILD_DEF void ape_ctx_init(ApeBuildCtx *ctx)
{
	ape_build_init();
	memset(ctx, 0, sizeof(ApeBuildCtx));

	ctx->toolchain = ape_toolchain_gcc();
	ctx->output_dir = ape_str_dup("build");
	ctx->parallel_jobs = 0; /* Auto-detect */
	ctx->verbosity = APE_VERBOSE_NORMAL;
}

APEBUILD_DEF void ape_ctx_cleanup(ApeBuildCtx *ctx)
{
	if (!ctx)
		return;

	/* Note: We don't free the toolchain here since it's in global storage */
	APEBUILD_FREE(ctx->output_dir);
	memset(ctx, 0, sizeof(ApeBuildCtx));
}

APEBUILD_DEF void ape_ctx_set_toolchain(ApeBuildCtx *ctx, ApeToolchainHandle tc)
{
	ctx->toolchain = tc;
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

APEBUILD_DEF ApeToolchainHandle ape_ctx_get_toolchain(ApeBuildCtx *ctx)
{
	return ctx->toolchain;
}

/* ============================================================================
 * Build Operations
 * ============================================================================ */

APEBUILD_PRIVATE int ape_builder_run_tasks(ApeBuilderHandle handle, ApeBuildCtx *ctx)
{
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder)
		return APEBUILD_FALSE;

	ApeVerbosity verbosity = ctx ? ctx->verbosity : APE_VERBOSE_NORMAL;
	int max_parallel = ctx ? ctx->parallel_jobs : 0;
	if (max_parallel <= 0)
		max_parallel = ape_build_get_cpu_count();

	/* Simple task scheduler using handle lists */
	ApeTaskHandleList pending = { 0 };
	ApeTaskHandleList running = { 0 };
	int completed = 0, failed = 0, skipped = 0;

	/* Add all tasks to pending */
	for (size_t i = 0; i < builder->tasks.count; i++) {
		pending.items[pending.count++] = builder->tasks.items[i];
	}

	while (pending.count > 0 || running.count > 0) {
		/* Check for completed tasks */
		for (size_t i = 0; i < running.count;) {
			ApeTask *task = ape_task_get(running.items[i]);
			if (!task) {
				/* Remove invalid task */
				for (size_t j = i; j < running.count - 1; j++)
					running.items[j] = running.items[j + 1];
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
				for (size_t j = i; j < running.count - 1; j++)
					running.items[j] = running.items[j + 1];
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
						if (pt && pt->status == APE_TASK_PENDING) {
							pt->status = APE_TASK_FAILED;
						}
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

			if (ready_handle == APE_INVALID_TASK)
				break;

			ApeTask *task = ape_task_get(ready_handle);
			if (!task)
				break;

			/* Check if rebuild needed */
			if (!ctx->force_rebuild && !ape_task_needs_rebuild(ready_handle)) {
				task->status = APE_TASK_SKIPPED;
				skipped++;
				if (verbosity >= APE_VERBOSE_VERBOSE) {
					ape_log_debug("Skipping %s (up to date)", task->name);
				}
				/* Remove from pending */
				for (size_t j = ready_idx; j < pending.count - 1; j++)
					pending.items[j] = pending.items[j + 1];
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
				for (size_t j = ready_idx; j < pending.count - 1; j++)
					pending.items[j] = pending.items[j + 1];
				pending.count--;
				continue;
			}

			/* Start process */
			task->proc = ape_cmd_start(&task->cmd);
			if (task->proc == APE_INVALID_HANDLE) {
				task->status = APE_TASK_FAILED;
				task->exit_code = -1;
				failed++;
				if (verbosity >= APE_VERBOSE_NORMAL) {
					ape_log_failure("%s failed to start", task->name);
				}
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
			for (size_t j = ready_idx; j < pending.count - 1; j++)
				pending.items[j] = pending.items[j + 1];
			pending.count--;
		}

		/* Short sleep if tasks are running */
		if (running.count > 0) {
			usleep(10000);
		}
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

APEBUILD_DEF int ape_builder_build(ApeBuilderHandle handle)
{
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder)
		return APEBUILD_FALSE;

	if (builder->built)
		return builder->build_failed ? APEBUILD_FALSE : APEBUILD_TRUE;

	/* We need a context for output_dir - create a temporary one if builder has no toolchain set */
	ApeBuildCtx temp_ctx;
	ApeBuildCtx *ctx = &temp_ctx;
	ape_ctx_init(ctx);

	/* Use builder's toolchain if set, otherwise use context's default */
	if (ape_toolchain_valid(builder->toolchain)) {
		ctx->toolchain = builder->toolchain;
	}

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
	if (!output_dir)
		output_dir = "build";
	ape_fs_mkdir_p(output_dir);

	/* Generate tasks */
	ape_builder_generate_tasks(handle);

	if (builder->tasks.count == 0) {
		if (verbosity >= APE_VERBOSE_NORMAL) {
			ape_log_info("Nothing to build for %s", builder->name);
		}
		builder->built = 1;
		ape_ctx_cleanup(ctx);
		return APEBUILD_TRUE;
	}

	if (verbosity >= APE_VERBOSE_NORMAL) {
		ape_log_build("Building %s...", builder->name);
	}

	int result = ape_builder_run_tasks(handle, ctx);

	builder->built = 1;
	builder->build_failed = !result;

	ape_ctx_cleanup(ctx);
	return result;
}

APEBUILD_DEF int ape_ctx_build(ApeBuildCtx *ctx, ApeBuilderHandle handle)
{
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder)
		return APEBUILD_FALSE;

	/* Set builder's toolchain from context if not already set */
	if (!ape_toolchain_valid(builder->toolchain)) {
		builder->toolchain = ctx->toolchain;
	}

	/* Set builder's output_dir from context if not already set */
	if (!builder->output_dir && ctx->output_dir) {
		builder->output_dir = ape_str_dup(ctx->output_dir);
	}

	return ape_builder_build(handle);
}

APEBUILD_DEF int ape_builder_clean(ApeBuilderHandle handle)
{
	ApeBuilder *builder = ape_builder_get(handle);
	if (!builder)
		return APEBUILD_FALSE;

	/* Remove output */
	char *output = ape_builder_output_path(handle);
	if (output && ape_fs_exists(output)) {
		ape_fs_remove(output);
	}
	APEBUILD_FREE(output);

	/* Remove object files */
	for (size_t i = 0; i < builder->sources.count; i++) {
		char *obj = ape_build_obj_path(NULL, handle, builder->sources.items[i]);
		if (obj && ape_fs_exists(obj)) {
			ape_fs_remove(obj);
		}
		APEBUILD_FREE(obj);
	}

	builder->built = 0;
	builder->build_failed = 0;

	return APEBUILD_TRUE;
}

APEBUILD_DEF int ape_ctx_clean(ApeBuildCtx *ctx, ApeBuilderHandle handle)
{
	(void)ctx;
	return ape_builder_clean(handle);
}

APEBUILD_DEF int ape_builder_rebuild(ApeBuilderHandle handle)
{
	ape_builder_clean(handle);
	ApeBuilder *builder = ape_builder_get(handle);
	if (builder)
		builder->built = 0;
	return ape_builder_build(handle);
}

APEBUILD_DEF int ape_ctx_rebuild(ApeBuildCtx *ctx, ApeBuilderHandle handle)
{
	ape_ctx_clean(ctx, handle);
	return ape_ctx_build(ctx, handle);
}

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

APEBUILD_DEF char *ape_build_obj_path(ApeBuildCtx *ctx, ApeBuilderHandle handle, const char *source)
{
	ApeBuilder *builder = ape_builder_get(handle);

	ApeToolchain *tc = NULL;
	if (builder)
		tc = ape_toolchain_get(builder->toolchain);
	if (!tc && ctx)
		tc = ape_toolchain_get(ctx->toolchain);
	if (!tc)
		return NULL;

	const char *output_dir = NULL;
	if (builder)
		output_dir = builder->output_dir;
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

APEBUILD_DEF char *ape_build_output_path(ApeBuildCtx *ctx, ApeBuilderHandle handle)
{
	(void)ctx;
	return ape_builder_output_path(handle);
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
	for (int i = 0; i < argc; i++) {
		ape_cmd_append(&run_cmd, argv[i]);
	}

	result = ape_cmd_run_status(&run_cmd);
	ape_cmd_free(&run_cmd);

	return result;
}
