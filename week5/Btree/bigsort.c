/*JS*********************************************************************
*
*    Program : BIGSORT
*    Language: ANSI-C
*    Author  : Joerg Schoen
*    Purpose : Sort huge amounts of data by doing a multi-phase
*              sorting on temporary files. For a description of the
*              algorithm see Section 2.4.4 in
*                Niklaus Wirth: "Algorithmen und Datenstrukturen mit
*                                Modula-2"
*
*  This implementation automatically recognizes if the input data fits
*   completely into memory and then does a in-place sort via quicksort.
*
*************************************************************************/

#ifndef lint
static const char rcsid[] = "$Id: bigsort.c,v 1.12 1998/04/01 17:07:48 joerg Stab joerg $";
#endif

/*********     INCLUDE                                         *********/
#include <jsalloca.h>

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <jsconfig.h>

#include <jssubs.h>

#ifndef CONFIG_NO_POSIX
# include <unistd.h>
#endif

/*#define DEBUG*/

/*********     DEFINES                                          *********/
#ifndef BSH_File
/*  START-DEFINITIONS  */
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
/*  END-DEFINITIONS  */
#endif

/*  Align fields for comparison routine  */
/*#define CONFIG_BIGSORT_ALIGN*/

/*  Maximum allocated memory  */
#ifndef BIGSORT_MAXMEM
# define BIGSORT_MAXMEM   500000
#endif

/*  Internal structures  */
typedef struct {
  FILE *SEQ_File;
  char *SEQ_Buffer;
  long  SEQ_Count;
  int   SEQ_NRuns,SEQ_DiffRuns;
} Sequence;

typedef struct {
  long      BSI_Len;
  void     *BSI_UPtr;
  int     (*BSI_Compare)(void *uptr,const void *a,const void *b);

  long      BSI_Count;
  Sequence *BSI_Seqs;
  int       BSI_NSeqs;

  int       BSI_Level;
  int       BSI_Curr;  /*  current output sequence  */
  void    **BSI_Temp;
} BigSort;

#define Prototype extern
/*********     PROTOTYPES                                       *********/
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

static int               distRuns(BigSort *pbs,int n,
				  int (*get)(void *uptr,void *buff,long nr),
				  int mode);
static void              bsSift(void **array,int (*compare)(void *uptr,
							    const void *a,
							    const void *b),
				void *uPtr,long l,long r);
static void              sortSpecial(void **array,long n,
				     int (*compare)(void *uptr,const void *a,
						    const void *b),
				     void *uPtr);
static int               selectRun(BigSort *pbs);

static int               mergeRuns(BigSort *pbs,int mode);

static FILE             *bsTmpFile(void);

Prototype long           BSortMaxMem;
Prototype int            BSortNSeqs;
Prototype const char    *BSortTempDir;

/*********     GLOBAL VARIABLES                                 *********/
long BSortMaxMem = BIGSORT_MAXMEM;
int BSortNSeqs = 32; /*  at least 3  */
const char *BSortTempDir;

/* ERROR-DEFINITIONS from bigSortOpen label _ERR_BSORTOPEN ord 18
   BSortMaxMem to small
*/

/*JS*********************************************************************
*   Reads a stream until EOF is signaled and sorts the units of size len
*    using the given comparison function. The routine get must return 0
*    on success, -1 on error and 1 on EOF. A handle is returned which
*    allows direct or sequential access to the sorted units. If no
*    comparison routine is given (compare == NULL), the stream is simply
*    transferred to a direct-access one.
*************************************************************************/

BigSortHandle *bigSortOpen(BigSortHandle *bsh,long len,
			   int (*get)(void *uptr,void *buff,long nr),
			   int (*compare)(void *uptr,const void *a,
					  const void *b),void *uPtr,int mode)

