/*JS*********************************************************************
*
*    Program : BUFFFILE
*    Language: ANSI-C + POSIX functions where available
*    Author  : Joerg Schoen
*    Purpose : Package to provide buffered access to arbitrary regions of
*              a file. The interface consists of the following routines:
*
*     bFileOpen, bFileClose:
*       Opens and closes the file for buffered access using a handle of
*       type BuffFile. Additionally the user may specify to use memory
*       mapped I/O for efficiency or that the file should be locked for
*       exclusive read/write.
*     bFileFlush:
*       Ensures that all dirty pages are written to disk.
*
*     bFileGet:
*       Gets a pointer to the specified region of the file. The returned
*       length may be smaller than the requested one if the region crosses a
*       page boundary. In that case multiple calls to bFileGet are necessary.
*       Writing is done by marking a buffer 'dirty'.
*     bFileSet:
*       Used to unprotect a region or to mark it dirty. This routine does
*       *not* load the pages, thus it works only after a single bFileGet for
*       the region or multiple bFileGet's that lock the region (to ensure
*       none of the buffers are reused).
*
*     bFileRead, bFileWrite, bFileByteSet,bFileByteCopy,bFileByteMove:
*       Reads and writes data, set and copys regions using multiple calls
*       to bFileGet.
*
*************************************************************************/

#ifndef lint
static const char rcsid[] = "$Id: bufffile.c,v 1.12 1998/02/26 17:50:45 joerg Stab joerg $";
#endif

/*********     INCLUDES                                         *********/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jsconfig.h"

#include "./jssubs.h"

#ifndef CONFIG_NO_POSIX
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <unistd.h>
#endif

/*********     DEFINES                                          *********/
#ifndef BFILEMODE_PROT
/*  START-DEFINITIONS  */
#include <stdio.h>     /*  For "FILE" definition  */
#include <jsconfig.h>  /*  For configurational definitions  */

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
/*  END-DEFINITIONS  */
#endif

#ifndef CONFIG_NO_POSIX
# define CONFIG_USE_MMAP /*  SVR4 and BSD support it  */
#else
# undef CONFIG_USE_MMAP  /*  Don't try on non-POSIX systems  */
#endif

#ifdef CONFIG_USE_MMAP
#include <sys/mman.h>
#endif

/* Using the defaults, the maximum used memory for buffering is
 *  DEFAULT_HASHLEN * DEFAULT_MAXPAGES * 2^DEFAULT_PAGESHIFT = 512KByte
 */
#ifndef BUFFFILE_DEFHASHLEN
# define BUFFFILE_DEFHASHLEN     16
#endif

#ifndef BUFFFILE_DEFMAXPAGES
/*  Maximum number of pages in a hash chain
 *   before we start to steal pages.
 */
# define BUFFFILE_DEFMAXPAGES     4
#endif

#ifndef BUFFFILE_DEFPAGESHIFT
/*  Default size of pages is 2^13 = 8KByte chunks  */
# define BUFFFILE_DEFPAGESHIFT   13
#endif

/* **  Internal macros  ** */
#define BF_PAGESIZE(bfp)           (1 << (bfp)->BF_PageShift)
#define BF_PAGEOFFSET(bfp,pageNr)  ((pageNr) << (bfp)->BF_PageShift)

#define freePage(p)  (free((p)->BFP_Page),free(p))

#define Prototype extern
/*********     PROTOTYPES                                       *********/
Prototype BuffFile      *bFileOpen(const char *name,int hashLen,int pLenShift,
				   int mode);
Prototype int            bFileClose(BuffFile *bfp);
Prototype int            bFileFlush(BuffFile *bfp,int mode);
static int               flushPage(BuffFile *bfp,BuffFilePage *p);
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

/*********     GLOBAL VARIABLES                                 *********/
int BuffFileMaxPage = BUFFFILE_DEFMAXPAGES;

/*JS*********************************************************************
*   Opens the named file for buffered I/O, using pLenShift for
*    determining the length of a page. mode specifies if the file is
*    opened for writing (BFILE_WRITE) or if memory mapped I/O should be
*    used if possible (BFILE_MMAP). File growth is not possible when
*    using memory mapped I/O.
*************************************************************************/

BuffFile *bFileOpen(const char *name,int hashLen,int pLenShift,int mode)

