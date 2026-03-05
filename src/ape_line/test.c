#include <stdio.h>
#include <string.h>

#include "ape_line_api.h"

int main()
{
	ape_line_history_init(NULL);
	char *line;
	if (ape_line_init(&(ape_line_opts){ .raw_mode_cbreak = 1, .install_handlers = 1, .enable_vt = 1 }) == 0) {
		while (ape_line_read("test> ", &line) >= 0) {
			ape_line_puts(line);
			ape_line_puts("\n");
		}
	}
}
