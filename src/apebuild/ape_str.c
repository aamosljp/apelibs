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
	ape_sb_init(&sb);
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
	ape_sl_init(&sl);
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
		if (i > 0 && sep)
			ape_sb_append_str(&sb, sep);
		ape_sb_append_str(&sb, sl->items[i]);
	}
	char *result = ape_sb_to_str_dup(&sb);
	ape_sb_free(&sb);
	return result;
}

APEBUILD_DEF ApeStrList ape_sl_clone(const ApeStrList *sl)
{
	ApeStrList result = ape_sl_new();
	for (size_t i = 0; i < sl->count; i++) {
		ape_sl_append_dup(&result, sl->items[i]);
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
		if (i > 0 && sep)
			ape_sb_append_str(&sb, sep);
		ape_sb_append_str(&sb, strs[i]);
	}
	char *result = ape_sb_to_str_dup(&sb);
	ape_sb_free(&sb);
	return result;
}

APEBUILD_DEF ApeStrList ape_str_split(const char *str, const char *delim)
{
	ApeStrList result = ape_sl_new();
	if (!str || !delim)
		return result;

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
			if (len > 0 && start[len - 1] == '\r')
				len--;
			ape_sl_append(&result, ape_str_ndup(start, len));
			start = p + 1;
		}
		p++;
	}
	if (start < p) {
		ape_sl_append_dup(&result, start);
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
	while (*a && *b) {
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
	while (*str && isspace((unsigned char)*str))
		str++;
	if (*str == '\0')
		return ape_str_dup("");
	const char *end = str + strlen(str) - 1;
	while (end > str && isspace((unsigned char)*end))
		end--;
	return ape_str_ndup(str, end - str + 1);
}

APEBUILD_DEF char *ape_str_trim_left(const char *str)
{
	if (!str)
		return NULL;
	while (*str && isspace((unsigned char)*str))
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
	while (end >= str && isspace((unsigned char)*end))
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
	ape_sb_append_strn(&sb, str, found - str);
	ape_sb_append_str(&sb, new_str);
	ape_sb_append_str(&sb, found + old_len);
	char *result = ape_sb_to_str_dup(&sb);
	ape_sb_free(&sb);
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
		ape_sb_append_strn(&sb, start, found - start);
		ape_sb_append_str(&sb, new_str);
		start = found + old_len;
	}
	ape_sb_append_str(&sb, start);

	char *result = ape_sb_to_str_dup(&sb);
	ape_sb_free(&sb);
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
	long val = strtol(str, &end, 10);
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
	long val = strtol(str, &end, 10);
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
	float val = strtof(str, &end);
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
	double val = strtod(str, &end);
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