/************************************************************************/
{
  BuffFile *bfp;

  if((bfp = (BuffFile *)calloc(1,sizeof(*bfp))) == NULL) return(NULL);

  bfp->BF_Type = mode;

  bfp->BF_HashLen = hashLen ? hashLen : BUFFFILE_DEFHASHLEN;
  bfp->BF_PageShift = pLenShift ? pLenShift : BUFFFILE_DEFPAGESHIFT;
  bfp->BF_MMPage = NULL;

#ifndef CONFIG_NO_POSIX
  if((bfp->BF_Handle = open(name,(mode & BFILE_WRITE) ?
			    ((mode & BFILE_CREATE) ? (O_RDWR|O_CREAT|O_TRUNC) :
			     O_RDWR) : O_RDONLY,0666)) < 0)
    goto error;

  /*  If required, lock the whole file  */
  if(mode & (BFILE_FLOCK | BFILE_FLOCKW)) {
    struct flock lock;

    lock.l_type   = (mode & BFILE_WRITE) ? F_WRLCK : F_RDLCK;
    lock.l_start  = 0;
    lock.l_whence = SEEK_SET;
    lock.l_len    = 0;
    if(fcntl(bfp->BF_Handle,(mode & BFILE_FLOCKW) ? F_SETLKW : F_SETLK,
	     &lock) < 0)
      goto error;
  }

#ifdef CONFIG_USE_MMAP
  if(mode & BFILE_MMAP) {
    struct stat stbuf;

    if(fstat(bfp->BF_Handle,&stbuf)) goto error;

    if(stbuf.st_size > 0) {
      bfp->BF_MMPageLen = stbuf.st_size;

      /*  If we open file for writing, restrict memory mapped region to
       *   multiples of pages. This will ensure that additional pages
       *   that are allocated to enlarge the file start on page boundaries
       *   in the file.
       */
      if(mode & BFILE_WRITE) bfp->BF_MMPageLen &= ~(BF_PAGESIZE(bfp) - 1);

      /*  Do not try to map empty regions  */
      if(bfp->BF_MMPageLen == 0)
	bfp->BF_MMPage = NULL;
      else if((bfp->BF_MMPage = mmap(NULL,bfp->BF_MMPageLen,
				     (mode & BFILE_WRITE) ?
				     (PROT_WRITE | PROT_READ) : PROT_READ,
				     MAP_SHARED,bfp->BF_Handle,0)) ==
	      (void *)-1) goto error;
    }
  }
#endif
#else
  /*  Use ANSI-fopen function and prevent additional buffering by stdio  */
  if((bfp->BF_Handle = fopen(name,(mode & BFILE_WRITE) ?
			     ((mode & BFILE_CREATE) ? "wb+" : "rb+") : "rb"))
     == NULL ||
     setvbuf(bfp->BF_Handle,NULL,_IONBF,0))
    goto error;
#endif

  if((bfp->BF_HashTab = (BuffFilePage **)calloc(bfp->BF_HashLen,
						sizeof(*(bfp->BF_HashTab))))
     == NULL)
    goto error;

  return(bfp);
error:
#ifndef CONFIG_NO_POSIX
  if(bfp->BF_Handle >= 0) close(bfp->BF_Handle);
#ifdef CONFIG_USE_MMAP
  if(bfp->BF_MMPage) munmap(bfp->BF_MMPage,bfp->BF_MMPageLen);
#endif

#else
  if(bfp->BF_Handle) fclose(bfp->BF_Handle);
#endif

  if(bfp->BF_HashTab) free(bfp->BF_HashTab);
  free(bfp);

  return(NULL);
}

/*JS*********************************************************************
*   Closes the buffered file, flushing all data.
*************************************************************************/

int bFileClose(BuffFile *bfp)

/************************************************************************/
{
  int i,ret;

  ret = 0;

  if(bFileFlush(bfp,0) < 0) ret = -1;

  for(i = 0 ; i < bfp->BF_HashLen ; i++) {
    BuffFilePage *p,*n;

    for(p = bfp->BF_HashTab[i] ; p ; p = n) {
      n = p->BFP_Next;
      freePage(p);
    }
  }

  free(bfp->BF_HashTab);

#ifdef CONFIG_USE_MMAP
  if(bfp->BF_MMPage && munmap(bfp->BF_MMPage,bfp->BF_MMPageLen) < 0) ret = -1;
#endif

  if(
#ifndef CONFIG_NO_POSIX
    close(bfp->BF_Handle) < 0
#else
    fclose(bfp->BF_Handle)
#endif
    ) ret = -1;

  free(bfp);

  return(ret);
}