/************************************************************************/
{
  long i,j;
  BigSort bs;
  int nSeqs;

  /*  Set up internal structure  */
  bs.BSI_Len     = len;
  bs.BSI_UPtr    = uPtr;
  bs.BSI_Compare = compare;
  bs.BSI_NSeqs   = nSeqs = BSortNSeqs;
  bs.BSI_Level   = 0;
  bs.BSI_Curr    = 0;
  bs.BSI_Count   = 0;
  bs.BSI_Temp    = NULL;

  /* *** 1. Distribute elements  *** */
  /*  How much elements may we store in memory (at least 2)?  */
  if((i = (BSortMaxMem - nSeqs * (len + sizeof(void *))) /
      (len + sizeof(void *))) < 2) {
    JSErrNo = _ERR_BSORTOPEN + 0;
    goto error2;
  }

  if((j = distRuns(&bs,i,get,mode)) < 0) goto error2;

  if(j > 0) {
    /* ***  Simple case: All elements fit in memory, so stop  *** */
    if(bsh == NULL && (bsh = (BigSortHandle *)malloc(sizeof(*bsh))) == NULL)
      goto error2;

    bsh->BSH_Type  = 0;
    bsh->BSH_Len   = len;
    bsh->BSH_Curr  = 0;
    bsh->BSH_Count = bs.BSI_Count;
    bsh->BSH_Buff  = bs.BSI_Temp;
    return(bsh);
  }

  /* *** 2. Mix sequences  *** */
  if(compare) {
    if(mergeRuns(&bs,mode) < 0) goto error;

#ifdef DEBUG
    if(bs.BSI_Seqs[0].SEQ_Count != bs.BSI_Count)
      fprintf(stderr,"ERROR: Different count %ld != %ld!\n",
	      bs.BSI_Seqs[0].SEQ_Count,bs.BSI_Count);
#endif
  }

  /* ***  Finished, set up return value  *** */
  if(bsh == NULL && (bsh = (BigSortHandle *)malloc(sizeof(*bsh))) == NULL)
    goto error;

  bsh->BSH_Type  = 1;
  bsh->BSH_Len   = len;
  bsh->BSH_Curr  = 0;
  bsh->BSH_Count = bs.BSI_Count;
  bsh->BSH_File  = bs.BSI_Seqs[0].SEQ_File;

  /*  Close everything and clean up  */
#ifdef CONFIG_BIGSORT_ALIGN
  free(bs.BSI_Seqs[0].SEQ_Buffer);
#endif
  for(i = 1 ; i < bs.BSI_NSeqs ; i++) {
    if(bs.BSI_Seqs[i].SEQ_File && fclose(bs.BSI_Seqs[i].SEQ_File)) goto error;
#ifdef CONFIG_BIGSORT_ALIGN
    free(bs.BSI_Seqs[i].SEQ_Buffer);
#endif
  }
  free(bs.BSI_Seqs);

#ifndef CONFIG_BIGSORT_ALIGN
  /*  Now its time to free temporary work space from distRuns  */
  free(bs.BSI_Temp);
#endif

  return(bsh);
error:
  if(bs.BSI_Seqs) {
#ifdef CONFIG_BIGSORT_ALIGN
    free(bs.BSI_Seqs[0].SEQ_Buffer);
#endif
    for(i = 1 ; i < bs.BSI_NSeqs ; i++) {
      if(bs.BSI_Seqs[i].SEQ_File && fclose(bs.BSI_Seqs[i].SEQ_File))
	goto error;
#ifdef CONFIG_BIGSORT_ALIGN
      free(bs.BSI_Seqs[i].SEQ_Buffer);
#endif
    }
    free(bs.BSI_Seqs);
  }

#ifndef CONFIG_BIGSORT_ALIGN
  free(bs.BSI_Temp);
#endif

error2:
  return(NULL);
}

/*JS*********************************************************************
*   Closes the handle obtained by bigSortOpen. If mode is non-zero, bsh
*    itself is not deallocated.
*************************************************************************/

int bigSortClose(BigSortHandle *bsh,int mode)

/************************************************************************/
{
  if(bsh->BSH_Type == 0) {
#ifdef CONFIG_BIGSORT_ALIGN
    long i;

    for(i = 0 ; i < bsh->BSH_Count ; i++) free(bsh->BSH_Buff[i]);
#endif

    free(bsh->BSH_Buff);
  } else if(fclose(bsh->BSH_File)) {
    if(!mode) free(bsh);
    return(-1);
  }

  if(!mode) free(bsh);

  return(0);
}

