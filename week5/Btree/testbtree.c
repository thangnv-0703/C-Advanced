/*JS*********************************************************************
*
*    Program : TESTBTREE
*    Language: ANSI-C
*    Author  : Joerg Schoen
*    Purpose : Test Bayer-Tree routines.
*
*************************************************************************/

#ifndef lint
static const char rcsid[] = "$Id$";
#endif

/*********     INCLUDES                                         *********/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <jssubs.h>

/*********     DEFINES                                          *********/
#if defined(DEBUGI) || defined(DEBUGD) || defined(DEBUGU)
# define OPTSTRING  "hp:f:uF:H:b:mM:dw:"
#else
# define OPTSTRING  "hp:f:uF:H:b:mM:d"
#endif

/*  Default values  */
#define DEFTREENAME   "btree.tst"

#define RESERVED      (2 * sizeof(long))

#define DEFPLENSHIFT   9  /*  Default: 512 bytes on page  */
#define DEFFIELDLEN    7  /*  ASCII fields  */

#define Prototype extern
/*********     PROTOTYPES                                       *********/
static int               fCompare(void *user,const void *a,const void *b);
static long              pageAdmin(BayerTree *bt,long page);
static int               prPages(BayerTree *bt,char *p,long page,
				 long n,int depth);
static char             *getWord(FILE *fp);
static long              populGet(FILE *fp,char *buff,long nr);
static void              checkLocks(BuffFile *bf);

static int FieldLen;
/*JS*********************************************************************
*
*************************************************************************/

int main(int argc,char *argv[])

