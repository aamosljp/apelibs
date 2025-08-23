#include "ape_args_api.h"

#ifndef APE_ARGS_IMPLEMENTATION_INCLUDED
#define APE_ARGS_IMPLEMENTATION_INCLUDED

/* User can define a custom function prefix (eg. static) */
#ifndef APE_ARGS_DEF
#define APE_ARGS_DEF
#endif

/* Can also define custom private function prefix */
#ifndef APE_ARGS_PRIVATE
#define APE_ARGS_PRIVATE static
#endif

#ifndef APE_ARGS_TRUE
#define APE_ARGS_TRUE (1)
#define APE_ARGS_FALSE (0)
#else
#if !defined(APE_ARGS_FALSE)
#pragma GCC error "Need to define both APE_ARGS_TRUE and APE_ARGS_FALSE or neither"
#endif
#endif

#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY

#endif