/*JS*********************************************************************
*   Provides direct access to the sorted stream. Read set nr (0 .. n-1)
*    into buff and return ordinal of it. On error -1 is returned and -2
*    on EOF. If nr == -1, the next set in the sequence is returned
*    (sequential access). nr == -2 is similar to nr == -1, but the
*    current position is not moved.
*************************************************************************/

long bigSortGet(BigSortHandle *bsh,long nr,void *buff)

/************************************************************************/
{
  long number,len;

  /*  Current position  */
  number = (nr >= 0) ? nr : bsh->BSH_Curr;

  /*  Check for EOF  */
  if(number >= bsh->BSH_Count) return(-2);

  len = bsh->BSH_Len;

  if(bsh->BSH_Type == 0) {
    /*  Memory list  */
    memcpy(buff,bsh->BSH_Buff[number],len);
  } else if((nr >= 0 && fseek(bsh->BSH_File,number * len,SEEK_SET)) ||
	    fread(buff,len,1,bsh->BSH_File) != 1 ||
	    /*  Go back to current position if required  */
	    (nr == -2 && fseek(bsh->BSH_File,number * len,SEEK_SET)))
    return(-1);

  if(nr != -2) bsh->BSH_Curr = number + 1;

  return(number);
}

/*JS*********************************************************************
*   Provides direct access to the stream, allowing to search for a
*    specific set.
*************************************************************************/

long bigSortFind(BigSortHandle *bsh,const void *cont,
		 int (*compare)(void *uptr,const void *a,const void *b),
		 void *uPtr,void *dest)

/************************************************************************/
{
  long l,r;

  l = 0;
  r = bsh->BSH_Count;

  if(bsh->BSH_Type == 1) {
    void *temp;

    if((temp = malloc(bsh->BSH_Len)) == NULL) return(-1);

    /* **  Binary search in file  ** */
    while(l < r) {
      long m = (l + r) / 2;

      if(fseek(bsh->BSH_File,m * bsh->BSH_Len,SEEK_SET) ||
	 fread(temp,bsh->BSH_Len,1,bsh->BSH_File) != 1) {
	free(temp);
	return(-1);
      }

      if((*compare)(uPtr,temp,cont) < 0)
	l = m + 1;
      else
	r = m;
    }

    free(temp);

    if(dest && (fseek(bsh->BSH_File,l * bsh->BSH_Len,SEEK_SET) ||
		fread(dest,bsh->BSH_Len,1,bsh->BSH_File) != 1))
      return(-1);

    return(l);
  }

  /* **  Binary search in memory list  ** */
  while(l < r) {
    long m = (l + r) / 2;

    if((*compare)(uPtr,bsh->BSH_Buff[m],cont) < 0)
      l = m + 1;
    else
      r = m;
  }

  if(dest) memcpy(dest,bsh->BSH_Buff[l],bsh->BSH_Len);

  return(l);
}

/*JS*********************************************************************
*   Distribute initial data to the sequences, sorting chunks of a fixed
*    size using a modified heap sort.
*************************************************************************/

static int distRuns(BigSort *pbs,int n,
		    int (*get)(void *uptr,void *buff,long nr),int mode)

