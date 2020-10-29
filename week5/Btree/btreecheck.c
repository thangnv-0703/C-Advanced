/*JS*********************************************************************
*
*    Program : BTREE
*    Language: ANSI-C
*    Author  : Joerg Schoen
*    Purpose : Implement a B-Tree library.
*
*    Part    : Check a tree for consistency.
*
*************************************************************************/

#ifndef lint
static const char rcsid[] = "$Id: btreecheck.c,v 1.5 1998/05/22 08:43:37 joerg Stab joerg $";
#endif

/*********     INCLUDES                                         *********/
#include <jsalloca.h>

#include "btreeint.h"

/*********     DEFINES                                          *********/

#define Prototype extern
/*********     PROTOTYPES                                       *********/
Prototype int            bayTreeCheck(BayerTree *bt,
				      int (*routine)(BayerTree *b,char *p,
						     long pagenr,long n,
						     int depth),int mode);

/*JS*********************************************************************
*   Check a tree for consistency. Bits in mode:
*     0: Print messages to stderr.
*     1: Break immediately on error.
*     2: Print statistics.
*     3: Call routine also for elements, passing the element as p, the
*        set number as pagenr, n is -1 to recognize these calls.
*     4: Ensures uniqueness of the tree.
*************************************************************************/

int bayTreeCheck(BayerTree *bt,int (*routine)(BayerTree *bt,char *p,
					      long pagenr,long n,int depth),
		 int mode)

/************************************************************************/
{
  BayTreePos *stack;
  char *oField;
  BTreeSetNr oSetNr;
  long maxElements;
  int depth,count,nPages,sMax,mDepth;

  oField = NULL;
  sMax = 10;
  if((stack = (BayTreePos *)malloc(sMax * sizeof(*stack))) == NULL) goto error;

  mDepth = -1;

  /*  Walk tree and check for correct order and consistency  */
  depth = 0;
  stack[depth].BTP_PageNr = bt->BAT_Root;
  stack[depth].BTP_Position = -1;
  for(maxElements = nPages = count = 0 ; depth >= 0 ; ) {
    BTreePage page;
    char *p;
    long fLen,currMax,curr;

    if((p = bayGetPage(bt,stack[depth].BTP_PageNr,0)) == NULL) goto error;
    curr = stack[depth].BTP_Position;

    /*  End of page?  */
    if(curr >= BTPAGE_LENGTH(bt,p)) {
      /*  Walk up and go to next element  */
      depth--;
      continue;
    }

    if(BTPAGE_ISLEAF(bt,p)) {
      fLen = BTLEN_FIELDL(bt);
      currMax = bt->BAT_MaxElementLeaf;
      page = -1;
    } else {
      fLen = BTLEN_FIELD(bt);
      currMax = bt->BAT_MaxElement;

      /*  Load subpage  */
      memcpyl(&page,p + (BTOFF_START(bt) + BTOFF_SUBPAGEM1(bt)) +
	      (stack[depth].BTP_Position + 1) * fLen,sizeof(page));
    }

    /*  All pages except root page must lay within a certain range.
     *   Non-root page must lay in certain interval, root page
     *   may only be empty when tree is empty.
     */
    if(curr < 0 &&
       (depth ?	(BTPAGE_LENGTH(bt,p) < currMax / 2 ||
		 currMax < BTPAGE_LENGTH(bt,p)) :
	(BTPAGE_LENGTH(bt,p) == 0 && page != -1))) {
      JSErrNo = _ERR_BAYTREE + 3;

      if(mode & 1)
	fprintf(stderr,"\
  ERROR bayTreeCheck: Page Length %ld not in [%ld,%ld]!\n",
		BTPAGE_LENGTH(bt,p),currMax / 2,currMax);
      if(mode & 2) goto error;
    }

    if(curr >= 0) { /*  Look at element  */
      char *s;
      BTreeSetNr setNr;

      s = p + BTOFF_START(bt) + BTOFF_CONTENT(bt) + curr * fLen;
      memcpyl(&setNr,s + (BTOFF_SETNR(bt) - BTOFF_CONTENT(bt)),sizeof(setNr));

      /*  Undocumented debugging mode  */
      if(mode & 32)
	printf("%-25s (i=%3d p=%3ld curr=%3ld: set %3ld)\n",s,depth,
	       stack[depth].BTP_PageNr,curr,setNr);
      count++;

      if(oField) {
	int res = (*bt->BAT_Compare)(bt->BAT_User,oField,s);

	if((mode & 16) && res == 0) {
	  JSErrNo = _ERR_BAYTREE + 2;

	  if(mode & 1)
	    fprintf(stderr,"\
  ERROR: Field not unique on page %ld pos %ld: '%.*s',%ld, '%.*s',%ld!\n",
		    stack[depth].BTP_PageNr,curr,(int)bt->BAT_FieldLength,
		    oField,oSetNr,(int)bt->BAT_FieldLength,s,setNr);

	  if(mode & 2) goto error;
	} else if(res > 0 || (res == 0 && oSetNr > setNr)) {
	  JSErrNo = _ERR_BAYTREE + 4;

	  if(mode & 1)
	    fprintf(stderr,"\
  ERROR: Comparison failed on page %ld pos %ld: '%.*s',%ld > '%.*s',%ld!\n",
		    stack[depth].BTP_PageNr,curr,(int)bt->BAT_FieldLength,
		    oField,oSetNr,(int)bt->BAT_FieldLength,s,setNr);
	  if(mode & 2) goto error;
	}
      } else if((oField = (char *)
#ifdef CONFIG_USE_ALLOCA
		 alloca(bt->BAT_FieldLength)
#else
		 malloc(bt->BAT_FieldLength)
#endif
		 ) == NULL)
	goto error;

      memcpy(oField,s,bt->BAT_FieldLength);
      oSetNr = setNr;

      /*  Call user routine for this element  */
      if((mode & 8) && routine && (*routine)(bt,oField,oSetNr,-1,depth) < 0)
	goto error;
    } else {
      nPages++;
      maxElements += BTPAGE_ISLEAF(bt,p) ? bt->BAT_MaxElementLeaf :
	bt->BAT_MaxElement;

      /*  Call user routine for this page  */
      if(routine && (*routine)(bt,p,stack[depth].BTP_PageNr,BTPAGE_LENGTH(bt,p),
			       depth) < 0)
	goto error;
    }

    /*  Next element  */
    (stack[depth].BTP_Position)++;

    /*  One page down  */
    if(page >= 0) {
      depth++;

      if(depth >= sMax) {
	sMax += 10;
	if((stack = (BayTreePos *)realloc(stack,sMax * sizeof(*stack)))
	   == NULL) goto error;
      }
      stack[depth].BTP_PageNr = page;
      stack[depth].BTP_Position = -1;
      continue;
    }

    /*  Check that tree's depth doesn't change  */
    if(mDepth == -1)
      mDepth = depth;
    else if(mDepth != depth) {
      JSErrNo = _ERR_BAYTREE + 5;

      if(mode & 1)
	fprintf(stderr,"  ERROR: Tree's depth changed %d != %d!\n",depth,
		mDepth);
      if(mode & 2) goto error;
    }
  }

  if(mode & 20) {
    printf("  %d nodes on %d page(s)",count,nPages);
    if(mDepth > 0)
      printf(", depth %d",mDepth);
    if(count)
      printf(". Usage %.2g%%.\n",(1e2 * count) / maxElements);
    else
      printf(".\n");
  }

  free(stack);

#ifndef CONFIG_USE_ALLOCA
  free(oField);
#endif

  return(count);
error:
  return(-1);
}