/*JS*********************************************************************
*   Flushes all dirty buffers to disk. If Bit 0 of mode is set, buffers
*    are freed if they exceed the maximum number of buffers. If Bit 1 of
*    mode is set, a memory mapped region is extended if necessary to cover
*    the whole file. If Bit 2 of mode is set, the dirty flag is not
*    deleted when a page is flushed.
*************************************************************************/

int bFileFlush(BuffFile *bfp,int mode)

/************************************************************************/
{
  int i;

  /*  Flush all pages  */
  for(i = 0 ; i < bfp->BF_HashLen ; i++) {
    BuffFilePage *p;
    int count,countUL;

    /*  Flush all pages, count whole and unlocked ones  */
    for(count = countUL = 0, p = bfp->BF_HashTab[i] ; p ;
	p = p->BFP_Next, count++) {
      if(p->BFP_Flags & BFP_DIRTY) {
	if(flushPage(bfp,p)) goto error;
	if(!(mode & 4)) p->BFP_Flags &= ~BFP_DIRTY;
      }
      if(p->BFP_Prot == 0) countUL++;
    }

    /*  Free pages?  */
    if((mode & 3) && count > BuffFileMaxPage && countUL > 0) {
      BuffFilePage *previous;

      /*  Start freeing pages (count >= 0) at the end of the list  */
      count -= BuffFileMaxPage + countUL;
      for(p = bfp->BF_HashTab[i], previous = NULL ; p ; count++)
	if(count >= 0 && p->BFP_Prot == 0) {
	  BuffFilePage *next = p->BFP_Next;

	  if(previous) {
	    previous->BFP_Next = next;
	  } else {
	    bfp->BF_HashTab[i] = next;
	  }
	  freePage(p);
	  p = next; /*  previous doesn't change  */
	} else {
	  previous = p;
	  p = p->BFP_Next;
	}
    }
  }

#ifdef CONFIG_USE_MMAP
  /*  Try to extend memory mapped region only if file is opened for r/w  */
  if((mode & 2) && (bfp->BF_Type & (BFILE_MMAP|BFILE_WRITE)) ==
     (BFILE_MMAP|BFILE_WRITE) && bfp->BF_MMProt == 0) {
    struct stat stbuf;
    long size;

    /*  Check if some pages are still locked. Otherwise we
     *   might have the memory mapped region and a page
     *   pointing to the same file location.
     */
    for(i = 0 ; i < bfp->BF_HashLen ; i++) {
      BuffFilePage *p;

      for(p = bfp->BF_HashTab[i] ; p ; p = p->BFP_Next) {
	if(p->BFP_Prot) goto ende;
	p->BFP_PageNr = -1; /*  mark this page unused  */
      }
    }

    if(fstat(bfp->BF_Handle,&stbuf)) goto error;

    /*  Restrict memory mapped region to multiples of pages  */
    size = stbuf.st_size & ~(BF_PAGESIZE(bfp) - 1);

    /*  Check if file size has changed  */
    if(size != bfp->BF_MMPageLen) {
      if(bfp->BF_MMPage && munmap(bfp->BF_MMPage,bfp->BF_MMPageLen) < 0)
	goto error;

      if((bfp->BF_MMPageLen = size) == 0)
	bfp->BF_MMPage = NULL;
      else if((bfp->BF_MMPage = mmap(NULL,bfp->BF_MMPageLen,
				     (PROT_WRITE | PROT_READ),MAP_SHARED,
				     bfp->BF_Handle,0)) == (void *)-1)
	goto error;
    }
  }
ende:

# if defined(__linux__)
  if((bfp->BF_Type & (BFILE_MMAP|BFILE_WRITE)) == (BFILE_MMAP|BFILE_WRITE)) {
    /*  Schedule memory mapped region for write  */
    if(msync(bfp->BF_MMPage,bfp->BF_MMPageLen,MS_ASYNC) < 0) goto error;
  }
# endif
#endif

  /*  Flush all data to disk  */
#ifndef CONFIG_NO_POSIX
#if 0  /*  Prevent to much I/O  */
  if(fsync(bfp->BF_Handle) < 0) goto error;
#endif
#else
  if(fflush(bfp->BF_Handle) == EOF) goto error;
#endif

  return(0);
error:
  return(-1);
}