/************************************************************************/
{
  void **a,*tmp;
  long i,len,s,r;

  len = pbs->BSI_Len;

  /*  Set up temporary array  */
  if((a = (void **)
#ifdef CONFIG_BIGSORT_ALIGN
      malloc((n + 1) * sizeof(*a))
#else
      malloc((n + 1) * (sizeof(*a) + len) + sizeof(*a))
#endif
      ) == NULL) goto error;

#ifndef CONFIG_BIGSORT_ALIGN
  /*  Hack: Assuming most strict alignment for type "double" and
   *   sizeof(double) == 2 * sizeof(void *), the following
   *   calculation makes sure that fields are correctly aligned.
   */
  a[0] = (void *)&a[(n + 2) & ~1];
#endif

  /* ***  To recognize small amounts of data more quickly,
   * ***   try to read first as much data as possible.
   */
  for(i = 0 ; i < n ; i++) {
    int ret;

#ifdef CONFIG_BIGSORT_ALIGN
    if((a[i] = (void *)malloc(len)) == NULL) goto error;
#else
    if(i) a[i] = (char *)(a[i - 1]) + len;
#endif

    if((ret = (*get)(pbs->BSI_UPtr,a[i],pbs->BSI_Count)) < 0) goto error;

    /*  EOF?  */
    if(ret > 0) break;

    pbs->BSI_Count++;
  }

  if(i < n) {
#ifndef CONFIG_BIGSORT_ALIGN
    char *from,*to;
    ptrdiff_t diff;
#endif

    /*  Special case: Everything in memory. Sort a[0 .. i-1]
     *   if required and return
     */
#ifdef CONFIG_BIGSORT_ALIGN
    free(a[i]);
#endif

    if(pbs->BSI_Compare) {
      sortSpecial(a,i,pbs->BSI_Compare,pbs->BSI_UPtr);

      if((mode & BIGSORT_FDISTINCT) && i > 1) {
	/*  Remove duplicate entries  */
	for(s = 0, r = 1 ; r < i ; r++) {
	  if((*(pbs->BSI_Compare))(pbs->BSI_UPtr,a[s],a[r]) == 0) {
#ifdef CONFIG_BIGSORT_ALIGN
	    free(a[r]);
#endif
	    continue;
	  }

	  s++;
	  if(s < r)
#ifdef CONFIG_BIGSORT_ALIGN
	    a[s] = a[r];
#else
	    memcpy(a[s],a[r],len);
#endif
	}

	pbs->BSI_Count = ++s;

	/*  In case we did not align, we cannot reduce memory by setting
	 *   i = s, since the sorting routine shuffled everything around.
	 */
#ifdef CONFIG_BIGSORT_ALIGN
	i = s;
#endif
      }
    }

#ifndef CONFIG_BIGSORT_ALIGN
    from = (char *)&a[(n + 2) & ~1];
    to   = (char *)&a[(i + 2) & ~1];
    if(to != from) memmove(to,from,i * len);

    /*  Minimize memory requirement  */
    if((pbs->BSI_Temp = (void **)realloc(a,(i + 1) * sizeof(*a) + i * len +
					 sizeof(*a))) == NULL) goto error;

    /*  We do not know if realloc to a smaller size may change the pointer  */
    diff = ((char *)pbs->BSI_Temp - (char *)a) + (to - from);
    if(diff != 0)
      while(--i >= 0) pbs->BSI_Temp[i] = (char *)(pbs->BSI_Temp[i]) + diff;
#else
    if((pbs->BSI_Temp = (void **)realloc(a,i * sizeof(*a))) == NULL)
      goto error;
#endif

    return(1);
  }

  if(pbs->BSI_Compare == NULL) {
    int ret;

    /*  If no comparison routine given, simply copy everything to
     *   one stream and return
     */
    pbs->BSI_NSeqs = 1;
    if((pbs->BSI_Seqs = (Sequence *)malloc(sizeof(*(pbs->BSI_Seqs)))) == NULL ||
       (pbs->BSI_Seqs[0].SEQ_File = bsTmpFile()) == NULL)
      goto error;

    for(i = 0 ; i < n ; i++)
      if(fwrite(a[i],len,1,pbs->BSI_Seqs[0].SEQ_File) != 1) goto error;

    while((ret = (*get)(pbs->BSI_UPtr,a[0],pbs->BSI_Count)) == 0) {
      (pbs->BSI_Count)++;

      if(fwrite(a[0],len,1,pbs->BSI_Seqs[0].SEQ_File) != 1)
	goto error;
    }

    if(ret < 0) goto error;

#ifdef CONFIG_BIGSORT_ALIGN
    for(i = 0 ; i < n ; i++) free(a[i]);
#endif
    free(a);

    return(0);
  }

  /*  Set up final element  */
#ifdef CONFIG_BIGSORT_ALIGN
  if((a[n] = (void *)malloc(len)) == NULL) goto error;
#else
  a[n] = (char *)a[n - 1] + len;
#endif

  /* ***  Now initialize all streams  *** */
  if((pbs->BSI_Seqs = (Sequence *)
#ifdef CONFIG_BIGSORT_ALIGN
      malloc(pbs->BSI_NSeqs * sizeof(*(pbs->BSI_Seqs)))
#else
      malloc(pbs->BSI_NSeqs * (sizeof(*(pbs->BSI_Seqs)) + len) +
             sizeof(*(pbs->BSI_Seqs)))
#endif
      ) == NULL)
    goto error;

#ifndef CONFIG_BIGSORT_ALIGN
  pbs->BSI_Seqs[0].SEQ_Buffer =
    (char *)&pbs->BSI_Seqs[(pbs->BSI_NSeqs + 1) & ~1];
#endif

  for(i = 0 ; i < pbs->BSI_NSeqs ; i++) {
    pbs->BSI_Seqs[i].SEQ_File = NULL;
#ifdef CONFIG_BIGSORT_ALIGN
    if((pbs->BSI_Seqs[i].SEQ_Buffer = (char *)malloc(len)) == NULL)
      goto error;
#else
    if(i)
      pbs->BSI_Seqs[i].SEQ_Buffer = pbs->BSI_Seqs[i - 1].SEQ_Buffer + len;
#endif

    pbs->BSI_Seqs[i].SEQ_NRuns = pbs->BSI_Seqs[i].SEQ_DiffRuns =
      (i < (pbs->BSI_NSeqs-1)) ? 1 : 0;
    pbs->BSI_Seqs[i].SEQ_Count = 0;
  }

  if(selectRun(pbs) < 0) goto error;

  /* ***  Transfer array into a valid heap  *** */
  for(i = n / 2 ; i > 0 ; )
    bsSift(a,pbs->BSI_Compare,pbs->BSI_UPtr,--i,n);

  /* ***  Now read all elements and distribute them  *** */
  for(s = r = n ; ; ) {
#if 0
    printf("DIST %d %d\n",s,r);
    for(i = 0 ; i < r ; i++)printf("  a[%d]='%s'\n",i,a[i]);
#endif

    /*  Write a[0]  */
    tmp = a[0];
    a[0] = pbs->BSI_Seqs[pbs->BSI_Curr].SEQ_Buffer;
    pbs->BSI_Seqs[pbs->BSI_Curr].SEQ_Buffer = tmp;
    if(fwrite(tmp,len,1,pbs->BSI_Seqs[pbs->BSI_Curr].SEQ_File) != 1)
      goto error;
    (pbs->BSI_Seqs[pbs->BSI_Curr].SEQ_Count)++;

    /*  tmp contains the last written field  */

    if(r == n) {
      int ret;

      if((ret = (*get)(pbs->BSI_UPtr,a[n],pbs->BSI_Count)) < 0) goto error;

      if(ret > 0) { /*  EOF  */
	r--;
      } else {
	(pbs->BSI_Count)++;

	if((*(pbs->BSI_Compare))(pbs->BSI_UPtr,tmp,a[n]) <= 0) {
	  /*  Element belongs to same run, put in lower heap  */
	  tmp = a[0]; a[0] = a[n]; a[n] = tmp;
	  bsSift(a,pbs->BSI_Compare,pbs->BSI_UPtr,0,s);
	  continue;
	}
      }
    } else
      r--;

    /*  Put element in upper heap  */
    s--;
    if(s > 0) {
      tmp = a[0]; a[0] = a[s]; a[s] = tmp;
      bsSift(a,pbs->BSI_Compare,pbs->BSI_UPtr,0,s);
    }

    if(s < r) {
      tmp = a[s]; a[s] = a[r]; a[r] = tmp;
      if(s < n/2) bsSift(a,pbs->BSI_Compare,pbs->BSI_UPtr,s,r);
    }

    /*  This run finished  */
    if(s == 0) {
      /*  EOF reached?  */
      if(r == 0) break;

      /*  Start next run  */
      if(selectRun(pbs) < 0) goto error;
      s = r;
    }
  }

#ifdef CONFIG_BIGSORT_ALIGN
  for(i = 0 ; i <= n ; i++) free(a[i]);
  free(a);
#else
  /*  Since malloced region a contains buffers of streams, do not free now  */
  pbs->BSI_Temp = a;
#endif

  return(0);
error:
  return(-1);
}

