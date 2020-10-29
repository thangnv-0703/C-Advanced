/*JS*********************************************************************
*
*    Program : BTREE
*    Language: ANSI-C
*    Author  : Joerg Schoen
*    Purpose : Implement a B-Tree library.
*
*    Part    : Building up an empty tree from a stream.
*
*************************************************************************/

#ifndef lint
static const char rcsid[] = "$Id: btreebuild.c,v 1.7 1998/05/22 08:40:58 joerg Stab joerg $";
#endif

/*********     INCLUDES                                         *********/
#include <jsalloca.h>

#include "btreeint.h"

/*********     DEFINES                                          *********/
/* **  Interface for bigsort routines when building the tree  ** */
struct WrapBigSort {
  /*  User routine to get next entry  */
  long     (*Get)(void *uptr,char *buff,long nr);
  void      *GetUPtr;

  BayerTree *BTree;
  /*  To avoid double indirect access to the
   *   comparison function, copy it here.
   */
  int      (*Compare)(void *uptr,const void *a,const void *b);
};

#define Prototype extern
/*********     PROTOTYPES                                       *********/
Prototype int            bayTreeBuild(BayerTree *bt,
				      long (*get)(void *uptr,char *buff,
						  long nr),void *uPtr,
				      double frac,int mode);
static int               doBSGet(struct WrapBigSort *wbs,char *buff,long nr);
static int               doBSCompare(struct WrapBigSort *wbs,const void *a,
				     const void *b);

/*JS*********************************************************************
*   Fill an *empty* tree with a stream of (unsorted) entries. frac
*    specifies the percentage of page fill-up. The routine get must
*    return the current set number, -1 on error and -2 on EOF.
*************************************************************************/

int bayTreeBuild(BayerTree *bt,long (*get)(void *uptr,char *buff,long nr),
		 void *uPtr,double frac,int mode)

