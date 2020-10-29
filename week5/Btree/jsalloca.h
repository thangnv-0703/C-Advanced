/*JS*********************************************************************
*
*    Program : JSALLOCA
*    Language: ANSI-C
*    Author  : Joerg Schoen
*    Purpose : Test if 'alloca' is available on a platform and set
*              macro 'CONFIG_USE_ALLOCA'.
*
*************************************************************************/
#ifndef __JSALLOCA_H__
#define __JSALLOCA_H__

#include "jsconfig.h"

#ifndef CONFIG_NO_ALLOCA
# if defined(__cplusplus)
  /*  Better don't use alloca with C++  */
# elif defined(__sgi)
#  define CONFIG_USE_ALLOCA
#  include <alloca.h>  /*  need to include header  */
# elif defined(_AIX)
#  define CONFIG_USE_ALLOCA
#  pragma alloca       /*  pragma is needed  */
# elif defined(__linux)
#  define CONFIG_USE_ALLOCA /*  simply switch on  */
# endif
#endif

#endif
