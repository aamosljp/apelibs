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

#ifndef APE_CONFIG_INCLUDED
#define APE_CONFIG_INCLUDED

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define APE_CONFIG_WINDOWS
#if defined(_WIN64)
#define APE_CONFIG_WINDOWS_X64
#endif
#elif defined(__linux__) || defined(__unix__)
#define APE_CONFIG_LINUX
#elif defined(__APPLE__)
#define APE_CONFIG_APPLE
#endif

#ifndef APE_CONFIG_MALLOC
#if defined(APE_CONFIG_REALLOC) || defined(APE_CONFIG_FREE)
#pragma GCC error "Need to define MALLOC, REALLOC and FREE or none of them"
#else
#include <stdlib.h>
#define APE_CONFIG_MALLOC malloc
#define APE_CONFIG_REALLOC realloc
#define APE_CONFIG_FREE free
#endif
#else
#if !defined(APE_CONFIG_REALLOC) || !defined(APE_CONFIG_FREE)
#pragma GCC error "Need to define MALLOC, REALLOC and FREE or none of them"
#endif
#endif

#ifndef APE_CONFIG_ASSERT
#ifdef APE_CONFIG_USE_STDLIB_ASSERT
#include <assert.h>
#define APE_CONFIG_ASSERT(c) assert(c)
#else
#include <stdio.h>
#include <stdlib.h>
#define APE_CONFIG_ASSERT(c)                                                               \
	if (!(c)) {                                                                        \
		fprintf(stderr, "%s:%d Assertion '%s' failed\n", __FILE__, __LINE__, ##c); \
		exit(1);                                                                   \
	}
#endif
#endif

#include <stddef.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

/* ============================================================================
 * Config Types
 * ============================================================================ */

typedef enum {
	APE_CONFIG_TYPE_STRING,
	APE_CONFIG_TYPE_INT,
	APE_CONFIG_TYPE_FLOAT,
	APE_CONFIG_TYPE_BOOL,
	APE_CONFIG_TYPE_ARRAY,
	APE_CONFIG_TYPE_OBJECT
} ApeConfigType;

typedef struct ApeConfigValue ApeConfigValue;

typedef struct ApeConfigObjectItem {
	const char *key;
	ApeConfigValue *value;
} ApeConfigObjectItem;

typedef struct ApeConfigValue {
	ApeConfigType type;
	union {
		const char *string_value;
		int64_t int_value;
		double float_value;
		int bool_value;
		struct {
			size_t count;
			ApeConfigValue *items;
		} array;
		struct {
			size_t count;
			ApeConfigObjectItem *items;
		} object;
	} data;
} ApeConfigValue;

typedef struct {
	size_t capacity;
	size_t count;
	ApeConfigValue *items;
} ApeConfigArray;

typedef struct {
	size_t capacity;
	size_t count;
	struct {
		const char *key;
		ApeConfigValue value;
	} *items;
} ApeConfigObject;

/* Config document */
typedef struct {
	ApeConfigValue root;
} ApeConfig;

/* ============================================================================
 * Config API
 * ============================================================================ */

/* Initialize and cleanup */
void ape_config_init(ApeConfig *config);
void ape_config_cleanup(ApeConfig *config);

/* Parse from string */
int ape_config_parse(ApeConfig *config, const char *str);
int ape_config_parse_file(ApeConfig *config, const char *path);

/* Write to string */
char *ape_config_write(ApeConfig *config);
int ape_config_write_file(ApeConfig *config, const char *path);

/* Get values */
ApeConfigValue *ape_config_get(ApeConfig *config, const char *path);
const char *ape_config_get_string(ApeConfig *config, const char *path, const char *default_value);
int64_t ape_config_get_int(ApeConfig *config, const char *path, int64_t default_value);
double ape_config_get_float(ApeConfig *config, const char *path, double default_value);
int ape_config_get_bool(ApeConfig *config, const char *path, int default_value);

/* Set values */
void ape_config_set_string(ApeConfig *config, const char *path, const char *value);
void ape_config_set_int(ApeConfig *config, const char *path, int64_t value);
void ape_config_set_float(ApeConfig *config, const char *path, double value);
void ape_config_set_bool(ApeConfig *config, const char *path, int value);

/* Array operations */
ApeConfigValue *ape_config_array_get(ApeConfig *config, const char *path, size_t index);
void ape_config_array_set(ApeConfig *config, const char *path, size_t index, ApeConfigValue value);
void ape_config_array_append(ApeConfig *config, const char *path, ApeConfigValue value);
size_t ape_config_array_size(ApeConfig *config, const char *path);

/* Object operations */
ApeConfigValue *ape_config_object_get(ApeConfig *config, const char *path, const char *key);
void ape_config_object_set(ApeConfig *config, const char *path, const char *key, ApeConfigValue value);
int ape_config_object_has(ApeConfig *config, const char *path, const char *key);

/* Utility */
int ape_config_has(ApeConfig *config, const char *path);
ApeConfigType ape_config_type(ApeConfig *config, const char *path);

#if defined(__cplusplus)
}
#endif

