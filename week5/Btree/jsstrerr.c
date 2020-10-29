/*JS*********************************************************************
*
*    Program : JSSTRERROR
*    Language: ANSI-C
*    Author  : Joerg Schoen
*    Purpose : Get error string for internal error code.
*
*************************************************************************/

#ifndef lint
static const char rcsid[] = "$Id: jsstrerr.c,v 1.2 1997/07/20 21:21:05 joerg Stab joerg $";
#endif

/*********     INCLUDES                                         *********/
#include <stdio.h>
#include <string.h>

#include <jsconfig.h>

#include <jssubs.h>

#define Prototype extern
/*********     PROTOTYPES                                       *********/
Prototype const char    *jsStrError(int err);

/*JS*********************************************************************
*
*************************************************************************/

const char *jsStrError(int err)

/************************************************************************/
{
  if(err == 0) return("No error");

  if(JSErrors[0]) { /* ***  Read errors from file  *** */
    FILE *fp;
static char Buffer[100];

    if((fp = fopen(JSErrors[0],"r")) == NULL) goto nomsg;

    do {
      if(fgets(Buffer,100,fp) == NULL) goto nomsg;
    } while(err-- > 1);

    fclose(fp);

    /*  Delete trailing '\n'  */
    if(Buffer[0]) Buffer[strlen(Buffer) - 1] = '\0';
    return(Buffer);
  }

  return(JSErrors[err]);
nomsg:
  return("<FATAL: Error message not available>");
}
