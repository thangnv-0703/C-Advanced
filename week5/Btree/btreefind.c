/*JS*********************************************************************
*
*    Program : BTREE
*    Language: ANSI-C
*    Author  : Joerg Schoen
*    Purpose : Implement a B-Tree library.
*
*    Part    : Search functions.
*
*************************************************************************/

#ifndef lint
static const char rcsid[] = "$Id: btreefind.c,v 1.6 1998/05/22 08:42:15 joerg Stab joerg $";
#endif

/*********     INCLUDES                                         *********/
#include "btreeint.h"

/*********     DEFINES                                          *********/
#ifndef bayTreeFind
/*  START-DEFINITIONS 1 */
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
/*  END-DEFINITIONS  */
#endif

#define Prototype extern
/*********     PROTOTYPES                                       *********/
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

/*JS*********************************************************************
*   Start a find in the tree with start <= x <= end. If BAYTREEF_INVERT
*    in mode is set, the entries are scanned in inverse order.
*************************************************************************/

BayTreeHandle *bayTreeFStart(BayerTree *bt,BayTreeHandle *btl,
			     char *start,char *end,
			     int (*fCompare)(void *u,const void *a,
					     const void *b),
			     void *user,int mode)

/************************************************************************/
{
  int sMode;

  /*  User may request different comparison routine  */
  if(user == NULL) user = bt->BAT_User;
  if(fCompare == NULL) fCompare = bt->BAT_Compare;

  sMode = !(mode & BAYTREEF_INVERT) ?
    ((mode & BAYTREEF_SLOWER) ? BAYSEEK_LASTELP1 : 0) :
    ((mode & BAYTREEF_ELOWER) ? 0 : BAYSEEK_LASTELP1);

  if((btl == NULL && (btl = (BayTreeHandle *)malloc(sizeof(*btl))) == NULL) ||
     /*  Find position in tree  */
     (btl->BTL_TDepth = bayTreeSeek(bt,(mode & BAYTREEF_INVERT) ? end : start,
				    -1,&(btl->BTL_TPos),fCompare,user,sMode))
     < 0)
    return(NULL);

  /*  Set up structure  */
  btl->BTL_Type = (mode & (BAYTREEF_INVERT|BAYTREEF_SLOWER|BAYTREEF_ELOWER |
			   BAYTREEF_DISTINCT)) | BAYTREEF_ATSTART;
  btl->BTL_BTree   = bt;
  btl->BTL_Start   = start;
  btl->BTL_End     = end;
  btl->BTL_Old     = NULL;
  btl->BTL_Compare = fCompare;
  btl->BTL_User    = user;

  return(btl);
}

/*JS*********************************************************************
*   Returns next set number from the sequential stream, -2 on EOF, -1 on
*    error. If mode != 0, the current position in the tree is not moved.
*************************************************************************/

long bayTreeFNext(BayTreeHandle *btl,char *content,int mode)

