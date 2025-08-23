/*
 * ape_line.h - APE Line Input Library
 * -----------------------------------
 * Author: aamosljp (https://github.com/aamosljp)
 * Original Release Date: 2025-08-23
 * Current Version: 0.1.0
 *
 * License:
 *   This library is released into the public domain.
 *   You may use it for any purpose, commercial or private, without restriction.
 *
 * Description:
 *   ape_line is a small, single-header line editing library similar to GNU
 *   Readline, designed for writing shells and REPLs. It provides:
 *     - A termios abstraction for raw/cbreak modes (POSIX + Windows support).
 *     - A minimal line editor with cursor movement, backspace, and basic keys.
 *     - History management with optional file persistence.
 *     - Error handling and stringified error reporting.
 *     - Configurable callbacks for character handling, command execution,
 *       and detecting when a command is complete.
 *     - Simple, public domain distribution for easy embedding.
 *
 * Roadmap (roughly in order of importance):
 *     - Windows/cross-platform support
 *     - Search through the history (eg. Ctrl-R in bash)
 *     - Add default history file parser and writer
 *     - Signal-safe redraw and resize handling (SIGWINCH)
 *     - Add tab completion API
 *         - Custom callbacks
 *     - Add advanced line editing (word movement, delete word, etc.)
 *         - Multi-line editing (probably possible already with custom callbacks)
 *         - Possible vi-mode?
 *     - Option for using a custom line editor
 *     - Option for using a custom history backend
 *     - Colored/formatted output
 *         - Requires custom UTF-8/unicode aware string functions
 *     - Unit tests and more examples
 *
 * Usage Example:
 *   #define APE_LINE_IMPLEMENTATION
 *   #include "ape_line.h"
 *
 *   #include <stdio.h>
 *   #include <string.h>
 *
 *   int main()
 *   {
 *       ape_line_history_init(NULL);
 *       char *line;
 *       if (ape_line_init(&(ape_line_opts){ .raw_mode_cbreak = 1, .install_handlers = 1, .enable_vt = 1 }) == 0) {
 *           while (ape_line_read("test> ", &line) >= 0) {
 *               ape_line_puts(line);
 *               ape_line_puts("
");
 *           }
 *       }
 *   }
 *
 * Optional Macros (define before APE_LINE_IMPLEMENTATION):
 *   APE_LINE_MALLOC              // Custom malloc replacement
 *   APE_LINE_REALLOC             // Custom realloc replacement
 *   APE_LINE_FREE                // Custom free replacement
 *   APE_LINE_MAX_HISTFILE_LENGTH // Maximum length of history file path
 *   APE_LINE_STRIP_PREFIX        // Strip prefix from exported symbols
 *
 * Struct: ape_line_opts
 *   Options available when initializing ape_line:
 *     int raw_mode_cbreak;  // 0 = raw (full), 1 = cbreak (keeps ISIG etc.)
 *     int enable_vt;        // Windows: enable VT sequences
 *     int install_handlers; // POSIX: restore on SIGTSTP/SIGCONT
 *
 *     // Optional hooks (set to NULL or leave undefined to use defaults)
 *     ape_line_is_done_fn is_done_func;      // Return nonzero when a command is complete
 *     ape_line_exec_cmd_fn exec_cmd_func;    // Called when is_done_func() returns true
 *     ape_line_char_handler_fn char_handler_func; // Return nonzero if you handled c
 *
 * API Reference:
 *
 * Module: ape_line_editor
 *   void ape_line_editor_reset(ape_line_editor *e);
 *       Reset the internal state of the editor.
 *
 *   void ape_line_editor_command(ape_line_editor *e, char c);
 *       Process a single character input command.
 *
 *   void ape_line_editor_goto(ape_line_editor *e, ssize_t pos);
 *       Move the cursor to an absolute position in the buffer.
 *
 *   void ape_line_editor_move(ape_line_editor *e, ssize_t offset);
 *       Move the cursor relative to its current position.
 *
 *   char ape_line_editor_last_char(ape_line_editor *e);
 *       Get the last character entered into the buffer.
 *
 *   int ape_line_editor_set_str(ape_line_editor *e, char *s, size_t len);
 *       Set the editor buffer to the given string.
 *
 * Module: ape_line_history
 *   int ape_line_history_init(const char *histfile);
 *       Initialize history, optionally backed by a file.
 *
 *   int ape_line_history_shutdown(void);
 *       Shut down history and flush to disk if needed.
 *
 *   int ape_line_history_append(char *cmd, void *userdata);
 *       Append a new command to the history.
 *
 *   ape_line_history_entry *ape_line_history_get_index(size_t i);
 *       Get a history entry by index.
 *
 *   ape_line_history_entry *ape_line_history_next(void);
 *       Get the next history entry relative to the current pointer.
 *
 *   ape_line_history_entry *ape_line_history_previous(void);
 *       Get the previous history entry relative to the current pointer.
 *
 *   ape_line_history_entry *ape_line_history_get_last(void);
 *       Get the most recent history entry.
 *
 *   int ape_line_history_set_dirty(void);
 *       Mark history as modified (forces save on shutdown).
 *
 * Module: ape_line_error
 *   void ape_line_set_error(ape_line_error err);
 *       Set the last error state.
 *
 *   const char *ape_line_str_error(ape_line_error err);
 *       Convert an error enum to a string.
 *
 *   ape_line_error ape_line_last_error(void);
 *       Retrieve the last error that occurred.
 *
 * Module: ape_line_main
 *   int ape_line_init(const ape_line_opts *opts);
 *       Initialize ape_line with optional configuration.
 *
 *   void ape_line_shutdown(void);
 *       Restore terminal state and release resources.
 *
 *   ssize_t ape_line_read(const char *prompt, char **out_line);
 *       Read a line of input with editing support.
 *
 *   int ape_line_puts(const char *s);
 *       Write a string directly to the terminal output.
 *
 *   int ape_line_redraw(void);
 *       Redraw the current prompt and line buffer.
 *
 * Version History:
 *   0.1.0 2025-08-23 - Initial release with termios wrappers and basic line editor.
 */
