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
	char *content = ape_fs_read_file(src, &size);
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
	if (len > 0 && tmp[len - 1] == '/')
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
	return stat(path, &st) == 0 ? APEBUILD_TRUE : APEBUILD_FALSE;
}

APEBUILD_DEF int ape_fs_is_file(const char *path)
{
	struct stat st;
	if (stat(path, &st) != 0)
		return APEBUILD_FALSE;
	return S_ISREG(st.st_mode) ? APEBUILD_TRUE : APEBUILD_FALSE;
}

APEBUILD_DEF int ape_fs_is_dir(const char *path)
{
	struct stat st;
	if (stat(path, &st) != 0)
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
		while (last_slash > base_path && *last_slash != '/')
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

	GlobContext ctx = { .pattern = pattern, .base_path = base_path, .results = &results };

	ape_fs_iterdir_r(base_path, ape_fs_glob_callback, &ctx);

	APEBUILD_FREE(base_path);
	return results;
}

/* ============================================================================
 * File Metadata
 * ============================================================================ */

APEBUILD_DEF int ape_fs_stat(const char *path, ApeFileStat *out)
{
	struct stat st;
	if (stat(path, &st) != 0)
		return APEBUILD_FALSE;

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

APEBUILD_DEF time_t ape_fs_mtime(const char *path)
{
	struct stat st;
	if (stat(path, &st) != 0)
		return 0;
	return st.st_mtime;
}

APEBUILD_DEF size_t ape_fs_size(const char *path)
{
	struct stat st;
	if (stat(path, &st) != 0)
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
	return ape_fs_needs_rebuild(output, &input, 1);
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
		ape_sb_append_str(&sb, dir);
		ape_sb_append_char(&sb, '/');
	}
	ape_sb_append_str(&sb, stem);
	if (new_ext && *new_ext) {
		if (*new_ext != '.')
			ape_sb_append_char(&sb, '.');
		ape_sb_append_str(&sb, new_ext);
	}

	APEBUILD_FREE(dir);
	APEBUILD_FREE(stem);

	char *result = ape_sb_to_str_dup(&sb);
	ape_sb_free(&sb);
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
	if (is_absolute)
		ape_sb_append_char(&sb, '/');
	ape_sb_append_str(&sb, joined);

	if (sb.count == 0)
		ape_sb_append_str(&sb, ".");

	APEBUILD_FREE(joined);
	ape_sl_free(&parts);
	ape_sl_free(&result_parts);

	char *result = ape_sb_to_str_dup(&sb);
	ape_sb_free(&sb);
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
	while (common < from_parts.count && common < to_parts.count && ape_str_eq(from_parts.items[common], to_parts.items[common])) {
		common++;
	}

	ApeStrBuilder sb = ape_sb_new();

	/* Add ".." for each remaining from part */
	for (size_t i = common; i < from_parts.count; i++) {
		if (sb.count > 0)
			ape_sb_append_char(&sb, '/');
		ape_sb_append_str(&sb, "..");
	}

	/* Add remaining to parts */
	for (size_t i = common; i < to_parts.count; i++) {
		if (sb.count > 0)
			ape_sb_append_char(&sb, '/');
		ape_sb_append_str(&sb, to_parts.items[i]);
	}

	if (sb.count == 0)
		ape_sb_append_str(&sb, ".");

	APEBUILD_FREE(abs_from);
	APEBUILD_FREE(abs_to);
	ape_sl_free(&from_parts);
	ape_sl_free(&to_parts);

	char *result = ape_sb_to_str_dup(&sb);
	ape_sb_free(&sb);
	return result;
}

APEBUILD_DEF int ape_fs_is_absolute(const char *path)
{
	return path && path[0] == '/' ? APEBUILD_TRUE : APEBUILD_FALSE;
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

APEBUILD_DEF char *ape_fs_temp_mkdir(const char *prefix)
{
	const char *tmpdir = ape_fs_temp_dir();
	if (!prefix)
		prefix = "ape";

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
