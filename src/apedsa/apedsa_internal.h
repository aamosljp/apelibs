#include "apedsa_api.h"

#ifndef APEDSA_IMPLEMENTATION_INCLUDED
#define APEDSA_IMPLEMENTATION_INCLUDED

#include <stdint.h>
#include <string.h>

/* User can define a custom function prefix (eg. static) */
#ifndef APEDSA_DEF
#define APEDSA_DEF
#endif

/* Can also define custom private function prefix */
#ifndef APEDSA_PRIVATE
#define APEDSA_PRIVATE static
#endif

#ifndef APEDSA_TRUE
#define APEDSA_TRUE (1)
#define APEDSA_FALSE (0)
#else
#if !defined(APEDSA_FALSE)
#pragma GCC error "Need to define both APEDSA_TRUE and APEDSA_FALSE or neither"
#endif
#endif

#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY

#endif