#ifndef APE_LINE_INCLUDED
#define APE_LINE_INCLUDED

/* This file should include 'public' stuff (always included)

   Private code is ALL code that is inside '.c' files (you can also define filters in the generator).
*/

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define APE_LINE_WINDOWS
#if defined(_WIN64)
#define APE_LINE_WINDOWS_X64
#endif
#elif defined(__linux__) || defined(__unix__)
#define APE_LINE_LINUX
#elif defined(__APPLE__)
#define APE_LINE_APPLE
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#include <termios.h>
#include <stdlib.h>

#ifndef APE_LINE_MALLOC
#if defined(APE_LINE_REALLOC) || defined(APE_LINE_FREE)
#pragma GCC error "Need to define MALLOC, REALLOC and FREE or none of them"
#else
#define APE_LINE_MALLOC malloc
#define APE_LINE_REALLOC realloc
#define APE_LINE_FREE free
#endif
#else
#if !defined(APE_LINE_REALLOC) || !defined(APE_LINE_FREE)
#pragma GCC error "Need to define MALLOC, REALLOC and FREE or none of them"
#endif
#endif

#ifndef APE_LINE_MAX_HISTFILE_LENGTH
#define APE_LINE_MAX_HISTFILE_LENGTH 65536
#endif

typedef int (*ape_line_histfile_parser_func)(char *data, size_t len);
typedef int (*ape_line_histfile_writer_func)(char **buf);

typedef struct {
	size_t data_len;
	char *data;
	void *userdata;
} ape_line_history_entry;

typedef struct {
	size_t capacity;
	size_t count;
	ape_line_history_entry *items;
	ssize_t current_item;
	int histfile_fd;
	ape_line_histfile_parser_func histfile_parser;
	ape_line_histfile_writer_func histfile_writer;

	int inited;
} ape_line_history;

int ape_line_history_init(const char *histfilepath);
int ape_line_history_shutdown(void);
int ape_line_history_append(char *cmd, void *userdata);
ape_line_history_entry *ape_line_history_get_index(size_t i);
ape_line_history_entry *ape_line_history_next(void);
ape_line_history_entry *ape_line_history_previous(void);
ape_line_history_entry *ape_line_history_get_last(void);
int ape_line_history_set_dirty(void);

typedef struct ape_line_editor {
	ssize_t cursor;

	ssize_t buf_cap;
	ssize_t buf_len;
	char *buf_items;

	char last_char;

	void (*process_cmd_fn)(struct ape_line_editor *e, char c);

	int inited;
} ape_line_editor;