/************************************************************************/
{
  BigSortHandle bsh;
  struct WrapBigSort bswrap;
  struct BayTreeBuild {
    long PageNr,Current;
    long NPage; /*  Number of elements on every page  */
    long Count; /*  How long to wait until we set NPage to the fixed value */
#ifdef DEBUG
    long Nr;
#endif
  } *stack;
  char *oField;
  long nPage,nPagel,count,n,m;
  int i,depth;

  stack = NULL;

  /*  First check if tree is empty, otherwise do
   *   simply insert all entries directly.
   */
  if((oField = bayGetPage(bt,bayTreeRootPage(bt),0)) == NULL) return(-1);

  if(BTPAGE_LENGTH(bt,oField) != 0) {
    if((oField = (char *)
#ifdef CONFIG_USE_ALLOCA
	alloca(bt->BAT_FieldLength)
#else
	malloc(bt->BAT_FieldLength)
#endif
	) == NULL) return(-1);

    for(count = 0 ; (n = (*get)(uPtr,oField,count)) >= 0 ; count++)
      if(bayTreeInsert(bt,n,oField,mode) < 0)
	return(-1);

#ifndef CONFIG_USE_ALLOCA
    free(oField);
#endif

    if(n == -1) return(-1);

    return(0);
  }

  /*  First sort all elements for the tree building  */
  bswrap.BTree   = bt;
  bswrap.GetUPtr = uPtr;
  bswrap.Get     = get;
  bswrap.Compare = bt->BAT_Compare;

  if(bigSortOpen(&bsh,BTLEN_USER(bt),(int (*)(void *,void *,long))doBSGet,
		 (int (*)(void *,const void *,const void *))doBSCompare,
		 &bswrap,0) == NULL)
    return(-1);

  /*  How many elements do we have?  */
  n = bigSortLength(&bsh);

  if(!(mode & BAYTREEM_UNIQUE))
    oField = NULL;
  else if((oField = (char *)
#ifdef CONFIG_USE_ALLOCA
	   alloca(bt->BAT_FieldLength)
#else
	   malloc(bt->BAT_FieldLength)
#endif
	   ) == NULL) return(-1);

  /*  Get number of elements on every page and
   *   make sure it is within interval
   */
  if(frac < 0.5) frac = 0.5;
  else if(frac > 1.0) frac = 1.0;
  nPage  = (long)(frac * bt->BAT_MaxElement);
  nPagel = (long)(frac * bt->BAT_MaxElementLeaf);

  /*  Estimate needed depth of tree to contain all n elements  */
  for(depth = 1, m = nPagel + 1 ; (m - 1) < n ; depth++) m *= nPage + 1;

  if((stack = (struct BayTreeBuild *)malloc(depth * sizeof(*stack))) == NULL)
    goto error;

  /*  To fullfill the condition that every page has a length >= MaxElement / 2,
   *   we use the first page on every level to padd elements.
   */
  for(m = n, i = depth - 1 ; 0 < i ; i--) {
    ldiv_t ld;
    long currNPage,currMax,r;

    if(i == (depth - 1)) {
      currNPage = nPagel;
      currMax = bt->BAT_MaxElementLeaf;
    } else {
      currNPage = nPage;
      currMax = bt->BAT_MaxElement;
    }

    ld = ldiv(m + 1,currNPage + 1);
    m = ld.quot;

#ifdef DEBUG
    stack[i].Nr = ld.rem ? (m * currNPage + ld.rem - 1) : (m * currNPage);
#endif

    /*  Will final page on this level fullfill criteria?  */
    r = ld.rem - 1 - (currMax / 2);
    if(ld.rem == 0) {
      /*  Matches perfect  */
      stack[i].NPage = currNPage;
      stack[i].Count = -1;
      m--;
    } else if(r >= 0) {
      /*  Last page will contain less elements than wanted  */
      stack[i].NPage = currNPage;
      stack[i].Count = -1;
    } else if(currNPage > (currMax / 2) && (m + r) >= 0) {
      /*  Steal single elements from every page to make final page be OK  */
      stack[i].NPage = currNPage - 1;
      stack[i].Count = -r;
    } else if((currNPage + ld.rem) <= currMax) {
      /*  First page will contain more elements than expected  */
      stack[i].NPage = currNPage + ld.rem;
      stack[i].Count = 1;
      m--;
#ifdef DEBUG
      stack[i].Nr++;
#endif
    } else {
      /*  Distribute homogenous between first and last page  */
      stack[i].NPage = (currNPage + ld.rem) / 2;
      stack[i].Count = 1;
    }
  }
  stack[0].NPage = (depth == 1) ? nPagel : nPage;
#ifdef DEBUG
  stack[0].Nr = m;
#endif

  /*  Prevent underflow of root page  */
  if(m == 0 && depth > 1) {
    depth--;
    for(i = 0 ; i < depth ; i++) {
      stack[i].NPage = stack[i + 1].NPage;
      stack[i].Count = stack[i + 1].Count;
#ifdef DEBUG
      stack[i].Nr = stack[i + 1].Nr;
#endif
    }
  }

  /*  Set up stack of pages  */
  stack[0].PageNr = bayTreeRootPage(bt);
  stack[0].Current = -1;
  for(i = 1 ; i < depth ; i++) {
    if((stack[i].PageNr = (*(bt->BAT_PageAdmin))(bt,-1)) < 0) {
      while(0 < --i) (void)(*(bt->BAT_PageAdmin))(bt,stack[i].PageNr);
      goto error;
    }
    stack[i].Current = -1;
  }

#ifdef XDEBUG
  printf("%d elements, depth %d, nPage %d/%d\n",n,depth,nPage,nPagel);
  for(i = 0 ; i < depth ; i++)
    printf("  %d: pagnr %d nr %d nPage %d\n",i,stack[i].PageNr,stack[i].Nr,
	   stack[i].NPage);
#endif

  /* **  Now distribute the sorted keys homogenous in the tree  ** */
  count = 0;
  for(i = depth - 1 ; i >= 0 ; ) {
    char *p;
    long np;

    /*  The first pages get a precalculated number of elements,
     *   all other get nPage.
     */
    np = stack[i].NPage;

    if((p = bayGetPage(bt,stack[i].PageNr,BFILEMODE_DIRTY)) == NULL)
      goto error2;

    if(i == (depth - 1)) { /* **  Construct leaf pages  ** */
static const BTreePage Minus1 = -1L;
      long j,fLen;

      fLen = BTLEN_FIELDL(bt);

      /*  Fill leaf pages completely  */
      BTPAGE_SUBPAGE0(bt,p) = Minus1;
      for(j = 0 ; count < n && j < np ; j++, count++) {
	char *s;

#ifndef CONFIG_BTREE_EXTRALEAF
	/*  Set subpage field to -1  */
	memcpyl(p + BTOFF_START(bt) + BTOFF_SUBPAGE(bt) + j * fLen,&Minus1,
		sizeof(Minus1));
#endif
	s = p + BTOFF_START(bt) + BTOFF_USER(bt) + j * fLen;

	/*  If counting is correct, we must not get out of elements  */
	if(bigSortNext(&bsh,s) < 0) goto error2;

	if(oField) {
	  /*  If element matches exactly, content is not unique  */
	  if(count > 0 && (*(bt->BAT_Compare))(bt->BAT_User,oField,s) == 0) {
	    JSErrNo = _ERR_BAYTREE + 2;
	    goto error2;
	  }
	  memcpy(oField,s,bt->BAT_FieldLength);
	}
      }
      BTPAGE_LENGTH(bt,p) = j;

      if(--(stack[i].Count) == 0) stack[i].NPage = nPagel;

#ifdef DEBUG
      if(i && j < (bt->BAT_MaxElementLeaf/2))
	 printf("  WARNING: Leaf with %ld < %ld!\n",j,bt->BAT_MaxElementLeaf/2);
      stack[i].Nr -= j;
#endif

      i--; /*  Go to parent page  */
    } else {
      long fLen;

      fLen = BTLEN_FIELD(bt);

      /*  Set up pointer to subpage  */
      memcpyl(p + BTOFF_START(bt) + BTOFF_SUBPAGE(bt) + stack[i].Current * fLen,
	      &(stack[i + 1].PageNr),sizeof(stack[i + 1].PageNr));
      BTPAGE_LENGTH(bt,p) = ++(stack[i].Current);

      if(stack[i].Current < np && count < n) {
	char *s;

	/*  Set up element  */
	s = p + BTOFF_START(bt) + BTOFF_USER(bt) + stack[i].Current * fLen;

	/*  If counting is correct, we must not get out of elements  */
	if(bigSortNext(&bsh,s) < 0) goto error2;

#ifdef DEBUG
	stack[i].Nr--;
#endif

	if(oField) {
	  /*  If element matches exactly, content is not unique  */
	  if(count >0 && (*(bt->BAT_Compare))(bt->BAT_User,oField,s) == 0) {
	    /*  Set error condition  */
	    JSErrNo = _ERR_BAYTREE + 2;
	    goto error2;
	  }
	  memcpy(oField,s,bt->BAT_FieldLength);
	}

	count++;

	/*  Go to leaf page and get new pages  */
	do {
	  i++;
	  if((stack[i].PageNr = (*(bt->BAT_PageAdmin))(bt,-1)) < 0)
	    goto error2;
	  stack[i].Current = -1;
	} while(i < (depth - 1));
      } else {
	/*  Intermediate page full or no more elements to come  */
#ifdef DEBUG
	if(i && stack[i].Current < (bt->BAT_MaxElement/2))
	   printf("  WARNING: Level %d page with %ld < %ld!\n",i,
		  stack[i].Current,bt->BAT_MaxElement/2);
#endif

	if(--(stack[i].Count) == 0) stack[i].NPage = nPage;
	i--;
      }
    }
  }

#ifdef DEBUG
  /*  Check if our estimations were correct  */
  for(i = 0 ; i < depth ; i++)
    if(stack[i].Nr != 0)
      fprintf(stderr,"ERROR bayTreeBuild: Wrong estimation in level %d: %ld\n",
	      i,stack[i].Nr);
#endif

#ifndef CONFIG_USE_ALLOCA
  if(oField) free(oField);
#endif

  /*  Clean up  */
  if(bigSortClose(&bsh,1) < 0) goto error;

  free(stack);

  return(0);
error2:
  /*  Free also all pages and reset root page  */
  for(;;) {
    if(i < (depth - 1)) {
      char *p;
      long curr;

      if((p = bayGetPage(bt,stack[i].PageNr,0)) == NULL) goto error;

      if(stack[i].Current == -2) stack[i].Current = BTPAGE_LENGTH(bt,p);

      if(-1 < stack[i].Current) {
	curr = --(stack[i].Current);

	i++;
	memcpyl(&(stack[i].PageNr),p + BTOFF_START(bt) + BTOFF_SUBPAGE(bt) +
		curr * BTLEN_FIELD(bt),sizeof(stack[i].PageNr));

	if(i < (depth - 1)) {
	  stack[i].Current = -2;
	  continue;
	}
      }
    }

    if(i == 0) {
      char *p;

      /*  Clear root page  */
      if((p = bayGetPage(bt,bayTreeRootPage(bt),0)) == NULL) goto error;
      BTPAGE_LENGTH(bt,p) = 0;
      BTPAGE_SUBPAGE0(bt,p) = -1L;
      break;
    }

    /*  Free the pages  */
    if((*(bt->BAT_PageAdmin))(bt,stack[i].PageNr) < 0) goto error;
    i--;
  }

error:
  (void)bigSortClose(&bsh,1);
  if(stack) free(stack);
  return(-1);
}

