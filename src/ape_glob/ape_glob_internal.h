#include "ape_glob_api.h"

#ifndef APE_GLOB_IMPLEMENTATION_INCLUDED
#define APE_GLOB_IMPLEMENTATION_INCLUDED

/* User can define a custom function prefix (eg. static) */
#ifndef APE_GLOB_DEF
#define APE_GLOB_DEF
#endif

/* Can also define custom private function prefix */
#ifndef APE_GLOB_PRIVATE
#define APE_GLOB_PRIVATE static
#endif

#ifndef APE_GLOB_TRUE
#define APE_GLOB_TRUE (1)
#define APE_GLOB_FALSE (0)
#else
#if !defined(APE_GLOB_FALSE)
#pragma GCC error "Need to define both APE_GLOB_TRUE and APE_GLOB_FALSE or neither"
#endif
#endif

#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY

#endif
