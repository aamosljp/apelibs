#include "ape_lz_api.h"

#ifndef APE_LZ_IMPLEMENTATION_INCLUDED
#define APE_LZ_IMPLEMENTATION_INCLUDED

/* User can define a custom function prefix (eg. static) */
#ifndef APE_LZ_DEF
#define APE_LZ_DEF
#endif

/* Can also define custom private function prefix */
#ifndef APE_LZ_PRIVATE
#define APE_LZ_PRIVATE static
#endif

#ifndef APE_LZ_TRUE
#define APE_LZ_TRUE (1)
#define APE_LZ_FALSE (0)
#else
#if !defined(APE_LZ_FALSE)
#pragma GCC error "Need to define both APE_LZ_TRUE and APE_LZ_FALSE or neither"
#endif
#endif

#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY

#endif
