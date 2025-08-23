#include "ape_line_api.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include "ape_line_internal.h"

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
