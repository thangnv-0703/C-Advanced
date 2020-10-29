/*JS*********************************************************************
*
*    Program : JSGETOPT
*    Language: ANSI-C
*    Author  : Joerg Schoen. Original source from PDC distribution.
*    Purpose : My own get-option routine
*
*************************************************************************/

#ifndef lint
static const char rcsid[] = "$Id: jsgetopt.c,v 1.5 1996/10/23 16:13:45 joerg Stab joerg $";
#endif

/*********     INCLUDES 					*********/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*********     DEFINES  					*********/

#define Prototype extern
/*********     PROTOTYPES					*********/
Prototype int		 jsGetOpt(int *pargc,void *argv,const char *optstring,
				  char **poptarg);
Prototype int            jsFirstArg(char *argv[],const char *optstring);

Prototype int		 JSOptMode,JSOptIndex;

/*********     GLOBAL VARIABLES 				*********/
/*  Bit 0: Produce error messages on stderr.
 *  Bit 1: Mode STRICT: Options with additional
 *	   arguments must not be clustered.
 */
int JSOptMode = 1;

int JSOptIndex = 1; /*  Next arguments index  */

/*JS*********************************************************************
*   Routine to get options from the argument list or from a environment
*    variable. Returns the option if found. Deletes all used arguments.
*    If argv is NULL, all internal variables are reset to the default.
*    If pargc is NULL, then argv is assumed to be a pointer to a string
*    containing options. The scanning is done from the routine itself.
*    This is useful for scanning environment variables.
*************************************************************************/

int jsGetOpt(int *pargc,void *argv,const char *optstring,char **poptarg)