/************************************************************************/
{
  BayerTree *bt;
  long next,cLen;
  int depth;

  /*  Tree traversal: If traversing in ascending order
   *   ((btl->BTL_Type & BAYTREEF_INVERT) == 0) we start with curr = -1 and
   *   go up until curr = *p - 1, if traversing in descending order
   *   ((btl->BTL_Type & BAYTREEF_INVERT) == BAYTREEF_INVERT) we start with
   *   curr = *p and go down until curr = 0.
   */

  bt = btl->BTL_BTree;
  cLen = bt->BAT_FieldLength;
  depth = btl->BTL_TDepth;

  /*  Loop until the next element is loaded  */
  for(next = -1 ; next < 0 ;) {
    char *p,*s;
    BTreePage page;
    long curr,fLen;

    /*  Load current page and get current position  */
    if((p = bayGetPage(bt,btl->BTL_TPos[depth].BTP_PageNr,0)) == NULL)
      return(-1);
    curr = btl->BTL_TPos[depth].BTP_Position;

    fLen = BTPAGE_ISLEAF(bt,p) ? BTLEN_FIELDL(bt) : BTLEN_FIELD(bt);

    if(!(btl->BTL_Type & BAYTREEF_INVERT)) {
      /* ***  ASCENDING TRAVERSAL  *** */
      if(curr >= BTPAGE_LENGTH(bt,p)) { /*  end of page reached  */
	/*  Check for root page  */
	if(depth == 0) goto finished;

	/*  Walk up and go to next element  */
	depth--;
	continue;
      }

      s = p + BTOFF_START(bt) + curr * fLen;

      if(0 <= curr) {
	/*  Check for last element with condition "el <= end" */
	if(btl->BTL_End) {
	  int cmp = (*(btl->BTL_Compare))(btl->BTL_User,s + BTOFF_CONTENT(bt),
					  btl->BTL_End);

	  if(!(btl->BTL_Type & BAYTREEF_ELOWER) ? cmp > 0 : cmp >= 0)
	    goto finished;
	}

	/*  Loading next will stop loop on next iteration  */
	memcpyl(&next,s + BTOFF_SETNR(bt),sizeof(next));
	if(content) memcpy(content,s + BTOFF_CONTENT(bt),cLen);
      }

      /*  Next element  */
      if(mode == 0) (btl->BTL_TPos[depth].BTP_Position)++;

      /*  Load right subpage  */
#ifdef CONFIG_BTREE_EXTRALEAF
      if(BTPAGE_ISLEAF(bt,p))
	page = -1;
      else
#endif
	memcpyl(&page,s + BTOFF_SUBPAGE(bt),sizeof(page));
    } else {
      /* ***  DESCENDING TRAVERSAL  *** */
      /*  Correct value if necessary. We have to
       *   pre-decrement the current position.
       */
      if(curr == -1) {
	/*  Set to position at end of page  */
	curr = btl->BTL_TPos[depth].BTP_Position = BTPAGE_LENGTH(bt,p);
      } else
	curr--;

      /* **  End of page reached?  ** */
      if(curr < 0) {
	/*  Check for root page  */
	if(depth == 0) goto finished;

	/*  Walk up and go to next element  */
	depth--;
	continue;
      }

      s = p + BTOFF_START(bt) + curr * fLen;

      if(curr < BTPAGE_LENGTH(bt,p)) {
	/*  Check for last element with condition "start <= el" */
	if(btl->BTL_Start) {
	  int cmp = (*(btl->BTL_Compare))(btl->BTL_User,btl->BTL_Start,s +
					  BTOFF_CONTENT(bt));

	  if(!(btl->BTL_Type & BAYTREEF_SLOWER) ? cmp > 0 : cmp >= 0)
	    goto finished;
	}

	/*  Loading next will stop loop on next iteration  */
	memcpyl(&next,s + BTOFF_SETNR(bt),sizeof(next));
	if(content) memcpy(content,s + BTOFF_CONTENT(bt),cLen);
      }

      /*  Load left subpage  */
#ifdef CONFIG_BTREE_EXTRALEAF
      if(BTPAGE_ISLEAF(bt,p))
	page = -1;
      else
#endif
	memcpyl(&page,s + BTOFF_SUBPAGEM1(bt),sizeof(page));

      /*  Next element  */
      if(mode == 0) btl->BTL_TPos[depth].BTP_Position = curr;
    }

    /*  One page down  */
    if(page >= 0) {
      depth++;
      btl->BTL_TPos[depth].BTP_PageNr = page;
      /*  If traversing in descending order, will be set to
       *   page length on next call.
       */
      btl->BTL_TPos[depth].BTP_Position = -1;
    }

    /*  Remove multiple entries?  */
    if((btl->BTL_Type & BAYTREEF_DISTINCT) && next >= 0 && mode == 0) {
      if(btl->BTL_Old) {
	if((*(btl->BTL_Compare))(btl->BTL_User,btl->BTL_Old,
				 s + BTOFF_CONTENT(bt)) == 0) {
	  next = -1; /*  continue looping  */
	  continue; /*  skip memcpy below  */
	}
      } else if((btl->BTL_Old = (char *)malloc(cLen)) == NULL)
	return(-1);

      memcpy(btl->BTL_Old,s + BTOFF_CONTENT(bt),cLen);
    }
  }

  if(mode == 0) {
    btl->BTL_TDepth = depth;
    btl->BTL_Type &= ~BAYTREEF_ATSTART;
  }

  return(next);
finished:
  /*  We have not corrected BTP_Position, so that
   *   multiple calls will still yield EOF
   */
  btl->BTL_Type &= ~BAYTREEF_ATSTART;
  return(-2);
}

/*JS*********************************************************************
*   Rewind a sequential access stream.
*************************************************************************/

int bayTreeFRewind(BayTreeHandle *btl)

/************************************************************************/
{
  char *cont;
  int mode;

  /*  If stream is already rewound, we are done  */
  if(btl->BTL_Type & BAYTREEF_ATSTART) return(0);

  /*  Free old stack  */
  free(btl->BTL_TPos);

  if(btl->BTL_Old) {
    free(btl->BTL_Old);
    btl->BTL_Old = NULL;
  }

  if(!(btl->BTL_Type & BAYTREEF_INVERT)) {
    cont = btl->BTL_Start;
    mode = (btl->BTL_Type & BAYTREEF_SLOWER) ? BAYSEEK_LASTELP1 : 0;
  } else {
    cont = btl->BTL_End;
    mode = (btl->BTL_Type & BAYTREEF_ELOWER) ? 0 : BAYSEEK_LASTELP1;
  }

  if((btl->BTL_TDepth = bayTreeSeek(btl->BTL_BTree,cont,-1,&(btl->BTL_TPos),
				    btl->BTL_Compare,btl->BTL_User,mode)) < 0)
    return(-1);

  btl->BTL_Type |= BAYTREEF_ATSTART;

  return(0);
}

