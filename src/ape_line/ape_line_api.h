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
 *               ape_line_puts("\n");
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