void ape_line_editor_reset(ape_line_editor *e);
void ape_line_editor_command(ape_line_editor *e, char c);
void ape_line_editor_goto(ape_line_editor *e, ssize_t pos);
void ape_line_editor_move(ape_line_editor *e, ssize_t offset); // <0 = left, >0 = right
char ape_line_editor_last_char(ape_line_editor *e);
// void ape_line_editor_command(char *s); // Accepts some vim-like commands
int ape_line_editor_set_str(ape_line_editor *e, char *s, size_t len);

typedef int (*ape_line_is_done_fn)();
typedef int (*ape_line_exec_cmd_fn)(char *cmd);
typedef int (*ape_line_char_handler_fn)(char c);

typedef struct {
	int raw_mode_cbreak; // 0=raw (full), 1=cbreak (keeps ISIG etc.)
	int enable_vt; // win: enable VT sequences
	int install_handlers; // restore on SIGTSTP/SIGCONT (posix)

	// Optional hooks (set to NULL or leave undefined to use defaults)
	ape_line_is_done_fn is_done_func; // Return nonzero when a command is complete
	ape_line_exec_cmd_fn exec_cmd_func; // Called when is_done_func() returns true
	ape_line_char_handler_fn char_handler_func; // Return nonzero if you handled c
} ape_line_opts;

typedef struct {
	int s_in_fd, s_out_fd;
	struct termios s_saved_in, s_raw_in;
	unsigned s_depth;
	int s_initialized, s_is_tty;
	ape_line_opts s_opts;
	ape_line_editor s_editor;
	int s_esc_seq;
	int s_is_done;
	size_t s_prompt_len;
	char *s_prompt;
} ape_line_state;

int ape_line_init(const ape_line_opts *opts);
void ape_line_shutdown(void);
ssize_t ape_line_read(const char *prompt, char **out_line);
int ape_line_puts(const char *s);
int ape_line_redraw(void);

typedef enum {
	APE_LINE_ERROR_NONE = 0,
	APE_LINE_ERROR_WRITE,
	APE_LINE_ERROR_NOT_TTY,
	APE_LINE_ERROR_NOT_INITIALIZED,
	APE_LINE_ERROR_NO_OUTLINE,
	APE_LINE_ERROR_NO_PROMPT,
	APE_LINE_ERROR_INTERRUPT,
	APE_LINE_ERROR_READ,
} ape_line_error;

void ape_line_set_error(ape_line_error err);
const char *ape_line_str_error(ape_line_error err);
ape_line_error ape_line_last_error(void);

#if defined(__cplusplus)
}
#endif

#if defined(APE_LINE_STRIP_PREFIX)
#define LINE_FALSE APE_LINE_FALSE
#define LINE_TRUE APE_LINE_TRUE
#define line_editor ape_line_editor
#define line_editor_reset ape_line_editor_reset
#define line_editor_command ape_line_editor_command
#define line_editor_goto ape_line_editor_goto
#define line_editor_move ape_line_editor_move
#define line_editor_last_char ape_line_editor_last_char
#define line_editor_set_str ape_line_editor_set_str
#define line_is_done_fn ape_line_is_done_fn
#define line_exec_cmd_fn ape_line_exec_cmd_fn
#define line_char_handler_fn ape_line_char_handler_fn
#define line_opts ape_line_opts
#define line_state ape_line_state
#define line_init ape_line_init
#define line_read ape_line_read
#define line_puts ape_line_puts
#define line_redraw ape_line_redraw
#define line_error ape_line_error
#define LINE_ERROR_NONE APE_LINE_ERROR_NONE
#define LINE_ERROR_WRITE APE_LINE_ERROR_WRITE
#define LINE_ERROR_NOT_TTY APE_LINE_ERROR_NOT_TTY
#define LINE_ERROR_NOT_INITIALIZED APE_LINE_ERROR_NOT_INITIALIZED
#define LINE_ERROR_NO_OUTLINE APE_LINE_ERROR_NO_OUTLINE
#define LINE_ERROR_NO_PROMPT APE_LINE_ERROR_NO_PROMPT
#define LINE_ERROR_INTERRUPT APE_LINE_ERROR_INTERRUPT
#define LINE_ERROR_READ APE_LINE_ERROR_READ
#define line_set_error ape_line_set_error
#define line_str_error ape_line_str_error
#define line_last_error ape_line_last_error
#define line_histfile_parser_func ape_line_histfile_parser_func
#define line_histfile_writer_func ape_line_histfile_writer_func
#define line_history_entry ape_line_history_entry
#define line_history ape_line_history;
#define line_history_init ape_line_history_init
#define line_history_shutdown ape_line_history_shutdown
#define line_history_append ape_line_history_append
#define line_history_get_index ape_line_history_get_index
#define line_history_next ape_line_history_next
#define line_history_previous ape_line_history_previous
#define line_history_get_last ape_line_history_get_last
#define line_history_set_dirty ape_line_history_set_dirty
#endif