/************************************************************************/
{
  BuffFile *bf;
  FILE *fp;
  BayerTree *bt;
  char *treeName,*string,*l;
  double frac;
  int i,root,hashLen,buffShift,pageShift,fieldLen;
  long count;
#if defined(DEBUGI) || defined(DEBUGD) || defined(DEBUGU)
  int whentostop = -1;
#endif
  int fMode,bMode,sMode;

  /*  Set defaults  */
  bt = NULL;
  hashLen    = 0;
  buffShift  = 0;
  pageShift  = DEFPLENSHIFT;
  fieldLen = DEFFIELDLEN;
  fMode = BFILE_WRITE;
  bMode = sMode = 0;
  frac = 0.8;

  while((i = jsGetOpt(&argc,(void *)argv,OPTSTRING,&string)) != EOF)
    switch(i) {
    case 'h':
      goto help;
    case 'p':
      pageShift = atoi(string);
      break;
    case 'f':
      fieldLen = atoi(string);
      break;
    case 'H':
      hashLen = atoi(string);
      break;
    case 'b':
      buffShift = atoi(string);
      break;
    case 'm':
      fMode |= BFILE_MMAP;
      break;
#if defined(DEBUGI) || defined(DEBUGD) || defined(DEBUGU)
    case 'w':
      whentostop = atoi(string);
      break;
#endif
    case 'u':
      bMode |= BAYTREEM_UNIQUE;
      break;
    case 'F':
      frac = strtod(string,NULL);
      break;
    case 'M':
      BSortMaxMem = atoi(string);
      break;
    case 'd':
      sMode |= BAYTREEF_DISTINCT;
      break;
    }

#if 0
  setbuf(stdout,NULL);
#endif

  if(argc < 3) goto help;
  treeName = (argc < 4) ? DEFTREENAME : argv[3];

  if((fp = fopen(argv[2],"r")) == NULL)
    goto error;

  FieldLen = fieldLen;

  /*  Default: try to open existing file  */
  root = 0;
  if((bf = bFileOpen(treeName,hashLen,buffShift,fMode)) == NULL) {
    root = -1;
    if((bf = bFileOpen(treeName,hashLen,buffShift,fMode | BFILE_CREATE))
       == NULL) goto error;
  }

  if((bt = bayTreeOpen(NULL,bf,pageShift,fieldLen,RESERVED,root,
		       pageAdmin,fCompare,&fieldLen)) == NULL)
    goto error;

  /*  Do *not* change fieldLen in the followin, since it is used
   *   from the comparison routine internally.
   */

  /* ------------------------------------------------- */
  if(strchr(argv[1],'i')) {
    printf("INSERTING...\n");
    for(count = 0 ; (l = getWord(fp)) ; ) {
      l[fieldLen - 1] = '\0';
#ifdef DEBUGI
      printf("#%d \"%s\"\n",count+1,l);
#endif

      /*  Insert in tree  */
      if(bayTreeInsert(bt,count+1,l,bMode) < 0) goto error;
      count++;

#ifdef DEBUGI
      if(count == whentostop || count == whentostop-1)
	if(bayTreeCheck(bt,NULL,31)<0) goto error;
#endif
    }
    /*  statistics  */
    printf("Inserted %ld words\n",count);

    checkLocks(bf);
  }

  /* ------------------------------------------------- */
  if(strchr(argv[1],'b')) {
    printf("BUILDING...\n");
    if(bayTreeBuild(bt,(long (*)(void *,char *,long))populGet,fp,frac,bMode)
       < 0) goto error;
  }

  /* ------------------------------------------------- */
  if(strchr(argv[1],'d')) {
    printf("DELETING...\n");

    if(fseek(fp,0L,SEEK_SET)) goto error;

    for(count = 0 ; (l = getWord(fp)) ; ) {
      l[fieldLen - 1] ='\0';
#ifdef DEBUGD
      printf("#%d \"%s\"\n",count+1,l);
#endif

      /*  Delete in tree  */
      if(bayTreeDelete(bt,count+1,l,0) < 0) goto error;
      count++;

#ifdef DEBUGD
      if(count == whentostop || count == whentostop-1) {
	if(bayTreeCheck(bt,NULL,31)<0) goto error;
      }
#endif
    }
    /*  statistics  */
    printf("Deleted %ld words\n",count);

    checkLocks(bf);
  }

  /* ------------------------------------------------- */
  if(strchr(argv[1],'u')) {
    char *f;
    int len;

    printf("UPDATING...\n");

    if((f = (char *)malloc(fieldLen)) == NULL) goto error;

    if(fseek(fp,0L,SEEK_SET)) goto error;

    for(count = 0 ; (l = getWord(fp)) ; count++) {
      l[fieldLen - 1] ='\0';

      len = strlen(l);
      for(i = 0 ; i < len ; i++) f[len - 1 - i] = l[i];
      f[len] = '\0';

#ifdef DEBUGU
      printf("#%d \"%s\"\n",count+1,l);
#endif

      /*  Delete in tree  */
      if(bayTreeUpdate(bt,count+1,l,f,bMode) < 0) goto error;
#ifdef DEBUGU
#if 1
      if(count+1 == whentostop || count+1 == whentostop-1)
	if(bayTreeCheck(bt,NULL,31)<0) goto error;
#else
      if(bayTreeCheck(bt,NULL,count+1== whentostop || count+1 == whentostop-1 ?
		      31 : 19)<0) goto error;
#endif
#endif
    }
    /*  statistics  */
    printf("Updated %ld words\n",count);

    checkLocks(bf);
  }

  /* ------------------------------------------------- */
  if(strchr(argv[1],'t') || strchr(argv[1],'T')) {
    char buffer[1000];
    BayTreeHandle *btl;
    long nr;

    printf("TRAVERSING...\n");
    if(strchr(argv[1],'T')) sMode |= BAYTREEF_INVERT;
    if(strchr(argv[1],'a')) sMode |= BAYTREEF_SLOWER;
    if(strchr(argv[1],'e')) sMode |= BAYTREEF_ELOWER;

    if((btl = bayTreeFStart(bt,NULL,(argc > 4 && argv[4][0]) ? argv[4] :
			    NULL,(argc > 5 && argv[5][0]) ? argv[5] : NULL,
			    NULL,NULL,sMode)) == NULL)
      goto error;

    while((nr = bayTreeFNext(btl,buffer,0)) >= 0)
      printf("%6ld: %d '%s'\n",nr,btl->BTL_TDepth,buffer);

    if(nr != -2) goto error;

    if(bayTreeFEnd(btl,0) < 0) goto error;
  }

  /* ------------------------------------------------- */
  if(strchr(argv[1],'c')) {
    printf("CHECKING...\n");
    (void)bayTreeCheck(bt,prPages,0x05);
  }

  /* ------------------------------------------------- */
  if(strchr(argv[1],'l')) {
    printf("CHECKING...\n");
    (void)bayTreeCheck(bt,prPages,0x1d);
  }

  /* ------------------------------------------------- */
  if(strchr(argv[1],'x')) {
    printf("DESTROYING...\n");
    if(bayTreeDestroy(bt,1) < 0) goto error;
  }

  if(bayTreeClose(bt,0) || bFileClose(bf) || fclose(fp)) goto error;

  return(0);
error:
  if(bt) {
    (void)bayTreeClose(bt,0);
    (void)bFileClose(bf);
  }
  jsperror("testbtree");
  return(30);
help:
  fprintf(stderr,"\
Usage: testbtree [<opt>] <mode> <infile> [<treefile> [<start> [<end>]]]\n\
  Read strings from <infile>, cut them in chunks of %d bytes (can be changed\n\
  with '-f' option), and depending on <mode> insert/delete etc. them in a\n\
  Bayer tree in the file <treefile> (Default name is \"%s\").\n\
  Mode can consist of the following letters:\n\
    'i' Insert words from infile into tree.\n\
    'b' Build up tree from words in infile.\n\
    'd' Delete words from infile in tree.\n\
    'u' Update words from infile in tree with their reversed counterpart.\n\
    'c' Check tree.\n\
    'x' Destroy tree.\n\
    't' Traverse tree ('T' in inverted order).\n\
  Options:\n\
    -h         Show this help information.\n\
    -p<num>    Set page length for Bayer tree (shift value).\n\
    -f<num>    Set field length.\n\
    -F<frac>   Set initial page filling when building up.\n\
    -u         Request uniqueness when inserting and updating.\n\
    -d         Remove multiple entries when traversing.\n\
    -b<num>    Set page length for file buffering (shift value).\n\
    -H<num>    Set length of hash-table for file buffering.\n\
    -m         Use memory mapped I/O.\n\
    -M<maxmem> Set the maximum memory used for in-core sorting for the 'b'\n\
               command.\n",DEFFIELDLEN,DEFTREENAME);
  return(5);
}

