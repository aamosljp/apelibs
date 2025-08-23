#include "template_api.h"

#ifndef TEMPLATE_IMPLEMENTATION_INCLUDED
#define TEMPLATE_IMPLEMENTATION_INCLUDED

/* User can define a custom function prefix (eg. static) */
#ifndef TEMPLATE_DEF
#define TEMPLATE_DEF
#endif

/* Can also define custom private function prefix */
#ifndef TEMPLATE_PRIVATE
#define TEMPLATE_PRIVATE static
#endif

#ifndef TEMPLATE_TRUE
#define TEMPLATE_TRUE (1)
#define TEMPLATE_FALSE (0)
#else
#if !defined(TEMPLATE_FALSE)
#pragma GCC error "Need to define both TEMPLATE_TRUE and TEMPLATE_FALSE or neither"
#endif
#endif

#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY

#endif
