/*JS*********************************************************************
*
*    Program : JSPERROR
*    Language: ANSI-C
*    Author  : Joerg Schoen
*    Purpose : Similar to "perror", but recognizes library errors as well
*
*************************************************************************/

#ifndef lint
static const char rcsid[] = "$Id$";
#endif

/*********     INCLUDES                                         *********/
#include <stdio.h>
#include <errno.h>

#include <jssubs.h>

#define Prototype extern
/*********     PROTOTYPES                                       *********/
Prototype void           jsperror(const char *s);

/*JS*********************************************************************
*   The difficult question is: which error code has precedence? Currently
*    this is the library.
*************************************************************************/

void jsperror(const char *s)

/************************************************************************/
{
  if(JSErrNo)
    fprintf(stderr,"%s: %s\n",s,jsStrError(JSErrNo));
  else
    /*  Leave handling of errno == 0 to perror :)  */
    perror(s);

  return;
}