/*JS*********************************************************************
*   Internal routine to flush contents of a page.
*************************************************************************/

static int flushPage(BuffFile *bfp,BuffFilePage *p)

/************************************************************************/
{
  if(
#ifndef CONFIG_NO_POSIX
     lseek(bfp->BF_Handle,BF_PAGEOFFSET(bfp,p->BFP_PageNr),SEEK_SET) < 0 ||
     write(bfp->BF_Handle,p->BFP_Page,p->BFP_PageLen) != p->BFP_PageLen
#else
     fseek(bfp->BF_Handle,BF_PAGEOFFSET(bfp,p->BFP_PageNr),SEEK_SET) ||
     fwrite(p->BFP_Page,1,p->BFP_PageLen,bfp->BF_Handle) != p->BFP_PageLen
#endif
     ) return(-1);

  return(0);
}

/* ERROR-DEFINITIONS from bFileTruncate label _ERR_BFILETRUNC ord 16
   Locked pages beyond file size found
*/

/*JS*********************************************************************
*
*************************************************************************/

int bFileTruncate(BuffFile *bfp,long size)

/************************************************************************/
{
  int i;

  for(i = 0 ; i < bfp->BF_HashLen ; i++) {
    BuffFilePage *p,*prev,*next;

    for(prev = NULL, p = bfp->BF_HashTab[i] ; p ; p = next) {
      long offset;

      next = p->BFP_Next;
      offset = BF_PAGEOFFSET(bfp,p->BFP_PageNr);
      if((offset + p->BFP_PageLen) > size) {
	/*  Cannot truncate if exceeding page is still locked.
	 *   We have a problem here: Counting locks doesn't
	 *   provide enough information to decide if the
	 *   truncation violates a locked region. We thus
	 *   must trust the user who does the truncation.
	 *   We only check for the obvious case where the
	 *   start of the page lays already behind the new
	 *   size.
	 */
	if(offset > size && p->BFP_Prot) {
	  JSErrNo = _ERR_BFILETRUNC + 0;
	  goto error;
	}

	/*  Truncate or free this page  */
	if(offset < size) {
	  p->BFP_PageLen = size - offset;
	} else {
	  /*  Remove page from linked list  */
	  if(prev)
	    prev->BFP_Next = next;
	  else
	    bfp->BF_HashTab[i] = next;
	  freePage(p);
	  continue; /*  prev doesn' change  */
	}
      }
      prev = p;
    }
  }

#ifndef CONFIG_NO_POSIX
  if(ftruncate(bfp->BF_Handle,size) < 0) goto error;
#endif

  return(0);
error:
  return(-1);
}

/* ERROR-DEFINITIONS from bFileGet label _ERR_BFILEGET ord 16
   Cannot map complete region
   End of file
*/

/*JS*********************************************************************
*   Returns the start pointer for the specified region
*    [offset,offset + *pLen] in the file. The returned length in *pLen
*    may be shorter, thus requiring additional calls to bFileGet.
*    Properties for the region may be set immediately using mode:
*    'BFILEMODE_PROT' prevents reusage, 'BFILEMODE_UNPROT' allows.
*    'BFILEMODE_DIRTY' marks that the region must be written to disk.
*    'BFILEMMODE_REREAD' forces re-reading the page.
*************************************************************************/

char *bFileGet(BuffFile *bfp,long offset,long *pLen,int mode)