#endif

#ifdef APE_LINE_IMPLEMENTATION

#ifndef APE_LINE_IMPLEMENTATION_INCLUDED
#define APE_LINE_IMPLEMENTATION_INCLUDED

/* User can define a custom function prefix (eg. static) */
#ifndef APE_LINE_DEF
#define APE_LINE_DEF
#endif

/* Can also define custom private function prefix */
#ifndef APE_LINE_PRIVATE
#define APE_LINE_PRIVATE static
#endif

#ifndef APE_LINE_TRUE
#define APE_LINE_TRUE (1)
#define APE_LINE_FALSE (0)
#else
#if !defined(APE_LINE_FALSE)
#pragma GCC error "Need to define both APE_LINE_TRUE and APE_LINE_FALSE or neither"
#endif
#endif

/* Add any 'private' includes, typedefs etc. here
   Private means it is only included when APE_LINE_IMPLEMENTATION is defined by the user
*/

#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include "ape_line_api.h"
#include <fcntl.h>
#include <errno.h>

/* BEGIN ape_line_editor.c */
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
/* END ape_line_editor.c */

/* BEGIN ape_line_error.c */
ape_line_error _ape_line_last_error = APE_LINE_ERROR_NONE;
APE_LINE_DEF void ape_line_set_error(ape_line_error err)
{
	_ape_line_last_error = APE_LINE_ERROR_NONE;
}
APE_LINE_DEF const char *ape_line_str_error(ape_line_error err)
{
	switch (err) {
	case APE_LINE_ERROR_NONE:
		return "";
	case APE_LINE_ERROR_WRITE:
		return strerror(errno);
	case APE_LINE_ERROR_NOT_TTY:
		return "Not inside a tty!\n";
	case APE_LINE_ERROR_NOT_INITIALIZED:
		return "Need to call ape_line_init() first!\n";
	case APE_LINE_ERROR_NO_OUTLINE:
		return "3rd parameter can not be NULL\n";
	case APE_LINE_ERROR_NO_PROMPT:
		return "Prompt cannot be NULL\n";
	case APE_LINE_ERROR_INTERRUPT:
		return "Interrupt\n";
	case APE_LINE_ERROR_READ:
		return strerror(errno);
	}
}
APE_LINE_DEF ape_line_error ape_line_last_error()
{
	return _ape_line_last_error;
}
/* END ape_line_error.c */

