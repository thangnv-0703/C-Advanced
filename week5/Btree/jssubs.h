/*JS*********************************************************************
*
*    Program : JSSUBS.H
*    Language: ANSI-C
*    Author  : Joerg Schoen
*    Purpose : Include file for library 'libjssubs.a' and all programs
*		that use this library.
*
*************************************************************************/
#ifndef __JSSUBS_H__ /*   File must not be included more than once!   */
#define __JSSUBS_H__

/*********     GENERAL DEFINES					*********/
#ifndef FALSE
# define FALSE	0
#endif
#ifndef TRUE
# define TRUE	1
#endif

/* ****    Some useful macros (be aware of side effects because   **** */
/* ****     they treat arguments more than once).		  **** */
#ifndef MAX
# define MAX(a,b)  ((a) > (b) ? (a) :  (b))
#endif
#ifndef MIN
# define MIN(a,b)  ((b) > (a) ? (a) :  (b))
#endif
#ifndef ABS
# define ABS(x)    ((x) >= 0  ? (x) : -(x))
#endif

/* ****   I'd like to type PI if I want PI !   ***** */
#ifndef PI
# define PI         3.141592653589793238462643383279
#endif

/* *******  General definitions  *********** */
#define MAXLINE    500 /*  Maximal line length in some routines  */
#define MAXLENGTH  500 /*  Maximal string length in some routines  */

/*  Some systems define FILENAME_MAX very small (14 for example)  */
#ifndef JS_FILENAME_MAX
# if !defined(FILENAME_MAX) || (FILENAME_MAX < 256 && !defined(__MSDOS__))
#  define JS_FILENAME_MAX  256
# else
#  define JS_FILENAME_MAX  FILENAME_MAX
# endif
#endif

/* ******  Common environment variable for options and path ***** */
#define JSENVOPT     "JSOPTIONS"
#define JSENVPATH    "JSPATH"

/* ***********	 Extracted definitions of library code  ********* */

/* ----------- FILE: ErrCodes.c           ------------ */
#define _ERR_BFILETRUNC        1
#define _ERR_BFILEGET          2
#define _ERR_BFILESET          4
#define _ERR_BAYTREE           5
#define _ERR_BSORTOPEN        11

/* ----------- FILE: bigsort.c            ------------ */
#include <stdio.h>   /*	 For "FILE" definition    */

/*  Structure to access a sorted stream  */
typedef struct {
  /*  0: Memory list ("BSH_Buff"); 1: File access ("BSH_File")  */
  int     BSH_Type;
  long    BSH_Len,BSH_Count,BSH_Curr;
  union {
    FILE  *bsh_File;
    void **bsh_Buff;
  } c;
} BigSortHandle;
#define BSH_File  c.bsh_File
#define BSH_Buff  c.bsh_Buff

/*  For variable mode from bigSortOpen  */
#define BIGSORT_FDISTINCT       (1<<0) /* remove duplicate entries  */

/*  Obtain length of sorted stream  */
#define bigSortLength(bsh)      ((bsh)->BSH_Count)

/*  Seek or rewind a stream and get current position  */
#define bigSortSeek(bsh,pos)    ((bsh)->BSH_Curr = (pos))
#define bigSortRewind(bsh)      bigSortSeek(bsh,0)
#define bigSortCurrPos(bsh)     ((bsh)->BSH_Curr)

/*  Get next set in sequence (sequential access)  */
#define bigSortNext(bsh,buff)   bigSortGet(bsh,-1,buff)

/*  Same as bigSortNext, but do not advance internal pointer  */
#define bigSortPeek(bsh,buff)   bigSortGet(bsh,-2,buff)

/* ----------- FILE: btreeupdate.c        ------------ */
/*  Some abbreviations for convencience  */
#define bayTreeUpdate(bt,number,oContent,nContent,mode) \
		bayTreeUpdate2(bt,number,number,oContent,nContent,mode)
#define bayTreeInsert(bt,number,content,mode) \
		bayTreeUpdate(bt,number,NULL,content,mode)
