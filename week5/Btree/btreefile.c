/*JS*********************************************************************
*
*    Program : BTREE
*    Language: ANSI-C
*    Author  : Joerg Schoen
*    Purpose : Implement a B-Tree library.
*
*    Part    : File functions: open/ close/ create/ destroy a Bayer tree.
*
*************************************************************************/

#ifndef lint
static const char rcsid[] = "$Id: btreefile.c,v 1.7 1998/05/22 09:54:48 joerg Stab joerg $";
#endif

/*********     INCLUDES                                         *********/
#include "btreeint.h"

/*********     DEFINES                                          *********/
#ifndef bayTreeCreate
/*  START-DEFINITIONS 1 */
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
/*  END-DEFINITIONS  */
#endif

/* ERROR-DEFINITIONS from bayTree label _ERR_BAYTREE ord 17
   Page size too small
   Set not found
   Field content not unique
   Illegal page length
   Fields not sorted
   Tree's depth changed
*/

#define Prototype extern
/*********     PROTOTYPES                                       *********/
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

/*JS*********************************************************************
*   Set up a structure to operate on a Bayer tree.
*    Input parameters:
*      bf:         The underlying buffered file where the tree resides.
*                  The buffer size used to open the buffered file must
*                  be a multiple of the page size of the Bayer tree, so
*                  we can be sure to get pages as a whole when accessing
*                  them.
*      pLenShift:  Determines the page size (1 << pLenShift) that is
*                  used for the tree.
*      fLength:    Field length of tree entries.
*      reserved:   Number of reserved bytes at the end of every page.
*      root:       The root page of the tree. If root == -1, fPageAdmin
*                  is called to allocate a root page, thus effectively
*                  creating a Bayer tree.
*      fPageAdmin: The page administration routine. If this routine
*                  must call 'bFileFlush', it has to set Bit 2 in mode
*                  to prevent this routine from clearing the DIRTY
*                  flag on pages which results in errors!
*      fCompare:   Comparison routine for entries.
*      user:       User defined pointer that is passed to fCompare.
*************************************************************************/

BayerTree *bayTreeOpen(BayerTree *bt,BuffFile *bf,int pLenShift,long fLength,
		       long reserved,long root,
		       long (*fPageAdmin)(BayerTree *bt,long page),
		       int (*fCompare)(void *u,const void *a,
				       const void *b),void *user)

/************************************************************************/
{
  BayerTree *bt2;

  /* Set up structure */
  if(bt)
    bt2 = bt;
  else if((bt2 = (BayerTree *)malloc(sizeof(*bt2))) == NULL)
    return(NULL);

  bt2->BAT_BFile = bf;
  bt2->BAT_PageShift = pLenShift;
  bt2->BAT_FieldLength = fLength;
  bt2->BAT_PageAdmin = fPageAdmin;
  bt2->BAT_Compare = fCompare;
  bt2->BAT_User = user;

  /*  Maximum number of elements on one page.
   *   Substract number of reserved bytes.
   */
  bt2->BAT_MaxElement = (BTPAGE_SIZE(bt2) - BTOFF_START(bt2) - reserved) /
    BTLEN_FIELD(bt2);
#ifndef CONFIG_BTREE_EXTRALEAF
  bt2->BAT_MaxElementLeaf = bt2->BAT_MaxElement;
#else
  bt2->BAT_MaxElementLeaf = (BTPAGE_SIZE(bt2) - BTOFF_START(bt2) - reserved) /
    BTLEN_FIELDL(bt2);
#endif

  /*  If to less elements, user should better use bigger pages  */
  if(bt2->BAT_MaxElement < 2) {
    JSErrNo = _ERR_BAYTREE + 0;
    goto error;
  }

  bt2->BAT_Reserved = reserved;

  /*  Allocate new root page if required and set up empty root page.
   *   Must set BAT_Root before calling fPageAdmin so that routine
   *   might recognize that this is the very first call.
   */
  if((bt2->BAT_Root = root) == -1) {
    char *rPage;

    if((root = (*fPageAdmin)(bt2,-1)) < 0) goto error;

    if((rPage = bayGetPage(bt2,root,BFILEMODE_DIRTY)) == NULL)
      goto error;

    /*  Root page is empty and is a leaf page  */
    BTPAGE_LENGTH(bt2,rPage) = 0;
    BTPAGE_SUBPAGE0(bt2,rPage) = -1L;

    bt2->BAT_Root = root;
  }

  return(bt2);
error:
  if(bt == NULL) free(bt2);
  return(NULL);
}

/*JS*********************************************************************
*   Destroys a Bayer tree by freeing all pages. If flag is non-zero, the
*    root page is also freed, otherwise it is cleared.
*************************************************************************/

int bayTreeDestroy(BayerTree *bt,int flag)

