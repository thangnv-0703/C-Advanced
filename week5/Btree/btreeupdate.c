/*JS*********************************************************************
*
*    Program : BTREE
*    Language: ANSI-C
*    Author  : Joerg Schoen
*    Purpose : Implement a B-Tree library.
*
*    Part    : Update function. Does also insertion and deletion.
*
*************************************************************************/

#ifndef lint
static const char rcsid[] = "$Id: btreeupdate.c,v 1.10 1998/05/22 08:41:32 joerg Stab joerg $";
#endif

/*********     INCLUDES                                         *********/
#include "jsalloca.h"

#include "btreeint.h"

/*********     DEFINES                                          *********/
#ifndef bayTreeInsert
/*  START-DEFINITIONS  */
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
/*  END-DEFINITIONS  */
#endif

#define Prototype extern
/*********     PROTOTYPES                                       *********/
Prototype int            bayTreeUpdate2(BayerTree *bt,long oNumber,
					long nNumber,const char *oCont,
					const char *nCont,int mode);

/*JS*********************************************************************
*   Main routine to insert/delete/update fields. Updates the tree entry
*    'oCont' with set number 'oNumber' to 'nCont' with set number 'nNumber'.
*    If BAYTREEM_UNIQUE in mode is set, ensures that the new content is
*    unique in the tree.
*    This routine contains all the required stuff for page splitting and
*    concatenation, tree growth and shrinking due to root-page overflow
*    and underflow.
*************************************************************************/

int bayTreeUpdate2(BayerTree *bt,long oNumber,long nNumber,
		   const char *oCont,const char *nCont,int mode)

