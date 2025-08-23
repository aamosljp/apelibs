#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "ape_line_internal.h"

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
