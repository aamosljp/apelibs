#include "ape_pack_api.h"

#ifndef APE_PACK_IMPLEMENTATION_INCLUDED
#define APE_PACK_IMPLEMENTATION_INCLUDED

/* User can define a custom function prefix (eg. static) */
#ifndef APE_PACK_DEF
#define APE_PACK_DEF
#endif

/* Can also define custom private function prefix */
#ifndef APE_PACK_PRIVATE
#define APE_PACK_PRIVATE static
#endif

#ifndef APE_PACK_TRUE
#define APE_PACK_TRUE (1)
#define APE_PACK_FALSE (0)
#else
#if !defined(APE_PACK_FALSE)
#pragma GCC error "Need to define both APE_PACK_TRUE and APE_PACK_FALSE or neither"
#endif
#endif

#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY

#endif