#define bayTreeDelete(bt,number,content,mode) \
		bayTreeUpdate(bt,number,content,NULL,mode)

/*  We do not include mode, since specifying uniqueness doesn't make
 *   sense when only the number is changed. Actually, it will fail,
 *   since content already exists in the tree.
 */
#define bayTreeChSetNr(bt,oNumber,nNumber,content) \
		bayTreeUpdate2(bt,oNumber,nNumber,content,content,0)

/* ----------- FILE: bufffile.c           ------------ */
#include "jsconfig.h"  /*  For configurational definitions  */

typedef struct BuffFilePage BuffFilePage;
struct BuffFilePage {
  BuffFilePage *BFP_Next; /*  pointer to next page in linked list  */

  /*  PageLen may be smaller than the pagesize for read pages at EOF  */
  long          BFP_PageNr,BFP_PageLen;

  /*  Actual page  */
  char         *BFP_Page;

  /*  multiple protections for a region are OK  */
  short         BFP_Flags,BFP_Prot;
};
#define BFP_DIRTY     (1<<0)  /*  page is dirty  */
#define BFP_REREAD    (1<<1)  /*  re-read  */

typedef struct BuffFile BuffFile;
struct BuffFile {
  int BF_Type;
#ifndef CONFIG_NO_POSIX
  int BF_Handle;
#else
  FILE *BF_Handle;
#endif
  int BF_PageShift;
  int BF_HashLen;

  BuffFilePage **BF_HashTab;

  /*  For memory mapped I/O  */
  char *BF_MMPage;
  long BF_MMPageLen;
  short BF_MMProt;
};

/*  For routine bFileOpen  */
#define BFILE_WRITE          (1<<0)  /*  Open for r/w (default is rd-only)  */
#define BFILE_CREATE         (1<<1)  /*  Create file, deleting old contents  */
#define BFILE_MMAP           (1<<2)  /*  Use memory mapped I/O if possible  */
#define BFILE_FLOCK          (1<<3)  /*  Lock the whole file after opening  */
#define BFILE_FLOCKW         (1<<4)  /*  Same as FLOCK, but wait for others  */
#define BFILE_MULTI          (1<<5)  /*  Multiuser support when accessing  */

/* ***  For bFileGet  *** */
#define BFILEMODE_PROT       (1<<0)  /*  Prevent reusage of the page buffer  */
#define BFILEMODE_UNPROT     (1<<1)  /*  Allow reusage of the page buffer  */
#define BFILEMODE_CLEARPROT  (1<<2)  /*  Clear any protections   */
#define BFILEMODE_DIRTY      (1<<3)  /*  Mark a page dirty  */
#define BFILEMODE_REREAD     (1<<4)  /*  Force re-read even if in memory  */
/*  If the caller of bFileGet expects the returned length
 *   to be identical to passed one
 */
#define BFILEMODE_FULLMAP    (1<<5)

/*  How to get the current file length. You have to flush the file first!  */
#ifndef CONFIG_NO_POSIX
# define bFileFLength(bfp)   fdFileLength((bfp)->BF_Handle)
#else
# define bFileFLength(bfp)   fpFileLength((bfp)->BF_Handle)
#endif

/* ----------- FILE: btreefile.c          ------------ */
/*  Structure to access a Bayer tree  */
typedef struct BayerTree BayerTree;
struct BayerTree {
  /*  The underlying buffered file  */
  BuffFile *BAT_BFile;

  /*  Page shift for determination of page size and length of field  */
  int       BAT_PageShift;
  long      BAT_FieldLength;

  /*  Maximum number of elements per intermediate page and leaf page  */
  long      BAT_MaxElement,BAT_MaxElementLeaf;

  /*  Number of reserved bytes at end of every page  */
  long      BAT_Reserved;
  long      BAT_Root; /*  root page  */