/************************************************************************/
{
  BuffFilePage *p,*lastUnlocked,*previous;
  long len,pageNr;
  int hash,count,flag;

  len = *pLen;

#ifdef CONFIG_USE_MMAP
  if(bfp->BF_MMPage) {
    if(len > 0 ? offset < bfp->BF_MMPageLen : offset <= bfp->BF_MMPageLen) {
      /*  Check if boundary of memory mapped region is exceeded  */
      if(len > 0) {
	if((offset + len) > bfp->BF_MMPageLen) {
	  if(mode & BFILEMODE_FULLMAP) goto maperror;
	  *pLen = bfp->BF_MMPageLen - offset;
	}
      } else {
	if((offset + len) < 0) {
	  if(mode & BFILEMODE_FULLMAP) goto maperror;
	  *pLen = -offset;
	}
      }

      if(mode & BFILEMODE_PROT) bfp->BF_MMProt++;
      if(mode & BFILEMODE_UNPROT) bfp->BF_MMProt--;
      /*  Clearing lock takes precedence  */
      if(mode & BFILEMODE_CLEARPROT) bfp->BF_MMProt = 0;

      return(bfp->BF_MMPage + offset);
    } else if(!(bfp->BF_Type & BFILE_WRITE)) {
      /*  Trying to enlarge file in case of read-only
       *   access doesn't make sense.
       */
      JSErrNo = _ERR_BFILEGET + 1;
      goto error;
    }
  }
#endif

  if(len > 0) {
    pageNr = offset >> bfp->BF_PageShift;
    offset &= BF_PAGESIZE(bfp) - 1;
  } else {
    offset--;
    pageNr = offset >> bfp->BF_PageShift;
    offset &= BF_PAGESIZE(bfp) - 1;
    offset++;
  }
  hash = pageNr % bfp->BF_HashLen;

  flag = 0;
  lastUnlocked = NULL;
  for(count = 0, p = bfp->BF_HashTab[hash], previous = NULL ; p ;
      previous = p, p = p->BFP_Next, count++) {
    if(p->BFP_Prot == 0) {
      lastUnlocked = previous;
      flag |= 1; /*  page found  */
    }

    if(p->BFP_PageNr == pageNr) break;
  }

  if(p == NULL) {
    if(count < BuffFileMaxPage || !(flag & 1)) {
      /*  Allocate new page. We have not included the page in the structure
       *   itself to be sure it is correctly aligned.
       */
      if((p = (BuffFilePage *)malloc(sizeof(*p))) == NULL)
	goto error;

      if((p->BFP_Page = (char *)malloc(BF_PAGESIZE(bfp))) == NULL) {
	free(p);
	goto error;
      }

      p->BFP_PageNr = -1;
      p->BFP_Prot = 0;

      /*  Hang in front of list  */
      p->BFP_Next = bfp->BF_HashTab[hash];
      bfp->BF_HashTab[hash] = p;
    } else {
      /*  Reuse page and hang in front of list  */
      if(lastUnlocked) {
	p = lastUnlocked->BFP_Next;
	lastUnlocked->BFP_Next = p->BFP_Next;

	p->BFP_Next = bfp->BF_HashTab[hash];
	bfp->BF_HashTab[hash] = p;
      } else {
	/*  Page is already at head  */
	p = bfp->BF_HashTab[hash];
      }

      if((p->BFP_Flags & BFP_DIRTY) && flushPage(bfp,p)) goto error;
    }
    /*  Load page  */
    p->BFP_Flags = 0;
    p->BFP_PageNr = pageNr;

    flag |= 2;
  } else if((p->BFP_Flags & BFP_REREAD) || (mode & BFILEMODE_REREAD)) {
    flag |= 2;
    p->BFP_Flags &= ~BFP_REREAD;
  }

  /*  Read page?  */
  if(flag & 2) {
    if(
#ifndef CONFIG_NO_POSIX
       lseek(bfp->BF_Handle,BF_PAGEOFFSET(bfp,p->BFP_PageNr),SEEK_SET) < 0 ||
       (p->BFP_PageLen = read(bfp->BF_Handle,p->BFP_Page,BF_PAGESIZE(bfp)))
#else
       fseek(bfp->BF_Handle,BF_PAGEOFFSET(bfp,p->BFP_PageNr),SEEK_SET) < 0 ||
       (p->BFP_PageLen = fread(p->BFP_Page,1,BF_PAGESIZE(bfp),bfp->BF_Handle))
#endif
       < 0)
      goto error;
  }

  if(len > 0) {
    if((offset + len) > p->BFP_PageLen) {
      if(mode & BFILEMODE_DIRTY) {
	/*  User requested writing, so increase area  */
	p->BFP_PageLen = MIN(BF_PAGESIZE(bfp),offset + len);

	if((offset + len) > p->BFP_PageLen) {
	  if(mode & BFILEMODE_FULLMAP) goto maperror;
	  *pLen = p->BFP_PageLen - offset;
	}
      } else {
	/*  Check first for EOF before assuming map error  */
	if((*pLen = p->BFP_PageLen - offset) < 0) {
	  JSErrNo = _ERR_BFILEGET + 1;
	  goto error;
	}
	if(mode & BFILEMODE_FULLMAP) goto maperror;
      }
    }
  } else {
    if(offset > p->BFP_PageLen) {
      if(mode & BFILEMODE_DIRTY) {
	/*  User requested writing, so increase area  */
	p->BFP_PageLen = MIN(BF_PAGESIZE(bfp),offset);

	if((offset + len) < 0) {
	  if(mode & BFILEMODE_FULLMAP) goto maperror;
	  *pLen = -offset;
	}
      } else {
	/*  Doesn't work for negative lengths  */
	JSErrNo = _ERR_BFILEGET + 1;
	goto error;
      }
    } else if((offset + len) < 0) {
      if(mode & BFILEMODE_FULLMAP) goto maperror;
      *pLen = -offset;
    }
  }

  if(mode & BFILEMODE_DIRTY) p->BFP_Flags |= BFP_DIRTY;
  if(mode & BFILEMODE_PROT) p->BFP_Prot++;
  if(mode & BFILEMODE_UNPROT) p->BFP_Prot--;
  /*  Clearing lock takes precedence  */
  if(mode & BFILEMODE_CLEARPROT) p->BFP_Prot = 0;

  return(p->BFP_Page + offset);
maperror:
  JSErrNo = _ERR_BFILEGET + 0;
error:
  return(NULL);
}