/************************************************************************/
{
  register int	  c;	      /*  Option letter 		       */
  register char   *cp;	      /*  -> option in `optstring'             */
static int Sp	  = 1;	      /*  Position within argument	       */
static char *CurrPos = NULL;  /*  Current position in argument string  */
static char *ArgBuff = NULL;  /*  Buffer for saving additional         */
static int   ArgBuffLen = 0;  /*   arguments of options in strings     */
  register int	  oldSp;      /*  Saved `sp' for param test            */
  register int	  saveArgc;   /*  Saved `argc' for param test          */

  /*   Preset optional argument if given   */
  if(poptarg) *poptarg = NULL;

  if(argv == NULL) goto eof;

  if(pargc) {
    /*	 Search for next argument starting with '-' if not already found   */
    if(Sp == 1) {
      for( ; JSOptIndex < *pargc ; JSOptIndex++) {
	/*  Skip arguments that are already used from someone	 */
	/*   else or that do not start with '-' or stdin ("-").  */
	if(((char**)argv)[JSOptIndex] == NULL ||
	   ((char**)argv)[JSOptIndex][0] != '-' ||
	   ((char**)argv)[JSOptIndex][1] == '\0') continue;

	/*  Seems to be option, check if end-of-options ("--")  */
	if(((char**)argv)[JSOptIndex][1] == '-') {
	  register int i;

	  /*  Skip long style options, "--opt", they are to
	   *   be processed from somebody else.
	   */
	  if(((char**)argv)[JSOptIndex][2] != '\0') continue;

	  /*  Delete this command line argument ("--")  */
	  for((*pargc)--, i = JSOptIndex ; i < *pargc ; i++)
	    ((char**)argv)[i] = ((char**)argv)[i + 1];
	  ((char **)argv)[*pargc] = NULL;

	  JSOptIndex = Sp = 1;
	  goto eof;
	}
	break;
      }

      /*   No more arguments?  */
      if(JSOptIndex == *pargc) goto eof;
    }

    /*	 Get option letter   */
    c = ((char**)argv)[JSOptIndex][Sp];
  } else {
    /* ***  Scanning of strings  *** */
    /*	Set up currPos if not already	*/
    if(CurrPos == NULL) CurrPos = (char *) argv;

    if(Sp == 1) {
      /*  Search next argument in string    */
      while(isspace(*(unsigned char *)CurrPos)) CurrPos++;

      /*  End of string?  */
      if(*CurrPos == '\0') goto eof;

      if(*CurrPos != '-') {
	/*   Only options allowed in string   */
	cp = "ERROR: No option in environment variable -- Skipping!\n";

	/*  Skip this  */
	while(*CurrPos && !isspace(*(unsigned char *)CurrPos)) CurrPos++;

	goto error;
      }
    }

    /*	 Get option letter   */
    c = CurrPos[Sp];
  }

  /*   Get ready for next letter   */
  oldSp = Sp++;

  /*   Check if end of argument    */
  if(pargc) {
    saveArgc = *pargc;	/* save argc for param test */

    if(((char**)argv)[JSOptIndex][Sp] == '\0') {
      register int i;

      /*  Delete this command line argument  */
      for((*pargc)--, i = JSOptIndex ; i < *pargc ; i++)
	((char**)argv)[i] = ((char**)argv)[i + 1];
      ((char **)argv)[*pargc] = NULL;

      Sp = 1;  /*  Beginning of next argument  */
    }
  } else {
    /* ***  Scanning of strings  *** */
    if(isspace(((unsigned char *)CurrPos)[Sp]) || CurrPos[Sp] == '\0') {
      CurrPos += Sp; /*  adjust CurrPos  */
      Sp = 1; /*  Beginning of next argument  */
    }
  }

  /*  ':' or ';' for options is not allowed! Search option in option string */
  if(c == ':' || c == ';' || (cp = strchr(optstring,c)) == NULL) {
    cp = "ERROR: Illegal option '-%c'!\n";
    goto error;
  }

  if(cp[1] == ':' || cp[1] == ';') { /*  Option takes parameter    */
    /*	Mode STRICT: Options with additional  */
    /*	 arguments must not be clustered.     */
    if((JSOptMode & 2) && oldSp != 1) {
      cp = "ERROR: Option '-%c' must not be clustered.\n";
      goto error;
    }

    if(pargc) {
      if(saveArgc == *pargc) {   /*  argument w/o whitespace  */
	register int i;

	if(poptarg) *poptarg = &((char**)argv)[JSOptIndex][Sp];

	/*  Delete this command line argument  */
	for((*pargc)--, i = JSOptIndex ; i < *pargc ; i++)
	  ((char**)argv)[i] = ((char**)argv)[i + 1];
	((char **)argv)[*pargc] = NULL;

	Sp = 1; 	/*   This one is used up for argument   */
      } else if(cp[1] == ':') { /*  Option requires argument, check next   */
	register int i;

	if(JSOptIndex >= *pargc) {
	  cp = "ERROR: Option '-%c' requires an argument.\n";
	  goto error;
	} else if(poptarg)  /* argument w/ whitespace */
	  *poptarg = ((char**)argv)[JSOptIndex];

	/*  Delete this command line argument  */
	for((*pargc)--, i = JSOptIndex ; i < *pargc ; i++)
	  ((char**)argv)[i] = ((char**)argv)[i + 1];
	((char **)argv)[*pargc] = NULL;
      }
    } else {
      char *start;
      int c2;

      if(Sp > 1) {
	CurrPos += Sp;
	Sp = 1; /*  restart  */
      } else if(cp[1] == ':') { /*  Option requires argument, check next   */
	while(*CurrPos && isspace(*(unsigned char *)CurrPos)) CurrPos++;

	if(CurrPos == '\0') {
	  cp = "ERROR: Option '-%c' requires an argument.\n";
	  goto error;
	}
      }

      /*  Seek end of argument  */
      if((c2 = *CurrPos) == '\'' || c2 == '\"') { /* " */
	start = ++CurrPos;

	while(*CurrPos && *CurrPos != c2) CurrPos++;

	/*  Mark end of string  */
	if(*CurrPos) *CurrPos++ = '\0';
      } else {
	start = CurrPos;

	while(*CurrPos && !isspace(*(unsigned char *)CurrPos) ) CurrPos++;

	if(*CurrPos) *CurrPos++ = '\0';
      }

      /*  Save additional argument in buffer  */
      if(ArgBuffLen <= (CurrPos - start) &&
	 (ArgBuff = (char *)realloc(ArgBuff,CurrPos - start + 2)) == NULL) {
	cp = "ERROR: No memory for optional arguments.\n";
	goto error;
      }
      memcpy(ArgBuff,start,CurrPos - start);
      ArgBuff[CurrPos - start] = '\0';

      if(poptarg) *poptarg = ArgBuff;
    }
  }

  /*   Return option letter    */
  return(c);
error:
  /*  Print message if user wants   */
  if(JSOptMode & 1)
    fprintf(stderr,cp,(char) c);

  return('?');
eof:
  /*  Reset all and return EOF    */
  JSOptIndex = Sp = 1;
  CurrPos = NULL;
  return(EOF);
}

/*JS*********************************************************************
*   Return index of first non-option argument in argv.
*************************************************************************/

int jsFirstArg(char *argv[],const char *optstring)

/************************************************************************/
{
  int index;

  for(index = 1 ; argv[index] && argv[index][0] == '-' ; index++) {
    int i;

    /*  "-" argument?  */
    if(argv[index][1] == '\0') break;

    /*  End of arguments  */
    if(argv[index][1] == '-' && argv[index][2] == '\0') {
      index++;
      break;
    }

    /*  Scan this option  */
    for(i = 1 ; argv[index][i] ; i++) {
      const char *s;

      if((s = strchr(optstring,argv[index][i]))) {
	if(s[1] == ':') {
	  /*  option requires an argument  */
	  if(argv[index][i+1] == '\0') index++;
	  break;
	} else if(s[1] == ';') {
	  /*  optional argument, skip remainder of this arg  */
	  break;
	}
      }
    }
  }

  return(index);
}