#if defined(APE_CONFIG_STRIP_PREFIX)

#endif

#endif

#ifdef APE_CONFIG_IMPLEMENTATION

#ifndef APE_CONFIG_IMPLEMENTATION_INCLUDED
#define APE_CONFIG_IMPLEMENTATION_INCLUDED

/* User can define a custom function prefix (eg. static) */
#ifndef APE_CONFIG_DEF
#define APE_CONFIG_DEF
#endif

/* Can also define custom private function prefix */
#ifndef APE_CONFIG_PRIVATE
#define APE_CONFIG_PRIVATE static
#endif

#ifndef APE_CONFIG_TRUE
#define APE_CONFIG_TRUE (1)
#define APE_CONFIG_FALSE (0)
#else
#if !defined(APE_CONFIG_FALSE)
#pragma GCC error "Need to define both APE_CONFIG_TRUE and APE_CONFIG_FALSE or neither"
#endif
#endif

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

/* BEGIN ape_config.c */

/* Simple string builder for JSON writing */
typedef struct {
	size_t capacity;
	size_t count;
	char *items;
} ApeStrBuilder;

static void ape_sb_init(ApeStrBuilder *sb) {
	sb->capacity = 0;
	sb->count = 0;
	sb->items = NULL;
}

static ApeStrBuilder ape_sb_new(void) {
	ApeStrBuilder sb;
	ape_sb_init(&sb);
	return sb;
}

static void ape_sb_free(ApeStrBuilder *sb) {
	APE_CONFIG_FREE(sb->items);
	ape_sb_init(sb);
}

static void ape_sb_append_char(ApeStrBuilder *sb, char c) {
	if (sb->count >= sb->capacity) {
		sb->capacity = sb->capacity == 0 ? 32 : sb->capacity * 2;
		sb->items = APE_CONFIG_REALLOC(sb->items, sb->capacity);
	}
	sb->items[sb->count++] = c;
}

static void ape_sb_append_str(ApeStrBuilder *sb, const char *str) {
	for (const char *p = str; *p; p++) { ape_sb_append_char(sb, *p); }
}

static const char *ape_sb_to_str(ApeStrBuilder *sb) {
	if (sb->count >= sb->capacity) {
		sb->capacity = sb->count + 1;
		sb->items = APE_CONFIG_REALLOC(sb->items, sb->capacity);
	}
	sb->items[sb->count] = '\0';
	return sb->items;
}

static char *ape_sb_to_str_dup(const ApeStrBuilder *sb) {
	char *result = APE_CONFIG_MALLOC(sb->count + 1);
	memcpy(result, sb->items, sb->count);
	result[sb->count] = '\0';
	return result;
}

/* ============================================================================
 * Private Functions
 * ============================================================================ */

APE_CONFIG_PRIVATE void ape_config_free_value(ApeConfigValue *value) {
	if (!value) return;

	switch (value->type) {
	case APE_CONFIG_TYPE_STRING: APE_CONFIG_FREE((void *)value->data.string_value); break;
	case APE_CONFIG_TYPE_ARRAY:
		for (size_t i = 0; i < value->data.array.count; i++) { ape_config_free_value(&value->data.array.items[i]); }
		APE_CONFIG_FREE(value->data.array.items);
		break;
	case APE_CONFIG_TYPE_OBJECT:
		for (size_t i = 0; i < value->data.object.count; i++) {
			APE_CONFIG_FREE((void *)value->data.object.items[i].key);
			ape_config_free_value(value->data.object.items[i].value);
		}
		APE_CONFIG_FREE(value->data.object.items);
		break;
	default: break;
	}
}

