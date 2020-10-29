/*JS*********************************************************************
*
*    Program : BTREE
*    Language: ANSI-C
*    Author  : Joerg Schoen
*    Purpose : Implement a B-Tree
*
*    The complete B-Tree implementation sits upon the 'BuffFile'-package
*     that provides direct, buffered access to regions of a file,
*     allowing to read, write and lock pages.
*
*    The B-tree interface consists of the following routines (some of
*     them may be macros):
*
*      bayTreeCreate:   Create a new, empty bayer tree.
*      bayTreeOpen:     Open an existing Bayer tree.
*      bayTreeClose:    Close a bayer tree.
*
*      bayTreeBuild:    Build up a tree with a stream of entries.
*                       The tree must be empty initially.
*      bayTreeDestroy:  Delete a complete tree, freeing all pages.
*
*      bayTreeInsert, bayTreeDelete, bayTreeUpdate:
*                       Insert/delete and update single entries in a tree.
*      bayTreeChSetNr   Change the set number and not the contents of an
*                       entry.
*
*      bayTreeRootPage: Get the root page of a Bayer tree.
*      bayTreeCheck:    Check a given tree for correctness.
*      bayTreeChPage:   Move a given page in a tree to a new position.
*
*      bayTreeFind:     Find the set number of a unique node in the tree.
*      bayTreeFStart:   Start sequential access to keys in a certain range
*                       in ascending or descending order. On demand, multiple
*                       keys may be deleted.
*      bayTreeFNext:    Get the next key in the stream obtained by
*                       'bayTreeFStart'.
*      bayTreeFRewind:  Rewind a sequential access stream.
*      bayTreeFEnd:     Finish sequential access, close the stream.
*
*************************************************************************/

#ifndef __BTREEINT_H__
#define __BTREEINT_H__

/*********     INCLUDES                                         *********/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <jsconfig.h>

#include <jssubs.h>

/*
 *  Layout of pages in the tree:
 *  ------------------------------------------
 *             long   nElem     Number of elements on this page.
 *             long   lSubPage  Leftmost subpage.
 *
 *   nElem * | n*char field     Actual field content.
 *           | long   setNr     Set number of this field.
 *           | long   subPage   Number of sub page or -1 for leaf page.
 *
 *   If the configuration option 'CONFIG_BTREE_EXTRALEAF' is set, on
 *    leaf pages the subPage element is missing in every field, except
 *    the leftmost 'lSubPage'.
 *   The sequence of the sub-elements must not be changed since most
 *    memory copies depend on the subpage coming last!
 *   This layout should ensure that in case of a double field (n=8)
 *    it is aligned correctly. This is not true when CONFIG_BTREE_EXTRALEAF
 *    is used.
 */

/*  We use a different layout for leaf pages, thus having a B*Tree (?) */
#define CONFIG_BTREE_EXTRALEAF

/*  We might speed up memcpy(..,..,sizeof(long)) here by a special
 *   implementation, if the compiler does not recognize it.
 */
#define memcpyl(d,s,l)    memcpy(d,s,l)

/* **  Internal definitions/ macros  ** */
typedef long BTreeSetNr,BTreePage;

/*  Internal modes for routine bayTreeSeek. These
 *   modes are mutually exclusive.
 */
#define BAYSEEK_UNIQUE   BAYTREEM_UNIQUE /*  Element must not appear in tree */
#define BAYSEEK_EXACT    (1<<1) /*  Must be distinct from BAYTREEM_UNIQUE  */
#define BAYSEEK_LASTELP1 (1<<2) /*  Find last element plus 1  */

/*  How to access pages in the underlying buffered file  */
#define BTPAGE_SIZE(bt)           (1L << (bt)->BAT_PageShift)
#define BTPAGE_OFFSET(bt,pageNr)  \
     ((unsigned long)(pageNr) << (bt)->BAT_PageShift)

/*  How to get the length of the page and the leftmost subpage element  */
#define BTPAGE_LENGTH(bt,p)    (((BTreePage *)(p))[0])
#define BTPAGE_SUBPAGE0(bt,p)  (((BTreePage *)(p))[1])

/*  Check if we have a leaf page  */
#define BTPAGE_ISLEAF(bt,p)    (BTPAGE_SUBPAGE0(bt,p) == -1)

/*  Where do fields on the page actually start  */
#define BTOFF_START(bt)        (2L * sizeof(BTreePage))

#define BTLEN_FIELD(bt)        ((bt)->BAT_FieldLength + 2 * sizeof(BTreePage))

#ifdef CONFIG_BTREE_EXTRALEAF
/*  Here we save some space on leaf pages!  */
#define BTLEN_FIELDL(bt)       ((bt)->BAT_FieldLength + sizeof(BTreePage))
#else
#define BTLEN_FIELDL(bt)       ((bt)->BAT_FieldLength + 2 * sizeof(BTreePage))
#endif

/*  Offsets for the sub-elements  */
#define BTOFF_USER(bt)         0L
#define BTLEN_USER(bt)         ((bt)->BAT_FieldLength + sizeof(BTreePage))

#define BTOFF_CONTENT(bt)      0L
#define BTOFF_SETNR(bt)        ((bt)->BAT_FieldLength)

#define BTOFF_SUBPAGE(bt)      BTLEN_USER(bt)
/*  To access the subpage entry of the previous element  */
#define BTOFF_SUBPAGEM1(bt)    (-(long)sizeof(BTreePage))

/*  Internal auxiliary macro to access a page (returns a
 *   pointer to the page).
 */
#define bayGetPage(bt,page,mode) \
     bFileGet2((bt)->BAT_BFile,BTPAGE_OFFSET(bt,page),BTPAGE_SIZE(bt), \
	       (mode) | BFILEMODE_FULLMAP)

/*  Set flags on a page  */
#define baySetPage(bt,page,mode) \
	bFileSet((bt)->BAT_BFile,BTPAGE_OFFSET(bt,page),BTPAGE_SIZE(bt),mode)
#endif