/*JS*********************************************************************
*   Sift routine, see a documentation for heapsort for an explanation.
*************************************************************************/

static void bsSift(void **array,int (*compare)(void *uptr,const void *a,
					       const void *b),void *uPtr,
		   long l,long r)

/************************************************************************/
{
  long j;
  void *temp;

  /*   Get first element to sink in heap   */
  temp = array[l];

  while((j = (l << 1) + 1) < r) {
    if(j < (r - 1) && (*compare)(uPtr,array[j],array[j + 1]) > 0)
      j++;

    if((*compare)(uPtr,temp,array[j]) <= 0) break;

    /*   Copy lower element to top   */
    array[l] = array[j];
    l = j;
  }

  /*   Now the first element has reached its place   */
  array[l] = temp;

  return;
}

/*JS*********************************************************************
*   If the stream fits completely into memory, use a quicksort to sort it.
*************************************************************************/

/*  Generated automatically with "gensort.h"  */
#define GENSORT_NAME		     sortSpecial
#define GENSORT_ARGS		     ,compare,uPtr
#define GENSORT_ARGSPROTO	     ,int (*compare)(void *uptr, \
					const void *a,const void *b), \
					void *uPtr
#define GENSORT_TYPE		     void *
#define GENSORT_KEYTYPE 	     void *
#define GENSORT_GETKEY(a)            a
#define GENSORT_COMPAREKEYS(k1,k2)   (*compare)(uPtr,k1,k2) < 0