APE_CONFIG_PRIVATE ApeConfigValue *ape_config_get_or_create_path(ApeConfig *config, const char *path) {
	if (!path || strlen(path) == 0) { return &config->root; }

	// Split path by dots and navigate through objects
	ApeConfigValue *current = &config->root;
	const char *ptr = path;
	const char *start = path;

	while (*ptr) {
		if (*ptr == '.') {
			if (current->type != APE_CONFIG_TYPE_OBJECT) {
				// Convert to object if needed
				if (current->type != APE_CONFIG_TYPE_OBJECT) {
					ape_config_free_value(current);
					current->type = APE_CONFIG_TYPE_OBJECT;
					current->data.object.count = 0;
					current->data.object.items = NULL;
				}
			}

			// Extract key
			size_t key_len = ptr - start;
			char *key = APE_CONFIG_MALLOC(key_len + 1);
			strncpy(key, start, key_len);
			key[key_len] = '\0';

			// Find or create object item
			ApeConfigObjectItem *item = ape_config_find_object_item(current, key);
			if (!item) { item = ape_config_add_object_item(current, key); }
			APE_CONFIG_FREE(key);

			if (!item) { return NULL; }

			current = item->value;
			start = ptr + 1;
		}
		ptr++;
	}

	// Handle last key
	if (start < ptr) {
		if (current->type != APE_CONFIG_TYPE_OBJECT) {
			// Convert to object if needed
			if (current->type != APE_CONFIG_TYPE_OBJECT) {
				ape_config_free_value(current);
				current->type = APE_CONFIG_TYPE_OBJECT;
				current->data.object.count = 0;
				current->data.object.items = NULL;
			}
		}

		// Extract key
		size_t key_len = ptr - start;
		char *key = APE_CONFIG_MALLOC(key_len + 1);
		strncpy(key, start, key_len);
		key[key_len] = '\0';

		// Find or create object item
		ApeConfigObjectItem *item = ape_config_find_object_item(current, key);
		if (!item) { item = ape_config_add_object_item(current, key); }
		APE_CONFIG_FREE(key);

		if (!item) { return NULL; }

		current = item->value;
	}

	return current;
}

APE_CONFIG_PRIVATE ApeConfigObjectItem *ape_config_find_object_item(ApeConfigValue *object, const char *key) {
	if (object->type != APE_CONFIG_TYPE_OBJECT) { return NULL; }

	for (size_t i = 0; i < object->data.object.count; i++) {
		if (strcmp(object->data.object.items[i].key, key) == 0) { return &object->data.object.items[i]; }
	}

	return NULL;
}

APE_CONFIG_PRIVATE ApeConfigObjectItem *ape_config_add_object_item(ApeConfigValue *object, const char *key) {
	if (object->type != APE_CONFIG_TYPE_OBJECT) { return NULL; }

	// Check if key already exists
	ApeConfigObjectItem *existing = ape_config_find_object_item(object, key);
	if (existing) { return existing; }

	// Add new item
	object->data.object.count++;
	if (object->data.object.items == NULL) {
		object->data.object.items = APE_CONFIG_MALLOC(sizeof(ApeConfigObjectItem));
	} else {
		object->data.object.items =
			APE_CONFIG_REALLOC(object->data.object.items, object->data.object.count * sizeof(ApeConfigObjectItem));
	}

	ApeConfigObjectItem *new_item = &object->data.object.items[object->data.object.count - 1];
	new_item->key = APE_CONFIG_MALLOC(strlen(key) + 1);
	strcpy((char *)new_item->key, key);

	// Initialize with null value
	new_item->value = APE_CONFIG_MALLOC(sizeof(ApeConfigValue));
	new_item->value->type = APE_CONFIG_TYPE_STRING;
	new_item->value->data.string_value = NULL;

	return new_item;
}

APE_CONFIG_PRIVATE void ape_config_set_value(ApeConfigValue *value, ApeConfigValue new_value) {
	// Free existing value
	ape_config_free_value(value);

	// Copy new value
	*value = new_value;
}