/************************************************************************/
{
  BayTreePos *oStack,*nStack;
  int oDepth,nDepth;
  BTreePage page1,subPage;
  char *temp;

  /*  Quick stop: If old and new contents are identical, we are done.
   *   This also prevents us from complaining about non-uniqueness of
   *   the tree node in case that oCont and nCont are identical.
   */
  if(oCont && nCont && (*(bt->BAT_Compare))(bt->BAT_User,nCont,oCont) == 0 &&
     oNumber == nNumber)
    return(0);

  temp = NULL;

  /* ***  Get position where to delete  *** */
  if(oCont == NULL) {
    /*  no old content, means insertion  */
    oDepth = -1;
  } else {
    char *p1,*p2;
    BTreePage page2;
    long pos1,pos2,fLen;
    int sMax;

    if((oDepth = bayTreeSeek(bt,oCont,oNumber,&oStack,bt->BAT_Compare,
			     bt->BAT_User,BAYSEEK_EXACT)) < 0) goto error;

    /*  To delete an element on a non-leaf page: Walk down the tree
     *   until we reach a leaf page, then exchange the element to
     *   delete with the last one on the leaf page. That means in
     *   effect that the actual insertion and deletion in the main
     *   loop below both take place on leaf pages.
     */

    /*  Load page with element to delete and lock it, so we won't loose it  */
    page1 = oStack[oDepth].BTP_PageNr;
    pos1  = oStack[oDepth].BTP_Position;
    if((p1 = bayGetPage(bt,page1,BFILEMODE_DIRTY | BFILEMODE_PROT)) == NULL)
      goto error;

    fLen = BTLEN_FIELD(bt);
    sMax = oDepth; /*  we might have to increase the stack  */
    for(p2 = p1, pos2 = pos1 ; ; ) {
      /*  Leaf page?  */
      if(BTPAGE_ISLEAF(bt,p2)) break;

      /*  Load left sub page  */
      memcpyl(&page2,p2 + (BTOFF_START(bt) + BTOFF_SUBPAGEM1(bt)) +
	      pos2 * fLen,sizeof(page2));

      /*  Load sub page  */
      oDepth++;
      if(oDepth >= sMax) {
	sMax += 10;
	if((oStack = (BayTreePos *)realloc(oStack,sMax * sizeof(*oStack)))
	   == NULL) goto error1;
      }
      if((p2 = bayGetPage(bt,page2,BFILEMODE_DIRTY)) == NULL) goto error1;

      /*  Use last+1 position on intermediate pages  */
      oStack[oDepth].BTP_PageNr = page2;
      oStack[oDepth].BTP_Position = pos2 = BTPAGE_LENGTH(bt,p2);
#ifdef DEBUGD
      printf("  STEPPING down to page %d pos %d\n",page2,pos2);
#endif
    }

    /*  Check if start page wasn't already a leaf page  */
    if(p2 != p1) {
      /*  Adjust position to LAST element on leaf page and move
       *   this element to the one that must be deleted
       */
      oStack[oDepth].BTP_Position = --pos2;
      memcpy(p1 + BTOFF_START(bt) + BTOFF_USER(bt) + pos1 * fLen,
	     p2 + BTOFF_START(bt) + BTOFF_USER(bt) + pos2 * BTLEN_FIELDL(bt),
	     BTLEN_USER(bt));
    }

    /*  Release lock on the page we started with  */
    if(baySetPage(bt,page1,BFILEMODE_UNPROT) < 0) goto error;
  }

  /* ***  Get position where to insert  *** */
  if(nCont == NULL) {
    nDepth = -1;
  } else {
    /*  In finding the position where to insert we check for uniqueness  */
    if((nDepth = bayTreeSeek(bt,nCont,nNumber,&nStack,bt->BAT_Compare,
			     bt->BAT_User,(mode & BAYSEEK_UNIQUE))) < 0 ||
       (temp = (char *)
#ifdef CONFIG_USE_ALLOCA
	alloca(2 * bt->BAT_FieldLength)
#else
	malloc(2 * bt->BAT_FieldLength)
#endif
	) == NULL)
      goto error;

    /*  We use the temporary work space for passing elements up  */
    memcpy(temp,nCont,bt->BAT_FieldLength);
    nCont = temp;
  }

  /* ****  MAIN LOOP  **** */
  subPage = -1;
  while(oDepth >= 0 || nDepth >= 0) {
    /* ***  Handle deletion and insertion on same page  *** */
    if(nDepth == oDepth && nStack[nDepth].BTP_PageNr ==
       oStack[oDepth].BTP_PageNr) {
      char *p;
      long posD,posI,fLen;

      posD = oStack[oDepth].BTP_Position;
      posI = nStack[nDepth].BTP_Position;

#ifdef DEBUGU
      printf("  DEL/INS on same page %ld, posD %ld posI %ld\n",
	     nStack[nDepth].BTP_PageNr,posD,posI);
#endif

      if((p = bayGetPage(bt,nStack[nDepth].BTP_PageNr,BFILEMODE_DIRTY))
	 == NULL) goto error;

      fLen = BTPAGE_ISLEAF(bt,p) ? BTLEN_FIELDL(bt) : BTLEN_FIELD(bt);

      p += BTOFF_START(bt);
      if(posI < posD) {
	/*        | posI    | posD
	 *  ------++++++++++X-------
	 */
	p += posI * fLen;

	/*  Move intermediate elements one position up	*/
	memmove(p + fLen,p,(posD - posI) * fLen);
      } else if(posI > (posD + 1)) {
	long len;

	posI--;

	/*        | posD    | posI
	 *  ------X++++++++++-------
	 */
	p += posD * fLen;

	len = (posI - posD) * fLen;
	memmove(p,p + fLen,len);
	p += len; /*  adjust to position where to insert  */
      } else {
	/*        | posD                      | posD
	 *  ------X----------------- or  -----X-----------------
	 *        | posI                       | posI
	 */
	p += posD * fLen;
      }

      /*  Set up new entry  */
      memcpy(p + BTOFF_CONTENT(bt),nCont,bt->BAT_FieldLength);
      memcpyl(p + BTOFF_SETNR(bt),&nNumber,sizeof(nNumber));

#ifdef CONFIG_BTREE_EXTRALEAF
      if(fLen != BTLEN_FIELDL(bt))
#endif
	memcpyl(p + BTOFF_SUBPAGE(bt),&subPage,sizeof(subPage));

      break; /*  we are done!  */
    }

    /* ***  If on the same level, INSERTION comes first  *** */
    if(nDepth >= oDepth) {
      char *p1,*dest,*next;
      BTreePage nextSub;
      BTreeSetNr nextSetNr;
      long pos,n,fLen,currMax;
      int flag;

#define FLAG_RELEASELOCK   (1<<0)  /*  Release page lock at the end  */
#define FLAG_DIDPAGESPLIT  (1<<1)  /*  A page split occured  */
#define FLAG_DONTINSERT    (1<<2)  /*  Do not insert element on current page */
      flag = 0;
      page1 = nStack[nDepth].BTP_PageNr;
      pos = nStack[nDepth].BTP_Position;
      nextSub = -1; /*  next subpage element  */

      /*  Load page and get its length  */
      if((p1 = bayGetPage(bt,page1,BFILEMODE_DIRTY)) == NULL) goto error;
      n = BTPAGE_LENGTH(bt,p1);

      if(BTPAGE_ISLEAF(bt,p1)) {
	fLen = BTLEN_FIELDL(bt);
	currMax = bt->BAT_MaxElementLeaf;
      } else {
	fLen = BTLEN_FIELD(bt);
	currMax = bt->BAT_MaxElement;
      }

      /*  Check if page splitting is necessary  */
      if(n < currMax) { /* **  Insertion on non-filled pages  ** */
	dest = p1;
	nDepth = 0; /*  insertion finished  */
	next = NULL;
      } else { /* ***  PAGE SPLITTING  *** */
	BTreePage newPage;
	char *p2;

	/*  Lock old page first, otherwise it might get stolen
	 *   when allocating or accessing new pages.
	 */
	if(baySetPage(bt,page1,BFILEMODE_PROT) < 0) goto error;
	flag |= FLAG_RELEASELOCK | FLAG_DIDPAGESPLIT;

	if((newPage = (*(bt->BAT_PageAdmin))(bt,-1)) < 0 ||
	   (p2 = bayGetPage(bt,newPage,BFILEMODE_DIRTY)) == NULL)
	  goto error1;

#ifdef DEBUGI
	printf("  Page %ld split, new page %ld\n",page1,newPage);
#endif

	/*  Half size of old page and set up new one.
	 *  Central element on old page is put one level up, so
	 *   ensure pages will have same size afterwards.
	 */
	n /= 2;
	if(pos < n) {
	  /*  Insertion will be on old page, so decrease its length.
	   *   That ensures that finally both pages have the same
	   *   length.
	   */
	  n--;
	} else if(pos == n) {
	  /*  Special case: the element to insert is the central one
	   *   which is passed one level up.
	   */
	  flag |= FLAG_DONTINSERT;
	}

	/*  Set size of old page  */
	BTPAGE_LENGTH(bt,p1) = n;

	/*  Set up element to pass to next level  */
	if(!(flag & FLAG_DONTINSERT)) {
	  long n2;

	  /*  Central element of pages together is passed up  */
	  BTPAGE_LENGTH(bt,p2) = n2 = currMax - n - 1;

	  dest = p1 + BTOFF_START(bt) + n * fLen;

	  /*  Use second allocated work space for 'next' element
	   *   which will become the element to insert on the
	   *   next iteration.
	   */
	  next = temp + bt->BAT_FieldLength;
	  memcpy(next,dest + BTOFF_CONTENT(bt),bt->BAT_FieldLength);
	  memcpyl(&nextSetNr,dest + BTOFF_SETNR(bt),sizeof(nextSetNr));

	  /*  Set up new page with remaining elements. For the leftmost
	   *   subpage entry use the unused subpage entry of the element
	   *   passed up. Is also true for leaf pages.
	   */
#ifdef CONFIG_BTREE_EXTRALEAF
	  if(BTPAGE_ISLEAF(bt,p1)) {
	    BTPAGE_SUBPAGE0(bt,p2) = -1;
	    memcpy(p2 + BTOFF_START(bt),dest + fLen,n2 * fLen);
	  } else
#endif
	    memcpy(p2 + BTOFF_START(bt) + BTOFF_SUBPAGEM1(bt),dest +
		   BTOFF_SUBPAGEM1(bt) + fLen,n2 * fLen + sizeof(BTreePage));

	  if(pos <= n) {
	    dest = p1;
	  } else {
	    /*  Insert on new page, adjusting insertion position accordingly  */
	    pos -= n + 1; /*  account for lost central element  */
	    dest = p2;
	    n = n2;
	  }
	} else {
	  /*  New page gets all remaining elements from old page  */
	  BTPAGE_LENGTH(bt,p2) = currMax - n;

	  /*  Do not change inserted element, since it is passed up  */
	  next = NULL;

	  /*  Set up new page with remaining elements from old one,
	   *   use 'subPage' for leftmost subpage.
	   */
	  memcpyl(p2 + BTOFF_START(bt) + BTOFF_SUBPAGEM1(bt),
		  &subPage,sizeof(subPage));
	  memcpy(p2 + BTOFF_START(bt),p1 + BTOFF_START(bt) + n * fLen,
		 (currMax - n) * fLen);
	}

	/*  Subpage for next level  */
	nextSub = newPage;
      }

      if(!(flag & FLAG_DONTINSERT)) {
	char *dest2;

	/* ***  Do actual insertion on a page  *** */
	/*  Set new page length  */
	BTPAGE_LENGTH(bt,dest) = n + 1;

	dest2 = dest + BTOFF_START(bt) + pos * fLen;

	/*  Move contents  */
	if(pos < n) memmove(dest2 + fLen,dest2,(n - pos) * fLen);

	/*  Set up new element  */
	memcpy(dest2 + BTOFF_CONTENT(bt),nCont,bt->BAT_FieldLength);
	memcpyl(dest2 + BTOFF_SETNR(bt),&nNumber,sizeof(nNumber));

#ifdef CONFIG_BTREE_EXTRALEAF
	if(!BTPAGE_ISLEAF(bt,dest))
#endif
	  memcpyl(dest2 + BTOFF_SUBPAGE(bt),&subPage,sizeof(subPage));
      }

      subPage = nextSub;

      /*  If page was split, set up element to pass to upper level  */
      if(next) {
	memcpy((char *)nCont,next,bt->BAT_FieldLength);
	nNumber = nextSetNr;
      }

      /*  Finally check if root page was split  */
      if((flag & FLAG_DIDPAGESPLIT) && nDepth == 0) {
	BTreePage newPage;
	char *p2;

	if((newPage = (*(bt->BAT_PageAdmin))(bt,-1)) < 0 ||
	   (p2 = bayGetPage(bt,newPage,BFILEMODE_DIRTY)) == NULL) {
	  if(flag & FLAG_RELEASELOCK) goto error1;
	  goto error;
	}

#ifdef DEBUGI
	printf("  Root page %d split, newpage %d\n",nStack[nDepth].BTP_PageNr,
	       newPage);
#endif

	/*  Copy old root page to allocated one and set up new root page.
	 *   This procedure ensures that the location of the root page
	 *   never changes. The reserved fields are not copied.
	 */
	memcpy(p2,p1,BTPAGE_SIZE(bt) - bt->BAT_Reserved);

	/*  Clear root page, insertion in next iteration
	 *   will set it up correctly.
	 */
	BTPAGE_LENGTH(bt,p1) = 0;
	BTPAGE_SUBPAGE0(bt,p1) = newPage;
	nStack[nDepth].BTP_Position = 0;
      } else
	/*  Go to next level of insertion  */
	nDepth--;

      /*  Release lock for old page  */
      if((flag & FLAG_RELEASELOCK) && baySetPage(bt,page1,BFILEMODE_UNPROT) < 0)
	goto error;
    } else { /* ***  Do DELETION  *** */
      char *p1,*p2,*p3,*p;
      BTreePage page2,page3;
      long pos,fLen,fLen2,currMax,len,flag;

      /*  Load page and delete element  */
      page1 = oStack[oDepth].BTP_PageNr;
      pos  = oStack[oDepth].BTP_Position;
      if((p1 = bayGetPage(bt,page1,BFILEMODE_DIRTY)) == NULL) goto error;

      if(BTPAGE_ISLEAF(bt,p1)) {
	fLen = BTLEN_FIELDL(bt);
	currMax = bt->BAT_MaxElementLeaf;
      } else {
	fLen = BTLEN_FIELD(bt);
	currMax = bt->BAT_MaxElement;
      }

#ifdef DEBUGD
      printf("  Delete page %d len %d at pos %d\n",page1,
	     BTPAGE_LENGTH(bt,p1),pos);
#endif

      /*  Decrease number of elements on this page
       *   and delete actual element. Must ensure
       *   that the subpage element of the one to
       *   delete has already been handled.
       */
      len = --BTPAGE_LENGTH(bt,p1);
      p = p1 + BTOFF_START(bt) + pos * fLen;
      if(pos < len) memmove(p,p + fLen,(len - pos) * fLen);

      /*  Does page have still enough elements?  */
      if(len >= (currMax / 2)) {
	oDepth = -1; /*  deletion finished  */
	continue; /*  continue loop, probably we still have to insert?  */
      }

      /*  Underflow on root page?  */
      if(oDepth == 0) {
	/*  If root page is non-empty, stop deleting  */
	if(len == 0) {
	  /* ***  Special case: Root page is empty  *** */
#ifdef DEBUGD
	  printf("  Root page empty!\n");
#endif
	  /*  Get the only remaining leftmost subpage element, which
	   *   is the page that will become the new root page.
	   */
	  memcpyl(&page2,p1 + (BTOFF_START(bt) + BTOFF_SUBPAGEM1(bt)),
		  sizeof(page2));

	  /*  Empty tree reached?  */
	  if(page2 >= 0) {
	    /*  Copy the sub page to the root page and free sub page.
	     *   This again ensures that the position of the root page
	     *   never changes.
	     */
	    if(baySetPage(bt,page1,BFILEMODE_PROT) < 0) goto error;

	    if((p2 = bayGetPage(bt,page2,BFILEMODE_DIRTY)) == NULL)
	      goto error1;

	    memcpy(p1,p2,BTPAGE_SIZE(bt) - bt->BAT_Reserved);

	    /*  Unlock and free page  */
	    if(baySetPage(bt,page1,BFILEMODE_UNPROT) < 0 ||
	       (*(bt->BAT_PageAdmin))(bt,page2) < 0) goto error;
	  }
	}

	oDepth = -1; /*  deletion finished  */
	continue;
      }

      /* **  Handle page concatenation  **
       *
       *               page2
       *                /\
       *               /  \
       *              /    \
       *           page1  page3
       */
      /*  Get parent page  */
      page2 = oStack[oDepth - 1].BTP_PageNr;
      pos   = oStack[oDepth - 1].BTP_Position;

      if(baySetPage(bt,page1,BFILEMODE_PROT) < 0) goto error;
      if((p2 = bayGetPage(bt,page2,BFILEMODE_DIRTY | BFILEMODE_PROT)) == NULL)
	goto error1;

      fLen2 = BTLEN_FIELD(bt); /*  page2 cannot be a leaf page  */

      /*  Are we at the last position on the parent page?  */
      if(pos >= BTPAGE_LENGTH(bt,p2)) {
	/*  Exchange p1 and p3  */
	oStack[oDepth - 1].BTP_Position = --pos;
	flag = TRUE;
      } else {
	flag = FALSE;
      }

      /*  Get right hand side page  */
      if(flag && nDepth >= 0 && nStack[nDepth].BTP_PageNr == page2 &&
	 pos == nStack[nDepth].BTP_Position)
	/*  Special case: While inserting we just split the page left
	 *   to us, so the *real* left hand page is subPage instead
	 *   of the pointer on the page.
	 */
	page3 = subPage;
      else
	/*  Get left (flag==TRUE) or right (flag==FALSE) neighbouring page  */
	memcpyl(&page3,p2 + (BTOFF_START(bt) +
			     (flag ? BTOFF_SUBPAGEM1(bt) : BTOFF_SUBPAGE(bt))) +
		pos * fLen2,sizeof(page3));

      /*  Get neighbouring page  */
      if((p3 = bayGetPage(bt,page3,BFILEMODE_DIRTY)) == NULL) {
	(void)baySetPage(bt,page2,BFILEMODE_UNPROT);
	goto error1;
      }

#ifdef DEBUGD
      printf("  Underflow on page2 %d at pos %d flag %s page3 %d\n",
	     page2,pos,flag ? "YES" : "NO",page3);
#endif

      if(flag) { /*  Exchange left and right page  */
	long tp = page1; page1 = page3; page3 = tp;
	p = p1; p1 = p3; p3 = p;
      }

      /*  Go to position of element on page2  */
      p = p2 + BTOFF_START(bt) + pos * fLen2;

      /*  Check if we can lend elements from the neighbouring page, or if
       *   we can unite them to yield a new one.
       */
      len = BTPAGE_LENGTH(bt,p1) + BTPAGE_LENGTH(bt,p3);
      if(len >= currMax) {
	long newLen,diff;

	newLen = len / 2; /*  page1 will have length newLen afterwards  */

	/*  Redistribute elements  */
	if(BTPAGE_LENGTH(bt,p1) < newLen) {
	  diff = newLen - BTPAGE_LENGTH(bt,p1) - 1;

	  /*  Move the parent element from page2 and some elements from
	   *   page3 to page1. The subpage entry for the single element
	   *   from page2 is taken from the leftmost subpage from page3.
	   */
	  memcpy(p1 + BTOFF_START(bt) + BTOFF_USER(bt) +
		 BTPAGE_LENGTH(bt,p1) * fLen,p + BTOFF_USER(bt),BTLEN_USER(bt));

	  /*  Need to copy at least leftmost subpage element  */
#ifdef CONFIG_BTREE_EXTRALEAF
	  if(BTPAGE_ISLEAF(bt,p1))
	    memcpy(p1 + BTOFF_START(bt) + (BTPAGE_LENGTH(bt,p1) + 1) * fLen,
		   p3 + BTOFF_START(bt),diff * fLen);
	  else
#endif
	    memcpy(p1 + BTOFF_START(bt) + BTOFF_SUBPAGEM1(bt) +
		   (BTPAGE_LENGTH(bt,p1) + 1) * fLen,
		   p3 + BTOFF_START(bt) + BTOFF_SUBPAGEM1(bt),
		   diff * fLen + sizeof(BTreePage));

	  /*  Set new element on page2 and move contents of page3  */
	  memcpy(p + BTOFF_USER(bt),
		 p3 + BTOFF_START(bt) + BTOFF_USER(bt) + diff * fLen,
		 BTLEN_USER(bt));

#ifdef CONFIG_BTREE_EXTRALEAF
	  if(BTPAGE_ISLEAF(bt,p3))
	    memmove(p3 + BTOFF_START(bt),p3 + BTOFF_START(bt) +
		    (diff + 1) * fLen,(BTPAGE_LENGTH(bt,p3) - diff - 1) * fLen);
	  else
#endif
	    memmove(p3 + BTOFF_START(bt) + BTOFF_SUBPAGEM1(bt),
		    p3 + BTOFF_START(bt) + BTOFF_SUBPAGEM1(bt) +
		    (diff + 1) * fLen,(BTPAGE_LENGTH(bt,p3) - diff - 1) *
		    fLen + sizeof(BTreePage));

	  BTPAGE_LENGTH(bt,p1) += diff + 1;
	  BTPAGE_LENGTH(bt,p3) -= diff + 1;
	} else if(BTPAGE_LENGTH(bt,p3) < newLen) {
	  /*  Move parent element from page2 and some from page1 to page3
	   *   The subpage entry for the single element from page2 is
	   *   taken from the leftmost subpage from page3.
	   */
	  diff = newLen - BTPAGE_LENGTH(bt,p3) - 1;

#ifdef CONFIG_BTREE_EXTRALEAF
	  if(BTPAGE_ISLEAF(bt,p3))
	    memmove(p3 + BTOFF_START(bt) + (diff + 1) * fLen,
		    p3 + BTOFF_START(bt),BTPAGE_LENGTH(bt,p3) * fLen);
	  else
#endif
	    memmove(p3 + BTOFF_START(bt) + BTOFF_SUBPAGEM1(bt) +
		    (diff + 1) * fLen,
		    p3 + BTOFF_START(bt) + BTOFF_SUBPAGEM1(bt),
		    BTPAGE_LENGTH(bt,p3) * fLen + sizeof(BTreePage));

	  memcpy(p3 + BTOFF_START(bt) + BTOFF_USER(bt) + diff * fLen,
		 p + BTOFF_USER(bt),BTLEN_USER(bt));

	  /*  Need to copy at least leftmost subpage element  */
#ifdef CONFIG_BTREE_EXTRALEAF
	  if(BTPAGE_ISLEAF(bt,p1))
	    memcpy(p3 + BTOFF_START(bt),p1 + BTOFF_START(bt) +
		   (BTPAGE_LENGTH(bt,p1) - diff) * fLen,diff * fLen);
	  else
#endif
	    memcpy(p3 + BTOFF_START(bt) + BTOFF_SUBPAGEM1(bt),
		   p1 + BTOFF_START(bt) + BTOFF_SUBPAGEM1(bt) +
		   (BTPAGE_LENGTH(bt,p1) - diff) * fLen,
		   diff * fLen + sizeof(BTreePage));

	  /*  Set new element on page2 and move contents of page3  */
	  memcpy(p + BTOFF_USER(bt),
		 p1 + BTOFF_START(bt) + BTOFF_USER(bt) +
		 (BTPAGE_LENGTH(bt,p1) - diff - 1) * fLen,BTLEN_USER(bt));

	  BTPAGE_LENGTH(bt,p1) -= diff + 1;
	  BTPAGE_LENGTH(bt,p3) += diff + 1;
	}

	oDepth = -1; /*  stop deleting  */
      } else {
	/* **  We can decrease the number of pages  ** */
#ifdef DEBUGD
	printf("  CONCAT PAGES %d: %d %d , %d\n",page1,
	       BTPAGE_LENGTH(bt,p1),BTPAGE_LENGTH(bt,p3),len + 1);
#endif

	/*
	 *  Move single parent element from page2 and all elements
	 *   from page3 to page1. The subpage entry for the element
	 *   from page2 is taken from the leftmost subpage from page3.
	 */
	memcpy(p1 + BTOFF_START(bt) + BTOFF_USER(bt) +
	       BTPAGE_LENGTH(bt,p1) * fLen,
	       p + BTOFF_USER(bt),BTLEN_USER(bt));
#ifdef CONFIG_BTREE_EXTRALEAF
	if(BTPAGE_ISLEAF(bt,p1))
	  memcpy(p1 + BTOFF_START(bt) + (BTPAGE_LENGTH(bt,p1) + 1) * fLen,
		 p3 + BTOFF_START(bt),BTPAGE_LENGTH(bt,p3) * fLen);
	else
#endif
	  memcpy(p1 + BTOFF_START(bt) + BTOFF_SUBPAGEM1(bt) +
		 (BTPAGE_LENGTH(bt,p1) + 1) * fLen,
		 p3 + BTOFF_START(bt) + BTOFF_SUBPAGEM1(bt),
		 BTPAGE_LENGTH(bt,p3) * fLen + sizeof(BTreePage));

	/*  Concatenated page now contains all elements.  */
	BTPAGE_LENGTH(bt,p1) = len + 1;

	/*  Free the neighbouring page and unlock other one  */
	if((*(bt->BAT_PageAdmin))(bt,page3) < 0) {
	  (void)baySetPage(bt,page2,BFILEMODE_UNPROT);
	  if(flag) page1 = page3;
	  goto error1;
	}

	/*  Go to next level  */
	oDepth--;
      }

      /*  Remove all page locks  */
      if(baySetPage(bt,flag ? page3 : page1,BFILEMODE_UNPROT) < 0 ||
	 baySetPage(bt,page2,BFILEMODE_UNPROT) < 0) goto error;
    }
  }
  /* ****  END MAIN LOOP  **** */

  if(oCont) free(oStack);
  if(nCont) free(nStack);

#ifndef CONFIG_USE_ALLOCA
  if(temp) free(temp);
#endif

  return(0);
error1:
  /*  Clean up: Unlock page1 before return  */
  (void)baySetPage(bt,page1,BFILEMODE_UNPROT);
error:
#ifndef CONFIG_USE_ALLOCA
  free(temp);
#endif
  return(-1);
}