static	/*  Generated as a static routine    */
#include <gensort.h>

/*JS*********************************************************************
*   Select the next sequence to copy the next run to.
*************************************************************************/

static int selectRun(BigSort *pbs)

/************************************************************************/
{
  if(pbs->BSI_Seqs[pbs->BSI_Curr].SEQ_DiffRuns <
     pbs->BSI_Seqs[pbs->BSI_Curr + 1].SEQ_DiffRuns) {
    (pbs->BSI_Curr)++;
  } else if(pbs->BSI_Seqs[pbs->BSI_Curr].SEQ_DiffRuns == 0) {
    int i,nRuns0;

    (pbs->BSI_Level)++;
    nRuns0 = pbs->BSI_Seqs[0].SEQ_NRuns;
    for(i = 0 ; i < (pbs->BSI_NSeqs - 1) ; i++) {
      int save;

      save = pbs->BSI_Seqs[i].SEQ_NRuns;
      pbs->BSI_Seqs[i].SEQ_NRuns = nRuns0 + pbs->BSI_Seqs[i + 1].SEQ_NRuns;
      pbs->BSI_Seqs[i].SEQ_DiffRuns = pbs->BSI_Seqs[i].SEQ_NRuns - save;
    }

    pbs->BSI_Curr = 0;
  } else
    pbs->BSI_Curr = 0;

  (pbs->BSI_Seqs[pbs->BSI_Curr].SEQ_DiffRuns)--;

  if(pbs->BSI_Seqs[pbs->BSI_Curr].SEQ_File == NULL &&
     (pbs->BSI_Seqs[pbs->BSI_Curr].SEQ_File = bsTmpFile()) == NULL)
    return(-1);

  return(0);
}

/*JS*********************************************************************
*   The I/O intensive sorting is done here: Merge runs on nSeqs-1 input
*    streams to one output stream until one stream is exhausted. Then
*    exchange the output stream with the empty input stream and start
*    over. The number of runs on the input stream (i.e. real ones and
*    pseudo runs) must fulfill the condition of the Fibonacci number of
*    a certain order.
*************************************************************************/

static int mergeRuns(BigSort *pbs,int mode)