  /*  Page administration routine:
   *    page == -1: Allocate new page and return page number, -1
   *                on error.
   *    page >=  0: Free the given page. Returns -1 on error.
   */
  long    (*BAT_PageAdmin)(BayerTree *bt,long page);

  /*  Field comparison routine   */
  int     (*BAT_Compare)(void *user,const void *a,const void *b);

  void     *BAT_User;
};

/* ***  Creation of a tree is done by specifying -1 for the root page  *** */
#define bayTreeCreate(bf,pLenShift,fLength,reserved,fPageAdmin,fCompare) \
	bayTreeOpen(bf,pLenShift,fLength,reserved,-1,fPageAdmin,fCompare)

/* ***  Closing a tree simply frees the allocated memory  *** */
#define bayTreeClose(bt,mode)   ((mode) ? 0 : (free(bt),0))

/* ***  Portable way to access the root page  *** */
#define bayTreeRootPage(bt)     ((bt)->BAT_Root)

/*  What error number to expect when set was not found  */
#define BTERR_SETNOTFOUND       (_ERR_BAYTREE + 1)

/* ----------- FILE: btreefind.c          ------------ */
/* **  Find the first element that matches the condition  ** */
#define bayTreeFind(bt,content) bayTreeSeek(bt,content,-1,NULL, \
					    (bt)->BAT_Compare,(bt)->BAT_User,0)

/* ***  A stack to hold the current position in the tree  *** */
typedef struct {
  long BTP_PageNr;
  long BTP_Position;
} BayTreePos;

/* ***  A handle to access an ascending/ descending sequence of entries  *** */
typedef struct {
  int         BTL_Type;
  BayerTree  *BTL_BTree;
  char       *BTL_Start,*BTL_End;  /*  start and end  */
  char       *BTL_Old; /*  if removing duplicate entries  */

  /*  User may request different comparison function and user pointer when
   *   searching. These routines might only use the first part of every field
   *   when doing the comparisons, for example.
   */
  int       (*BTL_Compare)(void *user,const void *a,const void *b);
  void       *BTL_User;

  BayTreePos *BTL_TPos;  /*  TPos and TDepth hold the current position  */
  int         BTL_TDepth;
} BayTreeHandle;

/* ***  For routine bayTreeFStart  *** */
#define BAYTREEF_INVERT    (1<<0)   /*  invert traversal  */
#define BAYTREEF_SLOWER    (1<<1)   /*  condition for start is lower  */
#define BAYTREEF_ELOWER    (1<<2)   /*  condition for end is lower  */
#define BAYTREEF_DISTINCT  (1<<3)   /*  remove multiple entries  */

/* ***  Internal flag to mark that stream is already rewound  *** */
#define BAYTREEF_ATSTART   (1<<11)

/* ***  Closing a stream currently frees only the memory  *** */
#define bayTreeFEnd(btl,mode) (free((btl)->BTL_TPos),free((btl)->BTL_Old), \
			       (mode) ? 0 :(free(btl),0))

/*  Modes for routine bayTreeUpdate: Ensure that the inserted entity is
 *   unique regarding solely its contents
 */
#define BAYTREEM_UNIQUE    (1<<0)

/* *******   Prototypes of library functions  ***** */
/*  For C++ compatibility   */
# ifdef __cplusplus
#  define Prototype extern "C"
# else
#  define Prototype extern
# endif

/* ***** File: ErrCodes.c            ***** */
Prototype const char    *JSErrors[];

/* ***** File: bigsort.c             ***** */
Prototype BigSortHandle *bigSortOpen(BigSortHandle *bsh,long len,
				     int (*get)(void *uptr,void *buff,long nr),
				     int (*compare)(void *uptr,
						    const void *a,
						    const void *b),
				     void *uPtr,int mode);
Prototype int            bigSortClose(BigSortHandle *bsh,int mode);
Prototype long           bigSortGet(BigSortHandle *bsh,long nr,void *buff);
Prototype long           bigSortFind(BigSortHandle *bsh,const void *cont,
				     int (*compare)(void *uptr,const void *a,
						    const void *b),void *uPtr,
				     void *dest);
