# Strings Module TODO

Module for string manipulation, string builders, string lists, and generic dynamic arrays.

## String Builder

- [ ] ApeStrBuilder - Dynamic string/char buffer structure
- [ ] ape_sb_new - Create new string builder
- [ ] ape_sb_new_cap - Create with initial capacity
- [ ] ape_sb_free - Free string builder
- [ ] ape_sb_clear - Clear contents (keep capacity)
- [ ] ape_sb_append_char - Append single character
- [ ] ape_sb_append_str - Append C string
- [ ] ape_sb_append_strn - Append N characters from string
- [ ] ape_sb_append_sb - Append another string builder
- [ ] ape_sb_append_fmt - Append formatted string (printf-style)
- [ ] ape_sb_prepend_str - Prepend string
- [ ] ape_sb_insert - Insert string at position
- [ ] ape_sb_to_str - Get null-terminated string (returns internal buffer)
- [ ] ape_sb_to_str_dup - Get duplicated null-terminated string (caller owns)
- [ ] ape_sb_len - Get current length
- [ ] ape_sb_capacity - Get current capacity
- [ ] ape_sb_reserve - Reserve capacity
- [ ] ape_sb_shrink - Shrink capacity to fit contents

## String List

- [ ] ApeStrList - Dynamic array of strings
- [ ] ape_sl_new - Create new string list
- [ ] ape_sl_free - Free string list (and contained strings)
- [ ] ape_sl_free_shallow - Free list only (not contained strings)
- [ ] ape_sl_clear - Clear list
- [ ] ape_sl_append - Append string (takes ownership)
- [ ] ape_sl_append_dup - Append string (duplicates it)
- [ ] ape_sl_append_many - Append multiple strings
- [ ] ape_sl_prepend - Prepend string
- [ ] ape_sl_insert - Insert at position
- [ ] ape_sl_remove - Remove at position
- [ ] ape_sl_get - Get string at index
- [ ] ape_sl_len - Get list length
- [ ] ape_sl_contains - Check if list contains string
- [ ] ape_sl_index_of - Find index of string
- [ ] ape_sl_join - Join strings with separator
- [ ] ape_sl_clone - Clone string list

## Generic Dynamic Array

- [ ] APE_DA_DEFINE(name, type) - Macro to define typed dynamic array
- [ ] ape_da_init - Initialize dynamic array
- [ ] ape_da_free - Free dynamic array
- [ ] ape_da_append - Append single item
- [ ] ape_da_append_many - Append multiple items
- [ ] ape_da_prepend - Prepend item
- [ ] ape_da_insert - Insert at position
- [ ] ape_da_remove - Remove at position
- [ ] ape_da_pop - Remove and return last item
- [ ] ape_da_get - Get item at index
- [ ] ape_da_set - Set item at index
- [ ] ape_da_len - Get length
- [ ] ape_da_capacity - Get capacity
- [ ] ape_da_reserve - Reserve capacity
- [ ] ape_da_shrink - Shrink to fit
- [ ] ape_da_clear - Clear contents

## String Utilities

- [ ] ape_str_dup - Duplicate string
- [ ] ape_str_ndup - Duplicate N characters
- [ ] ape_str_concat - Concatenate strings (allocates new)
- [ ] ape_str_join - Join array of strings with separator
- [ ] ape_str_split - Split string by delimiter
- [ ] ape_str_split_lines - Split string into lines

## String Predicates

- [ ] ape_str_eq - Check string equality
- [ ] ape_str_eq_nocase - Check equality (case insensitive)
- [ ] ape_str_starts_with - Check if starts with prefix
- [ ] ape_str_ends_with - Check if ends with suffix
- [ ] ape_str_contains - Check if contains substring
- [ ] ape_str_is_empty - Check if empty or NULL

## String Transformation

- [ ] ape_str_trim - Trim whitespace (returns new string)
- [ ] ape_str_trim_left - Trim left whitespace
- [ ] ape_str_trim_right - Trim right whitespace
- [ ] ape_str_to_lower - Convert to lowercase
- [ ] ape_str_to_upper - Convert to uppercase
- [ ] ape_str_replace - Replace substring (first occurrence)
- [ ] ape_str_replace_all - Replace all occurrences
- [ ] ape_str_substr - Get substring

## String Searching

- [ ] ape_str_find - Find substring (returns index or -1)
- [ ] ape_str_find_char - Find character
- [ ] ape_str_rfind - Find from end
- [ ] ape_str_rfind_char - Find character from end

## String Conversion

- [ ] ape_str_to_int - Parse string as int
- [ ] ape_str_to_long - Parse string as long
- [ ] ape_str_to_float - Parse string as float
- [ ] ape_str_to_double - Parse string as double
- [ ] ape_str_from_int - Int to string
- [ ] ape_str_from_long - Long to string
- [ ] ape_str_from_float - Float to string