/* ============================================================================
 * Public API Implementation
 * ============================================================================ */

APE_CONFIG_DEF void ape_config_init(ApeConfig *config) {
	config->root.type = APE_CONFIG_TYPE_OBJECT;
	config->root.data.object.count = 0;
	config->root.data.object.items = NULL;
}

APE_CONFIG_DEF void ape_config_cleanup(ApeConfig *config) {
	if (config->root.type == APE_CONFIG_TYPE_OBJECT) {
		for (size_t i = 0; i < config->root.data.object.count; i++) {
			APE_CONFIG_FREE((void *)config->root.data.object.items[i].key);
			ape_config_free_value(config->root.data.object.items[i].value);
		}
		APE_CONFIG_FREE(config->root.data.object.items);
	}
	config->root.data.object.count = 0;
	config->root.data.object.items = NULL;
}

APE_CONFIG_PRIVATE void ape_config_skip_whitespace(const char **str) {
	while (**str && isspace(**str)) { (*str)++; }
}

APE_CONFIG_PRIVATE int ape_config_parse_string(const char **str, char **out) {
	ape_config_skip_whitespace(str);

	if (**str != '"') { return APE_CONFIG_FALSE; }

	(*str)++; // Skip opening quote

	const char *start = *str;
	while (**str && **str != '"') {
		if (**str == '\\') {
			// Skip escaped character
			(*str)++;
			if (**str) (*str)++;
		} else {
			(*str)++;
		}
	}

	if (**str != '"') {
		return APE_CONFIG_FALSE; // Unterminated string
	}

	size_t length = *str - start;
	*out = APE_CONFIG_MALLOC(length + 1);
	strncpy(*out, start, length);
	(*out)[length] = '\0';

	(*str)++; // Skip closing quote

	return APE_CONFIG_TRUE;
}

APE_CONFIG_PRIVATE int ape_config_parse_number(const char **str, double *out) {
	ape_config_skip_whitespace(str);

	char *endptr;
	double value = strtod(*str, &endptr);

	if (endptr == *str) {
		return APE_CONFIG_FALSE; // No number found
	}

	*out = value;
	*str = endptr;

	return APE_CONFIG_TRUE;
}

APE_CONFIG_PRIVATE int ape_config_parse_value(const char **str, ApeConfigValue *out);

APE_CONFIG_PRIVATE int ape_config_parse_array(const char **str, ApeConfigValue *out) {
	ape_config_skip_whitespace(str);

	if (**str != '[') { return APE_CONFIG_FALSE; }

	(*str)++; // Skip '['
	ape_config_skip_whitespace(str);

	out->type = APE_CONFIG_TYPE_ARRAY;
	out->data.array.count = 0;
	out->data.array.items = NULL;

	if (**str == ']') {
		(*str)++;
		return APE_CONFIG_TRUE; // Empty array
	}

	while (1) {
		ApeConfigValue item;
		if (!ape_config_parse_value(str, &item)) { return APE_CONFIG_FALSE; }

		out->data.array.count++;
		out->data.array.items = APE_CONFIG_REALLOC(out->data.array.items, out->data.array.count * sizeof(ApeConfigValue));
		out->data.array.items[out->data.array.count - 1] = item;

		ape_config_skip_whitespace(str);

		if (**str == ',') {
			(*str)++;
			continue;
		} else if (**str == ']') {
			(*str)++;
			break;
		} else {
			return APE_CONFIG_FALSE; // Expected ',' or ']'
		}
	}

	return APE_CONFIG_TRUE;
}

