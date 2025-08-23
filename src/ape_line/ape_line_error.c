#include "ape_line_internal.h"
#include <errno.h>
#include <string.h>

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