/*JS*********************************************************************
*   Field comparison routine
*************************************************************************/

static int fCompare(void *user,const void *a,const void *b)

/************************************************************************/
{
  return(strncmp(a,b,*(int *)user));
}

/*JS*********************************************************************
*   Page administration routine.
*************************************************************************/

static long pageAdmin(BayerTree *bt,long page)

/************************************************************************/
{
  char *r,*p;
  long len,root,offset;

  /*  Get and lock root page  */
  root = (bt->BAT_Root == -1) ? 0 : bt->BAT_Root;
  len = 1 << bt->BAT_PageShift;
  offset = len - RESERVED; /*  where our internal fields starts  */
  if((r = bFileGet(bt->BAT_BFile,root << bt->BAT_PageShift,&len,
		   BFILEMODE_DIRTY | BFILEMODE_PROT)) == NULL)
    goto error;

  if(page >= 0) { /* ***  Free a page  *** */
#if 0
    printf("FREE %d\n",page);
#endif
    len = 1 << bt->BAT_PageShift;
    if((p = bFileGet(bt->BAT_BFile,page << bt->BAT_PageShift,&len,
		     BFILEMODE_DIRTY)) == NULL) goto error;

    /*  Hang page in front of list  */
    memcpy(p + offset,r + offset,sizeof(page));
    memcpy(r + offset,&page,sizeof(page));

    /*  Unlock root page  */
    if(bFileSet(bt->BAT_BFile,root << bt->BAT_PageShift,1 << bt->BAT_PageShift,
		BFILEMODE_UNPROT) < 0) goto error;

    return(0);
  }

  if(bt->BAT_Root == -1) {
    /*  Set up first hidden field on root page: linked list of deleted pages */
    page = -1;
    memcpy(r + offset,&page,sizeof(page));

    /*  Second field: Total number of used pages  */
    page = 1;
    memcpy(r + offset + sizeof(long),&page,sizeof(page));

    /*  Creating root page, so return first page  */
    page = 0;
  } else {
    /*  Load hidden field  */
    memcpy(&page,r + offset,sizeof(page));

    if(page == -1) {
      long page2;

      /*  Use second hidden field to get page at end of file  */
      memcpy(&page,r + offset + sizeof(long),sizeof(page));
      page2 = page + 1;
      memcpy(r + offset + sizeof(long),&page2,sizeof(page2));

      /*  Prevent giving root page away multiple times  */
      if(page == 0) page = 1;
    } else {
static long Empty = -1L;
      len = 1 << bt->BAT_PageShift;
      if((p = bFileGet(bt->BAT_BFile,page << bt->BAT_PageShift,&len,
		       BFILEMODE_DIRTY)) == NULL) goto error;

      /*  Remove page from list  */
      memcpy(r + offset,p + offset,sizeof(page));
      memcpy(p + offset,&Empty,sizeof(Empty));
    }
  }

  /*  Unlock root page  */
  if(bFileSet(bt->BAT_BFile,root << bt->BAT_PageShift,1 << bt->BAT_PageShift,
	      BFILEMODE_UNPROT) < 0) goto error;

#if 0
  printf("NEW %d\n",page);
#endif

  return(page);
error:
  return(-1);
}

