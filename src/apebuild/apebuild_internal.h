#include "apebuild_api.h"

#ifndef APEBUILD_IMPLEMENTATION_INCLUDED
#define APEBUILD_IMPLEMENTATION_INCLUDED

/* User can define a custom function prefix (eg. static) */
#ifndef APEBUILD_DEF
#define APEBUILD_DEF
#endif

/* Can also define custom private function prefix */
#ifndef APEBUILD_PRIVATE
#define APEBUILD_PRIVATE static
#endif

#ifndef APEBUILD_TRUE
#define APEBUILD_TRUE (1)
#define APEBUILD_FALSE (0)
#else
#if !defined(APEBUILD_FALSE)
#pragma GCC error "Need to define both APEBUILD_TRUE and APEBUILD_FALSE or neither"
#endif
#endif

#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY

#endif