/*JS*********************************************************************
*   Simple interface to bFileGet to avoid pointer argument if one expects
*    the whole region to be mapped.
*************************************************************************/

char *bFileGet2(BuffFile *bfp,long offset,long len,int mode)

/************************************************************************/
{
  return(bFileGet(bfp,offset,&len,mode));
}

/* ERROR-DEFINITIONS from bFileGet label _ERR_BFILESET ord 16
   Page not in memory
*/

/*JS*********************************************************************
*   Sets the specified property of the region: 'BFILEMODE_PROT' prevents
*    reusage, 'BFILEMODE_UNPROT' allows. 'BFILEMODE_DIRTY' marks that the
*    region must be written to disk.
*************************************************************************/

int bFileSet(BuffFile *bfp,long offset,long len,int mode)

/************************************************************************/
{
  if(len < 0) {
    /*  If len is negative, simply do it the other way round  */
    offset += len;
    len = -len;
  }

  while(len > 0) {
    BuffFilePage *p;
    long pageNr,pOffset,len2;

#ifdef CONFIG_USE_MMAP
    if(bfp->BF_MMPage && offset < bfp->BF_MMPageLen) {
      /*  Check if boundary of memory mapped region is exceeded  */
      len2 = MIN(len,bfp->BF_MMPageLen - offset);

      if(mode & BFILEMODE_PROT) bfp->BF_MMProt++;
      if(mode & BFILEMODE_UNPROT) bfp->BF_MMProt--;
      /*  Clearing lock takes precedence  */
      if(mode & BFILEMODE_CLEARPROT) bfp->BF_MMProt = 0;
    } else
#endif
    {
      pageNr = offset >> bfp->BF_PageShift;
      pOffset = offset & (BF_PAGESIZE(bfp) - 1);

      for(p = bfp->BF_HashTab[pageNr % bfp->BF_HashLen] ; p ; p = p->BFP_Next)
	if(p->BFP_PageNr == pageNr) break;

      if(p == NULL) {
	JSErrNo = _ERR_BFILESET + 0;
	return(-1);
      }

      if(mode & BFILEMODE_DIRTY) p->BFP_Flags |= BFP_DIRTY;
      if(mode & BFILEMODE_PROT) p->BFP_Prot++;
      if(mode & BFILEMODE_UNPROT) p->BFP_Prot--;
      /*  Clearing lock takes precedence  */
      if(mode & BFILEMODE_CLEARPROT) p->BFP_Prot = 0;

      len2 = MIN(len,BF_PAGESIZE(bfp) - pOffset);
    }
    offset += len2;
    len -= len2;
  }

  return(0);
}

/*JS*********************************************************************
*   Read the specified length into the buffer. The returned length may be
*    shorter if the end of file is reached.
*************************************************************************/

int bFileRead(BuffFile *bfp,void *ptr,long offset,long size)