/************************************************************************/
{
  BayTreePos *stack;
  int depth,mStack;

  /*  Initialize stack  */
  mStack = 10;
  if((stack = (BayTreePos *)malloc(mStack * sizeof(*stack))) == NULL)
    goto error;

  stack[0].BTP_PageNr = bayTreeRootPage(bt);
  stack[0].BTP_Position = -1;
  for(depth = 0 ; 0 <= depth ; ) {
    char *p;
    long page;

    page = stack[depth].BTP_PageNr;

    if((p = bayGetPage(bt,page,0)) == NULL) goto error;

    /*  Last position?  */
    if(
#ifdef CONFIG_BTREE_EXTRALEAF
       !BTPAGE_ISLEAF(bt,p) &&
#endif
       stack[depth].BTP_Position < BTPAGE_LENGTH(bt,p))
      /*  Load subpage  */
      memcpyl(&page,p + (BTOFF_START(bt) + BTOFF_SUBPAGE(bt)) +
	      stack[depth].BTP_Position * BTLEN_FIELD(bt),sizeof(page));
    else
      page = -1;

    /*  If we are at the end of the page or on a leaf page, we
     *   immediately free it and walk one level up.
     */
    if(page == -1) {
      /*  Before moving one level up, free the page  */
      if(flag == 0 && depth == 0) {
	/*  If flag is 0, do not free root page, clear it instead  */
	BTPAGE_LENGTH(bt,p) = 0;
	BTPAGE_SUBPAGE0(bt,p) = -1L;
      } else if((*(bt->BAT_PageAdmin))(bt,stack[depth].BTP_PageNr) < 0)
	goto error;
      depth--;
      continue;
    }

    stack[depth].BTP_Position++;

    depth++;
    if(depth >= mStack) {
      mStack += 10;
      if((stack = (BayTreePos *)realloc(stack,mStack * sizeof(*stack))) ==
	 NULL) goto error;
    }
    stack[depth].BTP_PageNr   = page;
    stack[depth].BTP_Position = -1;
  }

  if(flag)
    /*  Tree no longer has a root page  */
    bt->BAT_Root = -1;

  free(stack);
  return(0);
error:
  return(-1);
}

/*JS*********************************************************************
*   Change the location of the given page by moving its contents to the
*    new one. If newPage == -1, the new one is allocated. The tree is
*    changed appropriately. The old page is *not* freed.
*************************************************************************/

int bayTreeChPage(BayerTree *bt,long page,long nPage)

/************************************************************************/
{
  char *p1,*p2;
  BTreePage newPage;

  /*  Get new page and lock it  */
  if(((newPage = nPage) == -1 &&
      (newPage = (*(bt->BAT_PageAdmin))(bt,-1)) < 0) ||
     (p1 = bayGetPage(bt,newPage,BFILEMODE_DIRTY | BFILEMODE_PROT)) == NULL)
    goto error2;

  /*  Get old one and copy it  */
  if((p2 = bayGetPage(bt,page,0)) == NULL) goto error2;
  memcpy(p1,p2,BTPAGE_SIZE(bt) - bt->BAT_Reserved);

  if(BTPAGE_LENGTH(bt,p1) == 0) {
#ifdef DEBUG
    /*  Empty page encountered -- must be root page  */
    if(page != bt->BAT_Root) {
      JSErrNo = _ERR_BAYTREE + 3;
      goto error;
    }
#endif

    bt->BAT_Root = newPage;
  } else {
    BayTreePos *stack;
    BTreeSetNr setNr;
    int depth;

    stack = NULL;

    /*  Load first element from page and search for it to build up the stack  */
    memcpyl(&setNr,p1 + BTOFF_START(bt) + BTOFF_SETNR(bt),sizeof(setNr));
    if((depth = bayTreeSeek(bt,p1 + BTOFF_START(bt) + BTOFF_CONTENT(bt),setNr,
			    &stack,bt->BAT_Compare,bt->BAT_User,BAYSEEK_EXACT))
       < 0) goto error;

    /*  Now change link to point to new page  */
    if(depth == 0) {
      /*  Is root page  */
      bt->BAT_Root = newPage;
    } else {
      if((p2 = bayGetPage(bt,stack[depth - 1].BTP_PageNr,BFILEMODE_DIRTY))
	 == NULL) {
	free(stack);
	goto error;
      }

#ifdef DEBUG
      {
	long page2;

	memcpyl(&page2,p2 + (BTOFF_START(bt) + BTOFF_SUBPAGEM1(bt)) +
		stack[depth - 1].BTP_Position * BTLEN_FIELD(bt),sizeof(page2));

	if(page2 != page) {
	  fprintf(stderr,"Internal Error in bayTreeChPage (%ld != %ld)!!\n",
		  page2,page);
	}
      }
#endif

      memcpyl(p2 + (BTOFF_START(bt) + BTOFF_SUBPAGEM1(bt)) +
	      stack[depth - 1].BTP_Position * BTLEN_FIELD(bt),&newPage,
	      sizeof(newPage));
    }

    free(stack);
  }

  /*  Finally unlock original page  */
  if(baySetPage(bt,newPage,BFILEMODE_UNPROT)) goto error2;

  return(0);
error:
  (void)baySetPage(bt,newPage,BFILEMODE_UNPROT);
error2:
  if(nPage == -1 && newPage != -1) (void)(*(bt->BAT_PageAdmin))(bt,newPage);
  return(-1);
}