/*JS*********************************************************************
*   Wrapper routine for sorting the input.
*************************************************************************/

static int doBSGet(struct WrapBigSort *wbs,char *buff,long nr)

/************************************************************************/
{
  long num;

  /*  Generate fields in a way that we can copy them
   *   simply into the tree afterwards.
   */
  if((num = (*(wbs->Get))(wbs->GetUPtr,buff + (BTOFF_CONTENT(wbs->BTree) -
					       BTOFF_USER(wbs->BTree)),nr))
     == -2)
    return(1); /*  signal EOF  */

  if(num < 0) return(-1);

  memcpyl(buff + (BTOFF_SETNR(wbs->BTree) - BTOFF_USER(wbs->BTree)),
	  &num,sizeof(num));

  return(0);
}

/*JS*********************************************************************
*   Wrapper routine for sorting of the input.
*************************************************************************/

static int doBSCompare(struct WrapBigSort *wbs,const void *a,const void *b)

/************************************************************************/
{
  int cmp;

  cmp = (*(wbs->Compare))(wbs->BTree->BAT_User,
			  (char *)a + (BTOFF_CONTENT(wbs->BTree) -
				       BTOFF_USER(wbs->BTree)),
			  (char *)b + (BTOFF_CONTENT(wbs->BTree) -
				       BTOFF_USER(wbs->BTree)));

  if(cmp == 0) {
    long an,bn;

    /*  Second order criteria in tree is set number  */
    memcpyl(&an,(char *)a + (BTOFF_SETNR(wbs->BTree) - BTOFF_USER(wbs->BTree)),
	    sizeof(an));
    memcpyl(&bn,(char *)b + (BTOFF_SETNR(wbs->BTree) - BTOFF_USER(wbs->BTree)),
	    sizeof(bn));

    /*  Prevent overflow on machines where sizeof(int) < sizeof(long)  */
    an -= bn;
    cmp = an == 0L ? 0 : (an < 0L ? -1 : 1);
  }

  return(cmp);
}