/************************************************************************/
{
  int *tr,i,nSeqs;
  long len;

  len = pbs->BSI_Len;

  nSeqs = pbs->BSI_NSeqs;

  if((tr = (int *)
#ifdef CONFIG_USE_ALLOCA
      alloca(nSeqs * sizeof(*tr))
#else
      malloc(nSeqs * sizeof(*tr))
#endif
      ) == NULL) goto error;

  /*  Initialize reading for input sequences  */
  for(i = 0 ; i < (nSeqs - 1) ; i++)
    if(pbs->BSI_Seqs[i].SEQ_Count > 0 &&
       (fseek(pbs->BSI_Seqs[i].SEQ_File,0L,SEEK_SET) ||
	fread(pbs->BSI_Seqs[i].SEQ_Buffer,len,1,pbs->BSI_Seqs[i].SEQ_File)
	!= 1))
      goto error;

  /*  Create missing file for merging  */
  if((pbs->BSI_Seqs[nSeqs - 1].SEQ_File = bsTmpFile()) == NULL) goto error;

  do {
    long remRuns;
    int k,mi;

    /* ***  Mix required number of runs for this level from sequences
     * ***   0 .. nSeqs-2 to output sequence nSeqs-1.
     */

    /*  Clear output sequence  */
    if(fseek(pbs->BSI_Seqs[nSeqs - 1].SEQ_File,0L,SEEK_SET) == -1
#ifndef CONFIG_NO_POSIX
       /*  Truncate the output file to zero length  */
       || ftruncate(fileno(pbs->BSI_Seqs[nSeqs - 1].SEQ_File),0) == -1
#endif
       ) goto error;
    pbs->BSI_Seqs[nSeqs - 1].SEQ_DiffRuns = 0;
    pbs->BSI_Seqs[nSeqs - 1].SEQ_Count = 0;

#if 0
    fprintf(stderr,"Level %d\n",pbs->BSI_Level);
    for(k = i = 0 ; i < nSeqs ; i++) {
      fprintf(stderr,"  Seq[%d]: a %d d %d n %d\n",1+i,
	      pbs->BSI_Seqs[i].SEQ_NRuns,pbs->BSI_Seqs[i].SEQ_DiffRuns,
	      pbs->BSI_Seqs[i].SEQ_Count);
      k += pbs->BSI_Seqs[i].SEQ_Count;
    }
    fprintf(stderr,"%d on sequences.\n",k);
#endif

    for(remRuns = pbs->BSI_Seqs[nSeqs - 2].SEQ_NRuns ; remRuns > 0 ;
	remRuns--) {
      /*  Count number of active sequences  */
      for(k = i = 0 ; i < (nSeqs - 1) ; i++)
	if(pbs->BSI_Seqs[i].SEQ_DiffRuns > 0) {
	  /*  Pseudo run on sequence i  */
	  (pbs->BSI_Seqs[i].SEQ_DiffRuns)--;
	} else if(pbs->BSI_Seqs[i].SEQ_Count > 0) {
	  tr[k++] = i;
	}

      if(k == 0) {
	/*  All sequences have at least one pseudo run, so
	 *   record one pseudo run for destination.
	 */
	(pbs->BSI_Seqs[nSeqs - 1].SEQ_DiffRuns)++;
	continue;
      }

      /* **  Now copy one run from sequences t[0] .. t[k-1] to nSeqs-1 ** */
      do {
	char *tmp;

	/*  Find minimal element  */
	for(mi = 0, i = 1 ; i < k ; i++)
	  if((*(pbs->BSI_Compare))(pbs->BSI_UPtr,
				   pbs->BSI_Seqs[tr[i]].SEQ_Buffer,
				   pbs->BSI_Seqs[tr[mi]].SEQ_Buffer) < 0)
	    mi = i;

	/*  Write minimal element from sequence tr[mi] to sequence nSeqs-1  */
	tmp = pbs->BSI_Seqs[tr[mi]].SEQ_Buffer;
	pbs->BSI_Seqs[tr[mi]].SEQ_Buffer = pbs->BSI_Seqs[nSeqs - 1].SEQ_Buffer;
	pbs->BSI_Seqs[nSeqs - 1].SEQ_Buffer = tmp;
	if(fwrite(tmp,len,1,pbs->BSI_Seqs[nSeqs - 1].SEQ_File) != 1)
	  goto error;

	(pbs->BSI_Seqs[nSeqs - 1].SEQ_Count)++;
	(pbs->BSI_Seqs[tr[mi]].SEQ_Count)--;

	/*  Read next element from input sequence, check for EOF  */
	if(pbs->BSI_Seqs[tr[mi]].SEQ_Count > 0 &&
	   fread(pbs->BSI_Seqs[tr[mi]].SEQ_Buffer,len,1,
		 pbs->BSI_Seqs[tr[mi]].SEQ_File) != 1)
	  goto error;

	/*  Check for end of run  */
	if(pbs->BSI_Seqs[tr[mi]].SEQ_Count == 0 ||
	   (*(pbs->BSI_Compare))(pbs->BSI_UPtr,tmp,
				 pbs->BSI_Seqs[tr[mi]].SEQ_Buffer) > 0) {
	  /*  Remove this sequence from input sequences  */
	  tr[mi] = tr[--k];
	}
      } while(k > 0);
    }

    /* ***  Rotate sequences  *** */
    {
      Sequence save;
      long diff;

      save = pbs->BSI_Seqs[nSeqs - 1];
      diff = pbs->BSI_Seqs[nSeqs - 2].SEQ_NRuns;
      for(i = nSeqs - 1 ; i > 0 ; i--) {
	pbs->BSI_Seqs[i] = pbs->BSI_Seqs[i - 1];
	pbs->BSI_Seqs[i].SEQ_NRuns -= diff;
      }
      pbs->BSI_Seqs[0] = save;
      pbs->BSI_Seqs[0].SEQ_NRuns = diff;
    }

    /*  Initialize read on former output sequence  */
    if(fseek(pbs->BSI_Seqs[0].SEQ_File,0L,SEEK_SET) ||
       fread(pbs->BSI_Seqs[0].SEQ_Buffer,len,1,pbs->BSI_Seqs[0].SEQ_File)
       != 1) goto error;

    (pbs->BSI_Level)--;
  } while(pbs->BSI_Level >= 0);

  if(mode & BIGSORT_FDISTINCT) { /*  Remove duplicate entries on stream  */
    FILE *fp;
    char *buff0,*buff1;
    long s,r,n;

    fp = pbs->BSI_Seqs[0].SEQ_File;
    buff0 = pbs->BSI_Seqs[0].SEQ_Buffer;
    buff1 = pbs->BSI_Seqs[1].SEQ_Buffer;
    n = pbs->BSI_Count;

    if(fseek(fp,0L,SEEK_SET) || fread(buff0,len,1,fp) != 1) goto error;

    for(s = 0, r = 1 ; r < n ; r++) {
      if(fread(buff1,len,1,fp) != 1) goto error;
      if((*(pbs->BSI_Compare))(pbs->BSI_UPtr,buff0,buff1) == 0)
	continue;

      s++;
      memcpy(buff0,buff1,len);
      if(s < r &&
	 (fseek(fp,s * len,SEEK_SET) || fwrite(buff0,len,1,fp) != 1 ||
	  fseek(fp,(r + 1) * len,SEEK_SET)))
	goto error;
    }

    pbs->BSI_Count = ++s;

#ifndef CONFIG_NO_POSIX
    /*  Truncate the output file  */
    if(ftruncate(fileno(fp),s * len) == -1) goto error;
#endif
  }

  /*  Rewind resulting sequence  */
  if(fseek(pbs->BSI_Seqs[0].SEQ_File,0L,SEEK_SET)) goto error;

#ifndef CONFIG_USE_ALLOCA
  free(tr);
#endif

  return(0);
error:
#ifndef CONFIG_USE_ALLOCA
  if(tr) free(tr);
#endif

  return(-1);
}

/*JS*********************************************************************
*   Returns a temporary file. May be changed to use a different directory
*    for temporary files.
*************************************************************************/

static FILE *bsTmpFile(void)

/************************************************************************/
{
  FILE *fp;

  if(BSortTempDir) {
    char name[JS_FILENAME_MAX];
    static int Counter;

#ifndef CONFIG_NO_POSIX
    sprintf(name,"%s/bstp%d_%01d",BSortTempDir,(int)getpid(),Counter);
#else
    sprintf(name,"%s/bstmp%02d",BSortTempDir,Counter);
#endif

    Counter++;
    if((fp = fopen(name,"wb+")) == NULL) return(NULL);
    if(remove(name) < 0) {
      fclose(fp);
      return(NULL);
    }
  } else
    fp = tmpfile();

  return(fp);
}