/************************************************************************/
{
  long toRead,l;

  for(toRead = size ; toRead > 0 ; toRead -= l) {
    char *adr;

    /*  Try to get as much as possible  */
    l = toRead;
    if((adr = bFileGet(bfp,offset,&l,0)) == NULL) return(-1);

    /*  EOF reached?  */
    if(l == 0) break;

    memcpy(ptr,adr,l);

    offset += l;
    ptr = (void *)((char *)ptr + l);
  }

  return(size - toRead);
}

/*JS*********************************************************************
*   Write the specified length from the buffer to the file. The returned
*    length may be shorter if an error occured.
*************************************************************************/

int bFileWrite(BuffFile *bfp,const void *ptr,long offset,long size)

/************************************************************************/
{
  long toWrite,l;

  for(toWrite = size ; toWrite > 0 ; toWrite -= l) {
    char *adr;

    l = toWrite;

    if((adr = bFileGet(bfp,offset,&l,BFILEMODE_DIRTY)) == NULL) return(-1);

    memcpy(adr,ptr,l);

    offset += l;
    ptr = (void *)((char *)ptr + l);
  }

  return(size - toWrite);
}

/*JS*********************************************************************
*   Set the region from offset to offset+size to the byte c.
*************************************************************************/

int bFileByteSet(BuffFile *bfp,int c,long offset,long size)

/************************************************************************/
{
  long toSet,l;

  for(toSet = size ; toSet > 0 ; toSet -= l) {
    char *adr;

    l = toSet;
    if((adr = bFileGet(bfp,offset,&l,BFILEMODE_DIRTY)) == NULL) return(-1);

    memset(adr,c,l);
    offset += l;
  }

  return(size - toSet);
}

/*JS*********************************************************************
*   Copys size bytes from offset sOff to dOff. Doesn't work if the areas
*    overlap.
*************************************************************************/

int bFileByteCopy(BuffFile *bfp,long sOff,long dOff,long size)

/************************************************************************/
{
  long toCopy,l,l2;

  for(toCopy = size ; toCopy > 0 ; toCopy -= l2) {
    char *s,*d;

    l = l2 = toCopy;
    if((s = bFileGet(bfp,sOff,&l,BFILEMODE_PROT)) == NULL ||
       (d = bFileGet(bfp,dOff,&l2,BFILEMODE_DIRTY)) == NULL)
      return(-1);

    if(l < l2) l2 = l;
    memcpy(d,s,l2);
    if(bFileSet(bfp,sOff,l,BFILEMODE_UNPROT) < 0) return(-1);

    sOff += l2;
    dOff += l2;
  }

  return(size - toCopy);
}

/*JS*********************************************************************
*   Moves size bytes from offset sOff to dOff. Works also if the areas
*    overlap.
*************************************************************************/

int bFileByteMove(BuffFile *bfp,long sOff,long dOff,long size)

/************************************************************************/
{
  long toCopy,l,l2;

  /*  If the destination offset is greater than the source
   *   offset, do it backwards in case areas overlap.
   */
  if(sOff < dOff) {
    sOff += size;
    dOff += size;
    toCopy = -size;
  } else
    toCopy = size;

  for( ; toCopy != 0 ; toCopy -= l2) {
    char *s,*d;

    l = l2 = toCopy;
    if((s = bFileGet(bfp,sOff,&l,BFILEMODE_PROT)) == NULL ||
       (d = bFileGet(bfp,dOff,&l2,BFILEMODE_DIRTY)) == NULL) return(-1);

    if(toCopy > 0) {
      if(l < l2) l2 = l;
      memmove(d,s,l2);
    } else {
      if(l > l2) l2 = l;
      memmove(d + l2,s + l2,-l2);
    }
    if(bFileSet(bfp,sOff,l,BFILEMODE_UNPROT) < 0) return(-1);

    sOff += l2;
    dOff += l2;
  }

  return(size - ABS(toCopy));
}

/*JS*********************************************************************
*   Count total number of locks currently present on buffered file.
*************************************************************************/

long bFileNLocks(BuffFile *bfp)

/************************************************************************/
{
  int i;
  long count;

  count = bfp->BF_MMProt;
  for(i = 0 ; i < bfp->BF_HashLen ; i++) {
    BuffFilePage *p;

    for(p = bfp->BF_HashTab[i] ; p ; p = p->BFP_Next)
      count += p->BFP_Prot;
  }

  return(count);
}
