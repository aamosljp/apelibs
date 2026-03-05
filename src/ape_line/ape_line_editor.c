#include <string.h>
#include <stdio.h>

#include "ape_line_internal.h"

APE_LINE_DEF void ape_line_editor_reset(ape_line_editor *e)
{
	e->buf_cap = 0;
	e->buf_len = 0;
	APE_LINE_FREE(e->buf_items);
	e->buf_items = NULL;
	e->cursor = 0;
	e->inited = 1;
	e->last_char = 0;
}

APE_LINE_DEF void ape_line_editor_command(ape_line_editor *e, char c)
{
	if (!e->inited)
		ape_line_editor_reset(e);
	if (e->process_cmd_fn)
		return e->process_cmd_fn(e, c);
	if (c == '\b') {
		if (e->cursor > 0) {
			if (e->cursor < e->buf_len) {
				memmove(e->buf_items + e->cursor - 1, e->buf_items + e->cursor, e->buf_len - e->cursor);
			}
			e->buf_len--;
			e->cursor--;
			e->last_char = e->buf_items[e->cursor - 1];
		}
		ape_line_redraw();
		return;
	}
	if (c == '\n') {
		e->last_char = c;
		return;
	}
	if (e->buf_len + 1 >= e->buf_cap) {
		e->buf_cap = e->buf_cap ? e->buf_cap * 2 : 128;
		e->buf_items = (char *)APE_LINE_REALLOC(e->buf_items, e->buf_cap);
	}
	if (e->cursor < e->buf_len) {
		memmove(e->buf_items + e->cursor + 1, e->buf_items + e->cursor, e->buf_len - e->cursor);
	}
	e->buf_items[e->cursor++] = c;
	e->buf_len++;
	e->last_char = c;
}

APE_LINE_DEF void ape_line_editor_move(ape_line_editor *e, ssize_t offset)
{
	if (!e->inited)
		ape_line_editor_reset(e);
	if (offset < 0) {
		if (e->cursor + offset > 0) {
			e->cursor += offset;
		} else {
			e->cursor = 0;
		}
	} else {
		if (e->cursor + offset <= e->buf_len) {
			e->cursor += offset;
		} else {
			e->cursor = e->buf_len;
		}
	}
}

APE_LINE_DEF void ape_line_editor_goto(ape_line_editor *e, ssize_t pos)
{
	if (!e->inited)
		ape_line_editor_reset(e);
	if (pos < 0)
		e->cursor = 0;
	else if (pos > e->buf_len)
		e->cursor = e->buf_len;
	else
		e->cursor = pos;
}

APE_LINE_DEF char ape_line_editor_last_char(ape_line_editor *e)
{
	return e->last_char;
}

APE_LINE_DEF int ape_line_editor_set_str(ape_line_editor *e, char *s, size_t len)
{
	e->buf_cap = len % 16 + 16;
	e->buf_items = (char *)malloc(e->buf_cap);
	e->buf_len = len;
	memcpy(e->buf_items, s, len);
	e->cursor = e->buf_len;
	e->last_char = e->buf_items[e->buf_len - 1];
	return 0;
}