APE_CONFIG_PRIVATE int ape_config_parse_object(const char **str, ApeConfigValue *out) {
	ape_config_skip_whitespace(str);

	if (**str != '{') { return APE_CONFIG_FALSE; }

	(*str)++; // Skip '{'
	ape_config_skip_whitespace(str);

	out->type = APE_CONFIG_TYPE_OBJECT;
	out->data.object.count = 0;
	out->data.object.items = NULL;

	if (**str == '}') {
		(*str)++;
		return APE_CONFIG_TRUE; // Empty object
	}

	while (1) {
		char *key;
		if (!ape_config_parse_string(str, &key)) { return APE_CONFIG_FALSE; }

		ape_config_skip_whitespace(str);

		if (**str != ':') {
			APE_CONFIG_FREE(key);
			return APE_CONFIG_FALSE; // Expected ':'
		}

		(*str)++; // Skip ':'

		ApeConfigValue value;
		if (!ape_config_parse_value(str, &value)) {
			APE_CONFIG_FREE(key);
			return APE_CONFIG_FALSE;
		}

		out->data.object.count++;
		out->data.object.items = APE_CONFIG_REALLOC(out->data.object.items, out->data.object.count * sizeof(ApeConfigObjectItem));

		ApeConfigObjectItem *item = &out->data.object.items[out->data.object.count - 1];
		item->key = key;
		item->value = APE_CONFIG_MALLOC(sizeof(ApeConfigValue));
		*(item->value) = value;

		ape_config_skip_whitespace(str);

		if (**str == ',') {
			(*str)++;
			continue;
		} else if (**str == '}') {
			(*str)++;
			break;
		} else {
			return APE_CONFIG_FALSE; // Expected ',' or '}'
		}
	}

	return APE_CONFIG_TRUE;
}

APE_CONFIG_PRIVATE int ape_config_parse_value(const char **str, ApeConfigValue *out) {
	ape_config_skip_whitespace(str);

	if (**str == '"') {
		char *string_value;
		if (ape_config_parse_string(str, &string_value)) {
			out->type = APE_CONFIG_TYPE_STRING;
			out->data.string_value = string_value;
			return APE_CONFIG_TRUE;
		}
	} else if (**str == '{') {
		return ape_config_parse_object(str, out);
	} else if (**str == '[') {
		return ape_config_parse_array(str, out);
	} else if (**str == 't' && strncmp(*str, "true", 4) == 0) {
		out->type = APE_CONFIG_TYPE_BOOL;
		out->data.bool_value = APE_CONFIG_TRUE;
		*str += 4;
		return APE_CONFIG_TRUE;
	} else if (**str == 'f' && strncmp(*str, "false", 5) == 0) {
		out->type = APE_CONFIG_TYPE_BOOL;
		out->data.bool_value = APE_CONFIG_FALSE;
		*str += 5;
		return APE_CONFIG_TRUE;
	} else if (**str == 'n' && strncmp(*str, "null", 4) == 0) {
		out->type = APE_CONFIG_TYPE_STRING; // Treat null as empty string
		out->data.string_value = NULL;
		*str += 4;
		return APE_CONFIG_TRUE;
	} else if (isdigit(**str) || **str == '-' || **str == '.') {
		double number;
		if (ape_config_parse_number(str, &number)) {
			// Check if it's an integer
			if (number == (int64_t)number) {
				out->type = APE_CONFIG_TYPE_INT;
				out->data.int_value = (int64_t)number;
			} else {
				out->type = APE_CONFIG_TYPE_FLOAT;
				out->data.float_value = number;
			}
			return APE_CONFIG_TRUE;
		}
	}

	return APE_CONFIG_FALSE;
}

APE_CONFIG_DEF int ape_config_parse(ApeConfig *config, const char *str) {
	const char *ptr = str;

	// Clean up any existing data
	ape_config_cleanup(config);

	// Initialize root as object
	ape_config_init(config);

	// Parse the JSON
	if (!ape_config_parse_value(&ptr, &config->root)) { return APE_CONFIG_FALSE; }

	return APE_CONFIG_TRUE;
}

APE_CONFIG_DEF int ape_config_parse_file(ApeConfig *config, const char *path) {
	// TODO: Implement file reading and parsing
	return APE_CONFIG_FALSE;
}

APE_CONFIG_PRIVATE void ape_config_write_string(ApeStrBuilder *sb, const char *str) {
	if (!str) {
		ape_sb_append_str(sb, "null");
		return;
	}

	ape_sb_append_char(sb, '"');
	for (const char *p = str; *p; p++) {
		if (*p == '"') {
			ape_sb_append_str(sb, "\\");
		} else if (*p == '\\') {
			ape_sb_append_str(sb, "\\\\");
		} else if (*p == '\n') {
			ape_sb_append_str(sb, "\\n");
		} else if (*p == '\r') {
			ape_sb_append_str(sb, "\\r");
		} else if (*p == '\t') {
			ape_sb_append_str(sb, "\\t");
		} else {
			ape_sb_append_char(sb, *p);
		}
	}
	ape_sb_append_char(sb, '"');
}

