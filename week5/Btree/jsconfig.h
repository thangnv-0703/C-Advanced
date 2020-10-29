/*JS*********************************************************************
*
*    Program : JSCONFIG
*    Language: ANSI-C
*    Author  : Joerg Schoen
*    Purpose : Contains machine dependent settings for all programs.
*
*************************************************************************/
#ifndef __JSCONFIG_H__
#define __JSCONFIG_H__

/* *************************** */
/* ***   General settings  *** */
/* *************************** */
/* **  Force system header files to disable POSIX extensions  ** */
/*#define _POSIX_SOURCE*/

#ifdef __MSDOS__
/* **  Resort to the ANSI standard  ** */
# define CONFIG_NO_POSIX
# define CONFIG_NO_REGEX
#endif

/* **  Do we have 'strdup' (is neither POSIX nor ANSI conformant)?  ** */
/*#define CONFIG_NO_STRDUP*/

/* **  Do not use 'alloca' memory allocator  ** */
/*#define CONFIG_NO_ALLOCA*/

/* **  Set if your long ints have 64 bits  ** */
#ifdef __alpha
# define CONFIG_64BIT_LONG
#endif

/* **  System's delimiter for directories in a filename ** */
#define CONFIG_DIR_DELIMITER  '/'

/* **  Don't search directories starting with "."  ("." and ".." are
       never looked at)  ** */
#define CONFIG_IGNORE_POINTDIRS

/* **  Delimiter for PATH like variables  ** */
#define CONFIG_LIST_DELIMITER  ':'

/* **************************************** */
/* ***   What numerical library to use  *** */
/* **************************************** */
/*
 *  If none of the symbols 'CONFIG_LIB_IMSL', 'CONFIG_LIB_ESSL',
 *  'CONFIG_LIB_COMPLIB' or 'CONFIG_LIB_LAPACK' is defined,
 *  choose from the machine we are running on.
 */
#if !defined(CONFIG_LIB_IMSL) && !defined(CONFIG_LIB_ESSL) && \
    !defined(CONFIG_LIB_COMPLIB) && !defined(CONFIG_LIB_LAPACK) && \
    !defined(CONFIG_LIB_NOLIB)
# if defined(__sgi)  /* ***  SGI  *** */
#  define CONFIG_LIB_LAPACK
#  define CONFIG_LIB_COMPLIB
/*  Since I have had only trouble using IMSL, I disable it  */
/*#  define CONFIG_LIB_IMSL*/
# elif defined(_AIX) /* ***  AIX  *** */
#  define CONFIG_LIB_ESSL
/*  Since I have had only trouble using IMSL, I disable it  */
/*#  define CONFIG_LIB_IMSL*/
# endif
#endif

/* ***  File "usestat.c": Set global name of statistic file  ****/
/*
 *  My HOME directory on the different platforms the programs are
 *   compiled. Is used to locate user independent files (like
 *   the program usage statistic file).
 */

#ifndef CONFIG_HOME_DIR
# if defined(__sgi)
  /* ***  SGI  *** */
#  define CONFIG_HOME_DIR   "/usr/people/joerg"
# elif defined(_AIX) || defined(__hpux) || defined(__sparc) || defined(__osf__)
  /* *** AIX, HP, SUN or DEC  *** */
#  define CONFIG_HOME_DIR   "/u/pci/f81"
# elif defined(__linux)
  /* ***  LINUX  *** */
#  define CONFIG_HOME_DIR   "/home/joerg"
# endif
#endif

#ifndef STAT_FILENAME
# define STAT_FILENAME   CONFIG_HOME_DIR "/.prg_usage"
#endif

/* ***  Error codes are in separate file or in executable  */
#ifdef CONFIG_HOME_DIR
# ifndef CONFIG_ERRMSGFILE
/*#  define CONFIG_ERRMSGFILE   CONFIG_HOME_DIR "/lib/Error.txt"*/
# endif
#endif

/* ***  File "readparam.c": Customizations  ****/
#ifndef CONFIG_READPARAM_NO_EXPR
  /* **  Allow arithmetric expressions like $[3+4*5]  ** */
# define CONFIG_READPARAM_EXPR
#endif

/*  Since CONFIG_READPARAM_SHELLCMD uses "popen", disable
 *   it without POSIX
 */
#if !defined(CONFIG_READPARAM_NO_SHELLCMD) && !defined(CONFIG_NO_POSIX)
 /* ** Allow shell command evaluation like $(..) ** */
# define CONFIG_READPARAM_SHELLCMD
#endif

/* ***  File "compexpr.c": Use "intpow" if exponent is an integer value  *** */
#define CONFIG_USE_INTPOW

#ifdef __MSDOS__
/* ***  File "bigsort.c": Don't use to much memory under this ancient OS *** */
# define BIGSORT_MAXMEM   50000

/* ***  File "bufffile.c": Restrict memory usage under DOS  *** */
#define BUFFFILE_DEFHASHLEN     4
#define BUFFFILE_DEFPAGESHIFT  12
#endif

#ifdef __sgi
/* ***  File "bigsort.c": We need this for Silicon Graphics with -mips2  *** */
# define CONFIG_BIGSORT_ALIGN
#endif

/* ***  For "dbgalloc.c": File that contains line/File pairs of allocations
 * ***   that are to be considered OK
 */
#define CONFIG_DBGALLOC_OKFILE  CONFIG_HOME_DIR "/lib/dbgalloc.txt"

#endif /*  "#ifndef __JSCONFIG_H__"  */
