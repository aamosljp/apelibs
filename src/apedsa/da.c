#include "apedsa_internal.h"

void *__apedsa_da_growf(void *da, size_t esz, size_t growby, size_t min_cap)
{
	size_t min_len = apedsa_da_count(da) + growby;
	if (min_len > min_cap)
		min_cap = min_len;
	if (min_cap <= apedsa_da_cap(da))
		return da;
	if (min_cap < 2 * apedsa_da_cap(da))
		min_cap = 2 * apedsa_da_cap(da);
	else if (min_cap < 4)
		min_cap = 4;
	void *new_da = APEDSA_REALLOC((da) ? apedsa_da_header(da) : 0, min_cap * esz + sizeof(ApedsaDaHeader));
	new_da = (char *)new_da + sizeof(ApedsaDaHeader);
	if (da == NULL) {
		apedsa_da_header(new_da)->count = 0;
		apedsa_da_header(new_da)->aux = NULL;
		apedsa_da_header(new_da)->type = 0;
	}
	apedsa_da_header(new_da)->capacity = min_cap;
	return new_da;
}
