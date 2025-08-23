#include "ape_line_api.h"

#ifndef APE_LINE_IMPLEMENTATION_INCLUDED
#define APE_LINE_IMPLEMENTATION_INCLUDED

/* User can define a custom function prefix (eg. static) */
#ifndef APE_LINE_DEF
#define APE_LINE_DEF
#endif

/* Can also define custom private function prefix */
#ifndef APE_LINE_PRIVATE
#define APE_LINE_PRIVATE static
#endif

#ifndef APE_LINE_TRUE
#define APE_LINE_TRUE (1)
#define APE_LINE_FALSE (0)
#else
#if !defined(APE_LINE_FALSE)
#pragma GCC error "Need to define both APE_LINE_TRUE and APE_LINE_FALSE or neither"
#endif
#endif

/* Add any 'private' includes, typedefs etc. here
   Private means it is only included when APE_LINE_IMPLEMENTATION is defined by the user
*/

#define REPLACED_WITH_PRIVATE_CODE_DO_NOT_MODIFY

#endif