/*JS*********************************************************************
*   Do something for every page.
*************************************************************************/

static int prPages(BayerTree *bt,char *p,long page,long n,int depth)

/************************************************************************/
{
  if(n >= 0) {
#ifdef XXDEBUG
    printf("PAGE %d length %d, depth %d\n",page,n,depth);
#endif
  }
  return(0);
}

/*JS*********************************************************************
*   Get a word from a file.
*************************************************************************/

static char *getWord(FILE *fp)

/************************************************************************/
{
  int c,i;
static char buffer[1000];

  do
    c = getc(fp);
  while(c != EOF && !(isalnum(c) || c == '_'));
  if(c == EOF) return(NULL);

  buffer[0] = c;
  for(i = 1 ; (c = getc(fp)) != EOF ; i++) {
    if(!isalnum(c)) break;
    buffer[i] = c;
  }
  buffer[i] = '\0';

  return(buffer);
}

/*JS*********************************************************************
*
*************************************************************************/

static long populGet(FILE *fp,char *buff,long nr)

/************************************************************************/
{
  char *s;
static long Counter;

  if((s = getWord(fp)) == NULL) return(-2);
  memcpy(buff,s,FieldLen - 1);
  buff[FieldLen - 1] = '\0';

  return(++Counter);
}

/*JS*********************************************************************
*   Check if any locks are left over on the buffered file.
*************************************************************************/

static void checkLocks(BuffFile *bf)

/************************************************************************/
{
  BuffFilePage *p;
  int i;

  if(bf->BF_MMProt)
    fprintf(stderr,"ERROR: Memory mapped region still locked!\n");

  /*  Look for still locked pages  */
  for(i = 0 ; i < bf->BF_HashLen ; i++) {
    for(p = bf->BF_HashTab[i] ; p ; p = p->BFP_Next)
      if(p->BFP_Prot)
	fprintf(stderr,"ERROR: Hash %d Page %ld still locked!\n",i,
		p->BFP_PageNr);
  }
}