/* BEGIN ape_line_history.c */
static ape_line_history _ape_line_history_ = { 0 };
APE_LINE_PRIVATE int ape_line_parse_histfile()
{
	if (_ape_line_history_.inited)
		return -1;
	if (_ape_line_history_.histfile_parser) {
		char buf[APE_LINE_MAX_HISTFILE_LENGTH];
		size_t rn = read(_ape_line_history_.histfile_fd, buf, APE_LINE_MAX_HISTFILE_LENGTH);
		if (rn <= 0) {
			perror("Read histfile: ");
			return -1;
		}
		return _ape_line_history_.histfile_parser(buf, rn);
	}
	return 0;
}
APE_LINE_PRIVATE int ape_line_save_histfile()
{
	if (!_ape_line_history_.inited)
		return -1;
	if (_ape_line_history_.histfile_writer) {
		char *buf;
		size_t rn = _ape_line_history_.histfile_writer(&buf);
		size_t wn = write(_ape_line_history_.histfile_fd, buf, rn);
		if (wn <= 0) {
			perror("Write histfile: ");
			return -1;
		}
		return wn;
	}
	return 0;
}
APE_LINE_DEF int ape_line_history_init(const char *histfilepath)
{
	if (_ape_line_history_.inited)
		return -1;
	if (histfilepath) {
		_ape_line_history_.histfile_fd = open(histfilepath, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_ISUID | S_ISGID);
		if (_ape_line_history_.histfile_fd == -1) {
			perror("Open histfile: ");
		}
		int e = 0;
		if ((e = ape_line_parse_histfile()) <= 0) {
			return e;
		}
	} else {
		_ape_line_history_.histfile_fd = -1;
	}
	_ape_line_history_.current_item = -1;
	_ape_line_history_.capacity = 128;
	_ape_line_history_.count = 0;
	_ape_line_history_.items = (ape_line_history_entry *)APE_LINE_MALLOC(_ape_line_history_.capacity * sizeof(ape_line_history_entry));
	_ape_line_history_.inited = 1;
	return 0;
}
APE_LINE_DEF int ape_line_history_shutdown()
{
	if (!_ape_line_history_.inited)
		return -1;
	int e;
	if (_ape_line_history_.histfile_fd != -1) {
		if ((e = ape_line_save_histfile()) < 0)
			return e;
		if (_ape_line_history_.histfile_fd)
			close(_ape_line_history_.histfile_fd);
	}
	free(_ape_line_history_.items);
	_ape_line_history_.inited = 0;
	return 0;
}
APE_LINE_DEF int ape_line_history_append(char *cmd, void *userdata)
{
	if (!_ape_line_history_.inited)
		return -1;
	if (_ape_line_history_.count + 1 >= _ape_line_history_.capacity) {
		_ape_line_history_.capacity *= 2;
		_ape_line_history_.items = (ape_line_history_entry *)APE_LINE_REALLOC(
			_ape_line_history_.items, _ape_line_history_.capacity * sizeof(ape_line_history_entry));
	}
	_ape_line_history_.items[_ape_line_history_.count++] = (ape_line_history_entry){
		.data_len = strlen(cmd),
		.data = strdup(cmd),
		.userdata = userdata,
	};
	(void)ape_line_history_get_last();
	return _ape_line_history_.current_item = _ape_line_history_.count;
}
APE_LINE_DEF ape_line_history_entry *ape_line_history_get_index(size_t i)
{
	if (!_ape_line_history_.inited)
		return NULL;
	if (i < _ape_line_history_.count) {
		return _ape_line_history_.items + i;
	}
	return NULL;
}
APE_LINE_DEF ape_line_history_entry *ape_line_history_next()
{
	if (!_ape_line_history_.inited)
		return NULL;
	if (_ape_line_history_.current_item < 0)
		return NULL;
	if (_ape_line_history_.current_item + 1 < _ape_line_history_.count) {
		return _ape_line_history_.items + (++_ape_line_history_.current_item);
	}
	if (_ape_line_history_.current_item + 1 == _ape_line_history_.count)
		return NULL;
	return _ape_line_history_.items + _ape_line_history_.current_item;
}
APE_LINE_DEF ape_line_history_entry *ape_line_history_previous()
{
	if (!_ape_line_history_.inited)
		return NULL;
	if (_ape_line_history_.current_item < 0) {
		// ape_line_history_get_last();
		// _ape_line_history_.current_item++;
		return NULL;
	}
	if (_ape_line_history_.current_item > 0) {
		return _ape_line_history_.items + (--_ape_line_history_.current_item);
	}
	return _ape_line_history_.items + _ape_line_history_.current_item;
}
APE_LINE_DEF ape_line_history_entry *ape_line_history_get_last()
{
	if (!_ape_line_history_.inited)
		return NULL;
	if (_ape_line_history_.count > 0) {
		_ape_line_history_.current_item = _ape_line_history_.count;
		return _ape_line_history_.items + _ape_line_history_.current_item;
	}
	return NULL;
}
APE_LINE_DEF int ape_line_history_set_dirty()
{
	if (!_ape_line_history_.inited)
		return -1;
	_ape_line_history_.current_item = -1;
	return 0;
}
/* END ape_line_history.c */