APE_CONFIG_PRIVATE void ape_config_write_value(ApeStrBuilder *sb, ApeConfigValue *value);

APE_CONFIG_PRIVATE void ape_config_write_array(ApeStrBuilder *sb, ApeConfigValue *value) {
	if (value->type != APE_CONFIG_TYPE_ARRAY) {
		ape_sb_append_str(sb, "null");
		return;
	}

	ape_sb_append_char(sb, '[');

	for (size_t i = 0; i < value->data.array.count; i++) {
		if (i > 0) { ape_sb_append_str(sb, ", "); }

		ape_config_write_value(sb, &value->data.array.items[i]);
	}

	ape_sb_append_char(sb, ']');
}

APE_CONFIG_PRIVATE void ape_config_write_object(ApeStrBuilder *sb, ApeConfigValue *value) {
	if (value->type != APE_CONFIG_TYPE_OBJECT) {
		ape_sb_append_str(sb, "null");
		return;
	}

	ape_sb_append_char(sb, '{');

	for (size_t i = 0; i < value->data.object.count; i++) {
		if (i > 0) { ape_sb_append_str(sb, ", "); }

		ApeConfigObjectItem *item = &value->data.object.items[i];

		ape_config_write_string(sb, item->key);
		ape_sb_append_str(sb, ": ");

		ape_config_write_value(sb, item->value);
	}

	ape_sb_append_char(sb, '}');
}

APE_CONFIG_PRIVATE void ape_config_write_value(ApeStrBuilder *sb, ApeConfigValue *value) {
	switch (value->type) {
	case APE_CONFIG_TYPE_STRING: ape_config_write_string(sb, value->data.string_value); break;
	case APE_CONFIG_TYPE_INT: {
		char buffer[32];
		snprintf(buffer, sizeof(buffer), "%lld", (long long)value->data.int_value);
		ape_sb_append_str(sb, buffer);
	} break;
	case APE_CONFIG_TYPE_FLOAT: {
		char buffer[32];
		snprintf(buffer, sizeof(buffer), "%f", value->data.float_value);
		ape_sb_append_str(sb, buffer);
	} break;
	case APE_CONFIG_TYPE_BOOL: ape_sb_append_str(sb, value->data.bool_value ? "true" : "false"); break;
	case APE_CONFIG_TYPE_ARRAY: ape_config_write_array(sb, value); break;
	case APE_CONFIG_TYPE_OBJECT: ape_config_write_object(sb, value); break;
	}
}

APE_CONFIG_DEF char *ape_config_write(ApeConfig *config) {
	ApeStrBuilder sb = ape_sb_new();

	ape_config_write_value(&sb, &config->root);

	char *result = ape_sb_to_str_dup(&sb);
	ape_sb_free(&sb);

	return result;
}

APE_CONFIG_DEF int ape_config_write_file(ApeConfig *config, const char *path) {
	// TODO: Implement file writing
	return APE_CONFIG_FALSE;
}

APE_CONFIG_DEF ApeConfigValue *ape_config_get(ApeConfig *config, const char *path) {
	if (!path || strlen(path) == 0) { return &config->root; }

	// Use path navigation for nested objects
	return ape_config_get_or_create_path(config, path);
}

APE_CONFIG_DEF const char *ape_config_get_string(ApeConfig *config, const char *path, const char *default_value) {
	ApeConfigValue *value = ape_config_get(config, path);
	if (value && value->type == APE_CONFIG_TYPE_STRING) { return value->data.string_value; }
	return default_value;
}

APE_CONFIG_DEF int64_t ape_config_get_int(ApeConfig *config, const char *path, int64_t default_value) {
	ApeConfigValue *value = ape_config_get(config, path);
	if (value && value->type == APE_CONFIG_TYPE_INT) { return value->data.int_value; }
	return default_value;
}