/*JS*********************************************************************
*   Find the position of a node in the tree. If pstack != NULL, return
*    the current stack depth and the stack in *pStack, otherwise return
*    the set number of the node. If BAYSEEK_UNIQUE in mode is set,
*    returns an error if a node whose contents match exactly already
*    appears in the tree (in case we want to insert the new node).
*    If BAYSEEK_LASTELP1 in mode is set, we then find the last element
*    plus 1 that matches content, otherwise we find the first one.
*    If BAYSEEK_EXACT in mode is set, we only return non-error if an
*    element with the given content and set number appears in the tree
*    (for deletion and update).
*************************************************************************/

long bayTreeSeek(BayerTree *bt,const char *content,long number,
		 BayTreePos **pStack,
		 int (*fCompare)(void *u,const void *a,const void *b),
		 void *user,int mode)

/************************************************************************/
{
  BTreePage page;
  BayTreePos *stack;
  int depth,sMax;

  /*  Go down until we are on a leaf page  */
  sMax = 0;
  stack = NULL;
  for(depth = 0, page = bt->BAT_Root ; page >= 0 ; depth++) {
    char *p;
    long fLen,pos;

#ifdef DEBUG
    /*  If we are on the root page and not in level 0, we
     *   have a serious error!
     */
    if(depth > 0 && page == bt->BAT_Root) {
      fprintf(stderr,"\
ERROR in bayTreeSeek: Corrupted tree, entered root page!!\n");
      goto error;
    }
#endif

    if((p = bayGetPage(bt,page,0)) == NULL) goto error;

    fLen = BTPAGE_ISLEAF(bt,p) ? BTLEN_FIELDL(bt) : BTLEN_FIELD(bt);

    if(content) {
      long upper,comp;

      /*  Binary search on page  */
      pos = 0; upper = BTPAGE_LENGTH(bt,p);
      while(pos < upper) {
        long m = (pos + upper) >> 1;

        comp = (*fCompare)(user,p + (BTOFF_START(bt) + BTOFF_CONTENT(bt)) +
			   m * fLen,content);

	/*  Secondary order criteria is number (not
	 *   if contents are unique!)
	 */
	if(comp == 0 && !(mode & BAYSEEK_UNIQUE) && number >= 0) {
	  BTreeSetNr setNr;

	  memcpyl(&setNr,p + BTOFF_START(bt) + BTOFF_SETNR(bt) + m * fLen,
		  sizeof(setNr));

	  comp = setNr - number;
	}
	if(comp < 0 || (comp == 0 && (mode & BAYSEEK_LASTELP1)))
	  pos = m + 1;
        else
	  upper = m;
      }
    } else if(mode & BAYSEEK_LASTELP1) {
      pos = BTPAGE_LENGTH(bt,p);
    } else {
      pos = 0;
    }

    if(pStack) {
      if(depth >= sMax) {
	sMax += 10;
	if((stack = (BayTreePos *)realloc(stack,sMax * sizeof(*stack)))
	   == NULL) goto error;
      }

      stack[depth].BTP_PageNr   = page;
      stack[depth].BTP_Position = pos;
    }

    /*  Required element may already be found, so check here  */
    if(pos < BTPAGE_LENGTH(bt,p)) {
      BTreeSetNr setNr;

      if((mode & BAYSEEK_UNIQUE) || pStack == NULL) {
	if((*fCompare)(user,p + (BTOFF_START(bt) + BTOFF_CONTENT(bt)) +
		       pos * fLen,content) == 0) {
	  if(mode & BAYSEEK_UNIQUE) {
	    /*  If element matches exactly, content is not unique  */
	    JSErrNo = _ERR_BAYTREE + 2;
	    goto error;
	  }

	  /*  Load set number and return  */
	  memcpyl(&setNr,p + BTOFF_START(bt) + BTOFF_SETNR(bt) + pos * fLen,
		  sizeof(setNr));
	  return(setNr);
	}
      } else if(mode & BAYSEEK_EXACT) {
        memcpyl(&setNr,p + BTOFF_START(bt) + BTOFF_SETNR(bt) + pos * fLen,
		sizeof(setNr));

        if(setNr == number &&
	   (*fCompare)(user,p + (BTOFF_START(bt) + BTOFF_CONTENT(bt)) +
		       pos * fLen,content) == 0)
	  goto found;
      }
    }

    /*  Stop if we come to a leaf page  */
    if(BTPAGE_ISLEAF(bt,p)) break;

    /*  Load number of left subpage to go to  */
    memcpyl(&page,p + (BTOFF_START(bt) + BTOFF_SUBPAGEM1(bt)) +
	    pos * fLen,sizeof(page));
  }

  /*  In case of updating or looking for unique entry
   *   it must have been found previously.
   */
  if((mode & BAYSEEK_EXACT) || pStack == NULL) {
    JSErrNo = _ERR_BAYTREE + 1;
    goto error;
  }

found:
  *pStack = stack;
  return(depth);
error:
  free(stack);
  return(-1);
}