Prototype long           BSortMaxMem;
Prototype int            BSortNSeqs;
Prototype const char    *BSortTempDir;

/* ***** File: btreebuild.c          ***** */
Prototype int            bayTreeBuild(BayerTree *bt,
				      long (*get)(void *uptr,char *buff,
						  long nr),void *uPtr,
				      double frac,int mode);

/* ***** File: btreecheck.c          ***** */
Prototype int            bayTreeCheck(BayerTree *bt,
				      int (*routine)(BayerTree *b,char *p,
						     long pagenr,long n,
						     int depth),int mode);

/* ***** File: btreefile.c           ***** */
Prototype BayerTree     *bayTreeOpen(BayerTree *bt,BuffFile *bf,int pLenShift,
				     long fLength,long reserved,long root,
				     long (*fPageAdmin)(BayerTree *bt,
							long page),
				     int (*fCompare)(void *u,
						     const void *a,
						     const void *b),
				     void *user);
Prototype int            bayTreeDestroy(BayerTree *bt,int flag);
Prototype int            bayTreeChPage(BayerTree *bt,long page,long nPage);

/* ***** File: btreefind.c           ***** */
Prototype BayTreeHandle *bayTreeFStart(BayerTree *bt,BayTreeHandle *btl,
				       char *start,char *end,
				       int (*fCompare)(void *u,
						       const void *a,
						       const void *b),
				       void *user,int mode);
Prototype long           bayTreeFNext(BayTreeHandle *btl,char *content,
				      int mode);
Prototype int            bayTreeFRewind(BayTreeHandle *btl);
Prototype long           bayTreeSeek(BayerTree *bt,const char *content,
				     long number,BayTreePos **pStack,
				     int (*fCompare)(void *u,
						     const void *a,
						     const void *b),
				     void *user,int mode);

/* ***** File: btreeupdate.c         ***** */
Prototype int            bayTreeUpdate2(BayerTree *bt,long oNumber,
					long nNumber,const char *oCont,
					const char *nCont,int mode);

/* ***** File: bufffile.c            ***** */
Prototype BuffFile      *bFileOpen(const char *name,int hashLen,int pLenShift,
				   int mode);
Prototype int            bFileClose(BuffFile *bfp);
Prototype int            bFileFlush(BuffFile *bfp,int mode);
Prototype int            bFileTruncate(BuffFile *bfp,long size);
Prototype char          *bFileGet(BuffFile *bfp,long offset,long *pLen,
				  int mode);
Prototype char          *bFileGet2(BuffFile *bfp,long offset,long len,int mode);
Prototype int            bFileSet(BuffFile *bfp,long offset,long len,int mode);
Prototype int            bFileRead(BuffFile *bfp,void *ptr,long offset,
				   long size);
Prototype int            bFileWrite(BuffFile *bfp,const void *ptr,long offset,
				    long size);
Prototype int            bFileByteSet(BuffFile *bfp,int c,long offset,
				      long size);
Prototype int            bFileByteCopy(BuffFile *bfp,long sOff,long dOff,
				       long size);
Prototype int            bFileByteMove(BuffFile *bfp,long sOff,long dOff,
				       long size);
Prototype long           bFileNLocks(BuffFile *bfp);
Prototype int            BuffFileMaxPage;

/* ***** File: jserror.c             ***** */
Prototype int            JSErrNo;

/* ***** File: jsgetopt.c            ***** */
Prototype int		 jsGetOpt(int *pargc,void *argv,const char *optstring,
				  char **poptarg);
Prototype int            jsFirstArg(char *argv[],const char *optstring);
Prototype int		 JSOptMode,JSOptIndex;

/* ***** File: jsperror.c            ***** */
Prototype void           jsperror(const char *s);

/* ***** File: jsstrerr.c            ***** */
Prototype const char    *jsStrError(int err);

# undef Prototype   /* I like to be clean  */

#endif /*  "#ifdef __JSSUBS_H__"  */