APE_CONFIG_DEF double ape_config_get_float(ApeConfig *config, const char *path, double default_value) {
	ApeConfigValue *value = ape_config_get(config, path);
	if (value && value->type == APE_CONFIG_TYPE_FLOAT) { return value->data.float_value; }
	return default_value;
}

APE_CONFIG_DEF int ape_config_get_bool(ApeConfig *config, const char *path, int default_value) {
	ApeConfigValue *value = ape_config_get(config, path);
	if (value && value->type == APE_CONFIG_TYPE_BOOL) { return value->data.bool_value; }
	return default_value;
}

APE_CONFIG_DEF void ape_config_set_string(ApeConfig *config, const char *path, const char *value) {
	ApeConfigValue *target = ape_config_get_or_create_path(config, path);
	if (target && target->type == APE_CONFIG_TYPE_OBJECT) {
		ApeConfigObjectItem *item = ape_config_add_object_item(target, path);
		if (item) {
			// Free existing value
			if (item->value->type == APE_CONFIG_TYPE_STRING) { APE_CONFIG_FREE((void *)item->value->data.string_value); }

			// Set new value
			item->value->type = APE_CONFIG_TYPE_STRING;
			item->value->data.string_value = APE_CONFIG_MALLOC(strlen(value) + 1);
			strcpy((char *)item->value->data.string_value, value);
		}
	}
}

APE_CONFIG_DEF void ape_config_set_int(ApeConfig *config, const char *path, int64_t value) {
	ApeConfigValue *target = ape_config_get_or_create_path(config, path);
	if (target && target->type == APE_CONFIG_TYPE_OBJECT) {
		ApeConfigObjectItem *item = ape_config_add_object_item(target, path);
		if (item) {
			// Free existing value if it's a string
			if (item->value->type == APE_CONFIG_TYPE_STRING && item->value->data.string_value) {
				APE_CONFIG_FREE((void *)item->value->data.string_value);
			}

			// Set new value
			item->value->type = APE_CONFIG_TYPE_INT;
			item->value->data.int_value = value;
		}
	}
}

APE_CONFIG_DEF void ape_config_set_float(ApeConfig *config, const char *path, double value) {
	ApeConfigValue *target = ape_config_get_or_create_path(config, path);
	if (target && target->type == APE_CONFIG_TYPE_OBJECT) {
		ApeConfigObjectItem *item = ape_config_add_object_item(target, path);
		if (item) {
			// Free existing value if it's a string
			if (item->value->type == APE_CONFIG_TYPE_STRING && item->value->data.string_value) {
				APE_CONFIG_FREE((void *)item->value->data.string_value);
			}

			// Set new value
			item->value->type = APE_CONFIG_TYPE_FLOAT;
			item->value->data.float_value = value;
		}
	}
}

APE_CONFIG_DEF void ape_config_set_bool(ApeConfig *config, const char *path, int value) {
	ApeConfigValue *target = ape_config_get_or_create_path(config, path);
	if (target && target->type == APE_CONFIG_TYPE_OBJECT) {
		ApeConfigObjectItem *item = ape_config_add_object_item(target, path);
		if (item) {
			// Free existing value if it's a string
			if (item->value->type == APE_CONFIG_TYPE_STRING && item->value->data.string_value) {
				APE_CONFIG_FREE((void *)item->value->data.string_value);
			}

			// Set new value
			item->value->type = APE_CONFIG_TYPE_BOOL;
			item->value->data.bool_value = value;
		}
	}
}

APE_CONFIG_DEF int ape_config_has(ApeConfig *config, const char *path) {
	return ape_config_get(config, path) != NULL ? APE_CONFIG_TRUE : APE_CONFIG_FALSE;
}

APE_CONFIG_DEF ApeConfigType ape_config_type(ApeConfig *config, const char *path) {
	ApeConfigValue *value = ape_config_get(config, path);
	return value ? value->type : APE_CONFIG_TYPE_STRING;
}
/* END ape_config.c */

/* Internal functions */
APE_CONFIG_PRIVATE ApeConfigObjectItem *ape_config_find_object_item(ApeConfigValue *object, const char *key);
APE_CONFIG_PRIVATE ApeConfigObjectItem *ape_config_add_object_item(ApeConfigValue *object, const char *key);

#endif

#endif
