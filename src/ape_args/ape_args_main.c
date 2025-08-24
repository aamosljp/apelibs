#include <stdlib.h>

#include "ape_args_internal.h"

APE_ARGS_DEF inline char *ape_args_shift_args(int *argc, char ***argv)
{
	if ((*argc) <= 0)
		return NULL;
	(*argc)--;
	char *result = (*argv)[0];
	(*argv)++;
	return result;
}

APE_ARGS_DEF int ape_args_parse_args(const ape_args_parse_opts *opts)
{
}