/* BEGIN ape_line_main.c */
static ape_line_state _ape_line_state_g = { 0 };
APE_LINE_PRIVATE void ape_line_apply_raw(void)
{
	if (!_ape_line_state_g.s_is_tty)
		return;
	tcsetattr(_ape_line_state_g.s_in_fd, TCSAFLUSH, &_ape_line_state_g.s_raw_in);
}
APE_LINE_PRIVATE void ape_line_restore(void)
{
	if (!_ape_line_state_g.s_is_tty)
		return;
	tcsetattr(_ape_line_state_g.s_in_fd, TCSAFLUSH, &_ape_line_state_g.s_saved_in);
}
APE_LINE_PRIVATE void ape_line_at_exit(void)
{
	ape_line_restore();
	ape_line_history_shutdown();
}
APE_LINE_PRIVATE void ape_line_signal_handler(int sig)
{
	switch (sig) {
	case SIGTSTP:
		ape_line_restore();
		signal(SIGTSTP, SIG_DFL);
		raise(SIGTSTP);
		break;
	case SIGCONT:
		if (_ape_line_state_g.s_depth > 0)
			ape_line_apply_raw();
		break;
	case SIGINT:
		ape_line_editor_reset(&_ape_line_state_g.s_editor);
		ape_line_puts("^C\n");
		ape_line_redraw();
	default:
		break;
	}
}
APE_LINE_PRIVATE void ape_line_install_handlers()
{
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = ape_line_signal_handler;
	sigaction(SIGTSTP, &sa, NULL);
	sigaction(SIGCONT, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);
}
APE_LINE_PRIVATE int ape_line_def_is_done()
{
	int result = _ape_line_state_g.s_is_done == APE_LINE_TRUE || ape_line_editor_last_char(&_ape_line_state_g.s_editor) == '\n' ?
			     APE_LINE_TRUE :
			     APE_LINE_FALSE;
	if (result == APE_LINE_TRUE)
		_ape_line_state_g.s_is_done = APE_LINE_FALSE;
	return result;
}
APE_LINE_PRIVATE int ape_line_def_exec_cmd(char *cmd)
{
	if (strlen(cmd) > 0)
		ape_line_history_append(cmd, NULL);
	if (strcmp(cmd, "exit") == 0) {
		exit(0);
	}
	return 0;
}
APE_LINE_PRIVATE int ape_line_def_char_handler(char c)
{
	static char seq[3];
	static int seq_i = 0;
	if (_ape_line_state_g.s_esc_seq) {
		seq[seq_i++] = c;
		if (seq_i == 2) {
			if (seq[0] == '[') {
				switch (seq[1]) {
				case 'A': /* up */
				{
					ape_line_history_entry *ent = ape_line_history_previous();
					if (ent) {
						ape_line_editor_set_str(&_ape_line_state_g.s_editor, ent->data, ent->data_len);
					}
					ape_line_redraw();
				} break;
				case 'B': /* down */
				{
					ape_line_history_entry *ent = ape_line_history_next();
					if (ent) {
						ape_line_editor_set_str(&_ape_line_state_g.s_editor, ent->data, ent->data_len);
					}
					ape_line_redraw();
				} break;
				case 'C': /* right */
					ape_line_editor_move(&_ape_line_state_g.s_editor, 1);
					break;
				case 'D': /* left */
					ape_line_editor_move(&_ape_line_state_g.s_editor, -1);
					break;
				}
			}
			_ape_line_state_g.s_esc_seq = 0;
		}
		return APE_LINE_TRUE;
	}
	if (c == '\r' || c == '\n') {
		ape_line_editor_command(&_ape_line_state_g.s_editor, '\n');
		_ape_line_state_g.s_is_done = APE_LINE_TRUE;
		(void)!ape_line_puts("\r\n");
		return APE_LINE_TRUE;
	}
	if (c == 0x1b) { // Escape sequence
		_ape_line_state_g.s_esc_seq = 1;
		seq_i = 0;
		memset(seq, 0, 3);
		return APE_LINE_TRUE;
	}
	ape_line_history_set_dirty();
	if (c == 0x7f || c == 0x08) {
		ape_line_editor_command(&_ape_line_state_g.s_editor, '\b');
		(void)!ape_line_puts("\b \b");
		return APE_LINE_TRUE;
	}
	if (c >= 0x20 || c == '\t') {
		ape_line_editor_command(&_ape_line_state_g.s_editor, c);
		(void)!ape_line_puts(&c);
		return APE_LINE_TRUE;
	}
	return APE_LINE_FALSE;
}
APE_LINE_DEF int ape_line_init(const ape_line_opts *opts)
{
	if (_ape_line_state_g.s_initialized)
		return 0;
	_ape_line_state_g.s_opts = (opts) ? *opts : (ape_line_opts){ 0 };
	_ape_line_state_g.s_opts.is_done_func = opts->is_done_func ? opts->is_done_func : ape_line_def_is_done;
	_ape_line_state_g.s_opts.exec_cmd_func = opts->exec_cmd_func ? opts->exec_cmd_func : ape_line_def_exec_cmd;
	_ape_line_state_g.s_opts.char_handler_func = opts->char_handler_func ? opts->char_handler_func : ape_line_def_char_handler;
	_ape_line_state_g.s_in_fd = STDIN_FILENO;
	_ape_line_state_g.s_out_fd = STDOUT_FILENO;
	_ape_line_state_g.s_is_tty = isatty(_ape_line_state_g.s_in_fd) && isatty(_ape_line_state_g.s_out_fd);
	if (_ape_line_state_g.s_is_tty) {
		if (tcgetattr(_ape_line_state_g.s_in_fd, &_ape_line_state_g.s_saved_in) < 0)
			return -1;
		_ape_line_state_g.s_raw_in = _ape_line_state_g.s_saved_in;
		_ape_line_state_g.s_raw_in.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
		_ape_line_state_g.s_raw_in.c_oflag &= ~(OPOST);
		_ape_line_state_g.s_raw_in.c_cflag |= (CS8);
		_ape_line_state_g.s_raw_in.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
		if (_ape_line_state_g.s_opts.raw_mode_cbreak)
			_ape_line_state_g.s_raw_in.c_lflag |= ISIG;
		_ape_line_state_g.s_raw_in.c_cc[VMIN] = 1;
		_ape_line_state_g.s_raw_in.c_cc[VTIME] = 0;
		atexit(ape_line_at_exit);
		if (_ape_line_state_g.s_opts.install_handlers)
			ape_line_install_handlers();
	}
	_ape_line_state_g.s_initialized = APE_LINE_TRUE;
	return 0;
}
APE_LINE_DEF void ape_line_shutdown()
{
	if (!_ape_line_state_g.s_initialized)
		return;
	ape_line_restore();
	ape_line_history_shutdown();
	_ape_line_state_g.s_depth = 0;
	_ape_line_state_g.s_initialized = 0;
}
APE_LINE_PRIVATE int ape_line_write_prompt()
{
	char *p = (char *)APE_LINE_MALLOC(_ape_line_state_g.s_prompt_len + 1);
	memcpy(p, _ape_line_state_g.s_prompt, _ape_line_state_g.s_prompt_len);
	p[_ape_line_state_g.s_prompt_len] = 0;
	if (!p)
		return 0;
	(void)!write(_ape_line_state_g.s_out_fd, "\r\x1b[K", 4);
	size_t n = strlen(p), w = 0;
	while (w < n) {
		ssize_t r = write(_ape_line_state_g.s_out_fd, p + w, n - w);
		if (r < 0) {
			if (errno == EINTR)
				continue;
			ape_line_set_error(APE_LINE_ERROR_WRITE);
			return -1;
		}
		w += (size_t)r;
	}
	return 0;
}
APE_LINE_DEF int ape_line_puts(const char *s)
{
	size_t n = strlen(s), w = 0;
	while (w < n) {
		ssize_t r = write(_ape_line_state_g.s_out_fd, s + w, n - w);
		if (r < 0) {
			if (errno == EINTR)
				continue;
			ape_line_set_error(APE_LINE_ERROR_WRITE);
			return -1;
		}
		w += (size_t)r;
	}
	return 0;
}
APE_LINE_DEF int ape_line_redraw()
{
	if (!_ape_line_state_g.s_prompt)
		return 0;
	(void)!write(_ape_line_state_g.s_out_fd, "\x1b[2K", 4);
	ape_line_write_prompt();
	char *s = (char *)APE_LINE_MALLOC(_ape_line_state_g.s_editor.buf_len);
	memcpy(s, _ape_line_state_g.s_editor.buf_items, _ape_line_state_g.s_editor.buf_len);
	(void)!write(_ape_line_state_g.s_out_fd, s, _ape_line_state_g.s_editor.buf_len);
	char *ss = (char *)APE_LINE_MALLOC(6);
	int l = sprintf(ss, "\x1b[%ldG", _ape_line_state_g.s_editor.cursor + _ape_line_state_g.s_prompt_len + 1);
	(void)!write(_ape_line_state_g.s_out_fd, ss, l);
	return 0;
}
APE_LINE_DEF ssize_t ape_line_read(const char *prompt, char **out_line)
{
	if (!_ape_line_state_g.s_is_tty) {
		ape_line_set_error(APE_LINE_ERROR_NOT_TTY);
		return -1;
	}
	if (!_ape_line_state_g.s_initialized) {
		ape_line_set_error(APE_LINE_ERROR_NOT_INITIALIZED);
		return -1;
	}
	if (!out_line) {
		ape_line_set_error(APE_LINE_ERROR_NO_OUTLINE);
		return -1;
	}
	if (!prompt) {
		ape_line_set_error(APE_LINE_ERROR_NO_PROMPT);
		return -1;
	}
	*out_line = NULL;
	_ape_line_state_g.s_prompt_len = strlen(prompt);
	_ape_line_state_g.s_prompt = (char *)APE_LINE_MALLOC(_ape_line_state_g.s_prompt_len);
	memcpy(_ape_line_state_g.s_prompt, prompt, _ape_line_state_g.s_prompt_len);
	if (_ape_line_state_g.s_depth++ == 0)
		ape_line_apply_raw();
	if (ape_line_write_prompt() < 0) {
		_ape_line_state_g.s_depth--;
		return -1;
	}
	ape_line_editor_reset(&_ape_line_state_g.s_editor);
	for (;;) {
		ape_line_redraw();
		unsigned char ch;
		ssize_t r = read(_ape_line_state_g.s_in_fd, &ch, 1);
		if (r < 0) {
			if (errno == EINTR) { // Ctrl-C in cbreak OR signals
				// Emit newline to keep shell tidy, surface EINTR
				ape_line_puts("^C\r\n");
				ape_line_editor_reset(&_ape_line_state_g.s_editor);
				_ape_line_state_g.s_depth--;
				ape_line_set_error(APE_LINE_ERROR_INTERRUPT);
				errno = EINTR;
				return -1;
			}
			ape_line_editor_reset(&_ape_line_state_g.s_editor);
			_ape_line_state_g.s_depth--;
			ape_line_set_error(APE_LINE_ERROR_READ);
			return -1;
		}
		if (r == 0) { // EOF
			ape_line_editor_reset(&_ape_line_state_g.s_editor);
			_ape_line_state_g.s_depth--;
			return 0;
		}
		if (_ape_line_state_g.s_opts.char_handler_func((char)ch) == APE_LINE_FALSE) {
			continue;
		}
		if (_ape_line_state_g.s_opts.is_done_func()) {
			if (_ape_line_state_g.s_editor.buf_len > 0 &&
			    _ape_line_state_g.s_editor.buf_items[_ape_line_state_g.s_editor.buf_len - 1] == '\n') {
				ssize_t old_cursor = _ape_line_state_g.s_editor.cursor;
				ape_line_editor_goto(&_ape_line_state_g.s_editor, _ape_line_state_g.s_editor.buf_len);
				// ape_line_editor_enter_mode(&_ape_line_state_g.s_editor, APE_LINE_EDITOR_MODE_INSERT);
				ape_line_editor_command(&_ape_line_state_g.s_editor, '\b');
			}
			if (out_line) {
				*out_line = (char *)APE_LINE_MALLOC(_ape_line_state_g.s_editor.buf_len + 1);
				if (!*out_line) {
					ape_line_editor_reset(&_ape_line_state_g.s_editor);
					ape_line_set_error(APE_LINE_ERROR_NO_OUTLINE);
					return -1;
				}
				memcpy(*out_line, _ape_line_state_g.s_editor.buf_items, _ape_line_state_g.s_editor.buf_len);
				(*out_line)[_ape_line_state_g.s_editor.buf_len] = '\0';
			}
			(void)_ape_line_state_g.s_opts.exec_cmd_func(*out_line);
			ssize_t ret_len = (ssize_t)_ape_line_state_g.s_editor.buf_len;
			ape_line_editor_reset(&_ape_line_state_g.s_editor);
			return ret_len;
		}
	}
}
/* END ape_line_main.c */

#endif

#endif
