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
		struct tm *tm_info = localtime(&now);
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

	if (ape_log_config.use_colors && color) {
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
